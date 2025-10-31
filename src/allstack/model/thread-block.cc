#include "thread-block.h"

#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/simulator.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("ThreadBlock");

NS_OBJECT_ENSURE_REGISTERED(ThreadBlock);


TypeId
ThreadBlock::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::ThreadBlock")
            .SetParent<Application>()
            .SetGroupName("AllStack")
            .AddConstructor<ThreadBlock>()
            .AddTraceSource (
                "TotalSendMessageNum",
                "Total number of messages to send",
                MakeTraceSourceAccessor(&ThreadBlock::m_total_send_message_num_trace),
                "ns3::TracedValueCallback::Int32")
            .AddAttribute(
                "id",
                "The id of this thread block",
                UintegerValue(0),
                MakeUintegerAccessor(&ThreadBlock::m_id),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute(
                "send",
                "The rank that send to, -1 means no send",
                IntegerValue(-1),
                MakeIntegerAccessor(&ThreadBlock::m_send),
                MakeIntegerChecker<int32_t>())
            .AddAttribute(
                "recv",
                "The rank that recv from, -1 means no recv",
                IntegerValue(-1),
                MakeIntegerAccessor(&ThreadBlock::m_recv),
                MakeIntegerChecker<int32_t>())
            .AddAttribute(
                "channel",
                "Channel id",
                IntegerValue(-1),
                MakeIntegerAccessor(&ThreadBlock::m_channel),
                MakeIntegerChecker<int32_t>())
            .AddAttribute(
                "chunkSize",
                "Size of Chunk, times ChunkNum = send message size",
                UintegerValue(1024),
                MakeUintegerAccessor(&ThreadBlock::m_chunkSize),
                MakeUintegerChecker<uint32_t>());
    return tid;
}

ThreadBlock::ThreadBlock()
{
    NS_LOG_FUNCTION(this);
    m_gpuNode = nullptr;
    m_id = 0;
    m_waitDep = false;
    m_depid = 0;
    m_deps = 0;
    m_chunkSize = 1024;
    m_recv_message_num = 0;
    m_total_send_message_num_trace = 0;
    m_total_send_message_num_trace.ConnectWithoutContext(
        MakeCallback(&ThreadBlock::SendMessageNumChanged, this));
    m_step_finish_flag = false;
}

ThreadBlock::~ThreadBlock()
{
    NS_LOG_FUNCTION(this);
}

Ptr<GPUNode>
ThreadBlock::GetGPUNode() const
{
    NS_LOG_FUNCTION(this);
    return m_gpuNode;
}

void
ThreadBlock::SetGPUNode(Ptr<GPUNode> node)
{
    NS_LOG_FUNCTION(this);
    m_gpuNode = node;
}

uint32_t
ThreadBlock::AddStep(Ptr<ThreadBlockStep> step)
{
    NS_LOG_FUNCTION(this << step);
    uint32_t index = m_steps.size();
    m_steps.push_back(step);

    return index;
}

uint32_t
ThreadBlock::GetId() const
{
    NS_LOG_FUNCTION(this);
    return m_id;
}

int
ThreadBlock::GetSend() const
{
    NS_LOG_FUNCTION(this);
    return m_send;
}

int
ThreadBlock::GetRecv() const
{
    NS_LOG_FUNCTION(this);
    return m_recv;
}

int
ThreadBlock::GetChannel() const
{
    NS_LOG_FUNCTION(this);
    return m_channel;
}

void
ThreadBlock::UpdateTBStatus(uint32_t id, uint32_t s)
{
    NS_LOG_FUNCTION(this);
    if (m_waitDep && m_depid == id && m_deps <= s)
    {
        m_waitDep = false;
        NS_LOG_INFO("GPU " << m_node->GetId() << " ThreadBlock " << m_id << " Resumed.");
        DoStep();
    }
}

void
ThreadBlock::DoDispose()
{
    NS_LOG_FUNCTION(this);
    for (auto i = m_steps.begin(); i != m_steps.end(); i++)
    {
        Ptr<ThreadBlockStep> step = *i;
        step->Dispose();
        *i = nullptr;
    }
    m_steps.clear();    
    m_gpuNode = nullptr;
    Application::DoDispose();
}

void
ThreadBlock::StartApplication()
{
    NS_LOG_FUNCTION(this);
    m_currentStep = m_steps.begin();
    StartStep();
}

void
ThreadBlock::StopApplication()
{
    NS_LOG_FUNCTION(this);
}

void
ThreadBlock::StartStep()
{
    if (CheckDep())
    {
        DoStep();
    }
}

bool
ThreadBlock::CheckDep()
{
    NS_LOG_FUNCTION(this);
    Ptr<ThreadBlockStep> step = *m_currentStep;

    m_waitDep = false;
    if (step->GetDep())
    {
        uint32_t depid = step->GetDepId();
        uint32_t deps = step->GetDepS();
        int completed_step = m_gpuNode->GetTBStatus(depid);
        if (completed_step < 0 or uint32_t(completed_step) < deps)
        {
            m_waitDep = true;
            m_depid = depid;
            m_deps = deps;
            NS_LOG_INFO("GPU " << m_node->GetId() << " ThreadBlock " << m_id << " Paused.");
        }
    }
    return !m_waitDep;
}

void
ThreadBlock::DoStep()
{
    NS_LOG_FUNCTION(this);
    Ptr<ThreadBlockStep> step = *m_currentStep;
    NS_LOG_INFO("GPU " << m_node->GetId() << " ThreadBlock " << m_id << " Step " << step->GetS() << " [" << step->GetType() << "]"
                << " start. ( rank = " << DynamicCast<GPUNode>(m_node)->GetRank() << ", tb_id = " << m_id << " )");

    switch(step->GetType())
    {
        case ThreadBlockStep::NOP:
            CompleteStep();
            break;
        case ThreadBlockStep::REDUCE:
            DoReduce();
            break;
        case ThreadBlockStep::COPY:
            DoReduce(); // 使用 Reduce 时间模拟 Copy 操作
            break;
        case ThreadBlockStep::SEND:
            DoSend(step->GetCount());
            break;
        case ThreadBlockStep::RECV:
            DoRecv();
            break;
        case ThreadBlockStep::RECV_REDUCE_COPY:
            // DoRecv();
            // DoReduce();
            DoRecvReduceCopy();
            break;
        case ThreadBlockStep::RECV_REDUCE_COPY_SEND:
            // DoRecv();
            // DoReduce();
            // DoSend(step->GetCount());
            DoRecvReduceCopySend(step->GetCount());
            break;
        case ThreadBlockStep::RECV_REDUCE_SEND:
            // DoRecv();
            // DoReduce();
            // DoSend(step->GetCount());
            DoRecvReduceSend(step->GetCount());
            break;
        case ThreadBlockStep::RECV_COPY_SEND:
            // DoRecv();
            // DoSend(step->GetCount());
            DoRecvCopySend(step->GetCount());
            break;
        default:
            break;
    }
    
}

void
ThreadBlock::CompleteStep()
{
    NS_LOG_FUNCTION(this);
    Ptr<ThreadBlockStep> step = *m_currentStep;
    NS_LOG_INFO("GPU " << m_node->GetId() << " ThreadBlock " << m_id << " Step " << step->GetS() << " [" << step->GetType() << "]"
                << " complete. ( rank = " << DynamicCast<GPUNode>(m_node)->GetRank() << ", tb_id = " << m_id << " )");

    // notify completation
    if (step->GetHasdep())
    {
        m_gpuNode->UpdateTBStatus(m_id, step->GetS());
    }

    // next step
    m_currentStep ++;
    if (m_currentStep != m_steps.end())
    {
        StartStep();
    }
    else
    {
        NS_LOG_INFO("GPU " << m_node->GetId() << " ThreadBlock " << m_id << " All Steps Complete. ( rank = " << DynamicCast<GPUNode>(m_node)->GetRank() << " )");
        // record end time
        m_end_time = Simulator::Now();
        CompleteThreadBlock();
    }
}

void
ThreadBlock::CompleteThreadBlock()
{
    NS_LOG_FUNCTION(this);
    // all steps complete
    if (m_total_send_message_num_trace > 0)
    {
        // wait for all send complete
        m_step_finish_flag = true;
        return;
    }
    m_gpuNode->FinishedTBCallback();
    return;
}

void
ThreadBlock::SendMessageNumChanged(int oldValue, int newValue)
{
    NS_LOG_FUNCTION(this << oldValue << newValue);

    if (newValue == 0 && m_step_finish_flag)
    {
        m_step_finish_flag = false; // 重置标志以防止重复调用
        NS_LOG_INFO("All send operations completed. Finishing ThreadBlock.");
        m_gpuNode->FinishedTBCallback();
    }
}

void
ThreadBlock::DoReduce()
{
    NS_LOG_FUNCTION(this);
    Simulator::Schedule(Seconds(REDUCE_TIME), &ThreadBlock::CompleteStep, this);
}

void
ThreadBlock::DoSend(uint32_t chunks)
{
    NS_LOG_FUNCTION(this);
    m_total_send_message_num_trace.Set(m_total_send_message_num_trace + 1);

    Simulator::Schedule(Seconds(SEND_TIME), &ThreadBlock::CompleteStep, this);
    uint64_t size = chunks * m_chunkSize;
    this->GetSendRdmaClient()->DoSend(size);
}

void
ThreadBlock::DoRecv()
{
    NS_LOG_FUNCTION(this);
    if (m_recv_message_num > 0)
    {
        // 如果有可用的消息数量，直接完成并减一
        m_recv_message_num--;
        Simulator::Schedule(Seconds(0), &ThreadBlock::CompleteStep, this);
    }
    else
    {
        // 否则等待，通过 RdmaClient 接收消息
        this->GetRecvRdmaClient()->DoRecv();
    }
}

void
ThreadBlock::DoRecvReduceCopy()
{
    NS_LOG_FUNCTION(this);
    if (m_recv_message_num > 0)
    {
        // 如果有可用的消息数量，直接完成并减一
        m_recv_message_num--;
        Simulator::Schedule(Seconds(REDUCE_TIME + COPY_TIME), &ThreadBlock::CompleteStep, this);
    }
    else
    {
        // 否则等待，通过 RdmaClient 接收消息
        this->GetRecvRdmaClient()->DoRecv();
    }
}

void
ThreadBlock::DoRecvReduceCopySend(uint32_t chunks)
{
    NS_LOG_FUNCTION(this);
    if (m_recv_message_num > 0)
    {
        // 如果有可用的消息数量，直接完成并减一
        m_recv_message_num--;

        // 这里假设 Reduce 和 Copy 是同时进行的
        m_total_send_message_num_trace.Set(m_total_send_message_num_trace + 1);

        Simulator::Schedule(Seconds(REDUCE_TIME + COPY_TIME + SEND_TIME), &ThreadBlock::CompleteStep, this);
        uint64_t size = chunks * m_chunkSize;
        this->GetSendRdmaClient()->DoSend(size);
    }
    else
    {
        // 否则等待，通过 RdmaClient 接收消息
        this->GetRecvRdmaClient()->DoRecv();
    }
}

void
ThreadBlock::DoRecvReduceSend(uint32_t chunks)
{
    NS_LOG_FUNCTION(this);
    if (m_recv_message_num > 0)
    {
        // 如果有可用的消息数量，直接完成并减一
        m_recv_message_num--;
        m_total_send_message_num_trace.Set(m_total_send_message_num_trace + 1);

        Simulator::Schedule(Seconds(REDUCE_TIME + SEND_TIME), &ThreadBlock::CompleteStep, this);
        uint64_t size = chunks * m_chunkSize;
        this->GetSendRdmaClient()->DoSend(size);
    }
    else
    {
        // 否则等待，通过 RdmaClient 接收消息
        this->GetRecvRdmaClient()->DoRecv();
    }
}

void
ThreadBlock::DoRecvCopySend(uint32_t chunks)
{
    NS_LOG_FUNCTION(this);
    if (m_recv_message_num > 0)
    {
        // 如果有可用的消息数量，直接完成并减一
        m_recv_message_num--;
        m_total_send_message_num_trace.Set(m_total_send_message_num_trace + 1);
        Simulator::Schedule(Seconds(COPY_TIME + SEND_TIME), &ThreadBlock::CompleteStep, this);
        uint64_t size = chunks * m_chunkSize;
        this->GetSendRdmaClient()->DoSend(size);
    }
    else
    {
        // 否则等待，通过 RdmaClient 接收消息
        this->GetRecvRdmaClient()->DoRecv();
    }
}

Ptr<RdmaClient>
ThreadBlock::GetSendRdmaClient() const
{
    NS_LOG_FUNCTION(this);
    return m_send_rdma_client;
}

Ptr<RdmaClient>
ThreadBlock::GetRecvRdmaClient() const
{
    NS_LOG_FUNCTION(this);
    return m_recv_rdma_client;
}

void
ThreadBlock::BindRdmaClients(Ptr<RdmaClient> send_client, Ptr<RdmaClient> recv_client)
{
    NS_LOG_FUNCTION(this);
    // bind send rdma client
    if (send_client)
    {
        send_client->SetAboveLayerCallback(
            MakeCallback(&ThreadBlock::SendMessageDone, this),
            MakeNullCallback<void>()
        );
    }
    m_send_rdma_client = send_client;

    // bind recv rdma client
    if (recv_client)
    {
        recv_client->SetAboveLayerCallback(
            MakeNullCallback<void>(),
            MakeCallback(&ThreadBlock::RecvMessageDone, this)
        );
    }
    m_recv_rdma_client = recv_client;
}

void
ThreadBlock::RecvMessageDone()
{
    NS_LOG_FUNCTION(this);
    m_recv_message_num++;
    
    // 检查当前步骤是否是 RECV 且正在等待消息
    if (m_currentStep != m_steps.end())
    {
        Ptr<ThreadBlockStep> step = *m_currentStep;

        if (m_recv_message_num > 0) {
            switch(step->GetType())
            {
                case ThreadBlockStep::RECV:
                    m_recv_message_num--;
                    Simulator::Schedule(Seconds(0), &ThreadBlock::CompleteStep, this);
                    break;
                case ThreadBlockStep::RECV_REDUCE_COPY:
                    m_recv_message_num--;
                    Simulator::Schedule(Seconds(REDUCE_TIME + COPY_TIME), &ThreadBlock::CompleteStep, this);
                    break;
                case ThreadBlockStep::RECV_REDUCE_COPY_SEND:
                {
                    m_recv_message_num--;
                    Simulator::Schedule(Seconds(REDUCE_TIME + SEND_TIME + COPY_TIME), &ThreadBlock::CompleteStep, this);
                    m_total_send_message_num_trace.Set(m_total_send_message_num_trace + 1);
                    uint64_t size = step->GetCount() * m_chunkSize;
                    this->GetSendRdmaClient()->DoSend(size);
                    break;
                }
                case ThreadBlockStep::RECV_REDUCE_SEND:
                {
                    m_recv_message_num--;
                    Simulator::Schedule(Seconds(REDUCE_TIME), &ThreadBlock::CompleteStep, this);
                    m_total_send_message_num_trace.Set(m_total_send_message_num_trace + 1);
                    uint64_t size = step->GetCount() * m_chunkSize;
                    this->GetSendRdmaClient()->DoSend(size);
                    break;
                }
                case ThreadBlockStep::RECV_COPY_SEND:
                {
                    m_recv_message_num--;
                    Simulator::Schedule(Seconds(SEND_TIME), &ThreadBlock::CompleteStep, this);
                    m_total_send_message_num_trace.Set(m_total_send_message_num_trace + 1);
                    uint64_t size = step->GetCount() * m_chunkSize;
                    this->GetSendRdmaClient()->DoSend(size);
                    break;
                }
                default:
                    break;
            }
        }
    }
}

void
ThreadBlock::SendMessageDone()
{
    NS_LOG_FUNCTION(this);
    m_total_send_message_num_trace.Set(m_total_send_message_num_trace - 1);
}


}