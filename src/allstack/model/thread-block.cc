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
    m_total_send_message_num = 0;
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
        std::cout << Simulator::Now().GetSeconds()
                  << "\tGPU " << m_node->GetId()
                  << "\tThreadBlock " << m_id
                  << "\tResumed"
                  << std::endl;
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
            std::cout << Simulator::Now().GetSeconds()
                      << "\tGPU " << m_node->GetId()
                      << "\tThreadBlock " << m_id
                      << "\tPaused"
                      << std::endl;
        }
    }
    return !m_waitDep;
}

void
ThreadBlock::DoStep()
{
    NS_LOG_FUNCTION(this);
    Ptr<ThreadBlockStep> step = *m_currentStep;

    switch(step->GetType())
    {
        case ThreadBlockStep::NOP:
            CompleteStep();
            break;
        case ThreadBlockStep::REDUCE:
            DoReduce();
            break;
        case ThreadBlockStep::SEND:
            DoSend(step->GetCount());
            break;
        case ThreadBlockStep::RECV:
            DoRecv();
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
        CompleteThreadBlock();
    }
}

void
ThreadBlock::CompleteThreadBlock()
{
    NS_LOG_FUNCTION(this);
    // all steps complete
    if (m_total_send_message_num > 0)
    {
        // wait for all send complete
        Simulator::Schedule(Seconds(0.001), &ThreadBlock::CompleteThreadBlock, this);
        return;
    }
    m_gpuNode->FinishedTBCallback();
    return;
}

void
ThreadBlock::DoReduce()
{
    NS_LOG_FUNCTION(this);
    Simulator::Schedule(Seconds(0.001), &ThreadBlock::CompleteStep, this);
}

void
ThreadBlock::DoSend(uint32_t chunks)
{
    NS_LOG_FUNCTION(this);    
#ifdef NS3_MTP
    MtpInterface::explicitCriticalSection cs;
#endif
    m_total_send_message_num += 1;
#ifdef NS3_MTP
    cs.ExitSection();
#endif

    Simulator::Schedule(Seconds(0), &ThreadBlock::CompleteStep, this);
    uint64_t size = chunks * m_chunkSize;
    this->GetRdmaClient()->DoSend(size);
}

void
ThreadBlock::DoRecv()
{
    NS_LOG_FUNCTION(this);
    
#ifdef NS3_MTP
    MtpInterface::explicitCriticalSection cs;
#endif
    
    if (m_recv_message_num > 0)
    {
        // 如果有可用的消息数量，直接完成并减一
        m_recv_message_num--;
        
#ifdef NS3_MTP
        cs.ExitSection();
#endif
        
        Simulator::Schedule(Seconds(0), &ThreadBlock::CompleteStep, this);
    }
    else
    {
#ifdef NS3_MTP
        cs.ExitSection();
#endif
        
        // 否则等待，通过 RdmaClient 接收消息
        this->GetRdmaClient()->DoRecv();
    }
}

// get the rdma client bound to this thread block
Ptr<RdmaClient>
ThreadBlock::GetRdmaClient()
{
    NS_LOG_FUNCTION(this);
    return m_rdma_client;
}

void
ThreadBlock::BindRdmaClient(Ptr<RdmaClient> client)
{
    NS_LOG_FUNCTION(this);
    // client->SetAboveLayerCallback(
    //     MakeCallback(&ThreadBlock::CompleteStep, this),
    //     MakeCallback(&ThreadBlock::CompleteStep, this)
    // );
    // client->SetAboveLayerCallback(
    //     MakeNullCallback<void>(),
    //     MakeCallback(&ThreadBlock::CompleteStep, this)
    // );
    client->SetAboveLayerCallback(
        MakeCallback(&ThreadBlock::SendMessageDone, this),
        MakeCallback(&ThreadBlock::RecvMessageDone, this)
    );
    // client->SetAboveLayerCallback(
    //     MakeNullCallback<void>(),
    //     MakeCallback(&ThreadBlock::RecvMessageDone, this)
    // );
    m_rdma_client = client;
    m_rdma_clients.push_back(client);
}

void
ThreadBlock::RecvMessageDone()
{
    NS_LOG_FUNCTION(this);
    
#ifdef NS3_MTP
    MtpInterface::explicitCriticalSection cs;
#endif
    
    m_recv_message_num++;
    
    // 检查当前步骤是否是 RECV 且正在等待消息
    if (m_currentStep != m_steps.end())
    {
        Ptr<ThreadBlockStep> step = *m_currentStep;
        if (step->GetType() == ThreadBlockStep::RECV && m_recv_message_num > 0)
        {
            // 如果当前步骤是接收且有可用消息，完成步骤
            m_recv_message_num--;

#ifdef NS3_MTP
            cs.ExitSection();
#endif
            Simulator::Schedule(Seconds(0), &ThreadBlock::CompleteStep, this);
            return;
        }
    }
    
#ifdef NS3_MTP
    cs.ExitSection();
#endif
}

void
ThreadBlock::SendMessageDone()
{
    NS_LOG_FUNCTION(this);


#ifdef NS3_MTP
    MtpInterface::explicitCriticalSection cs;
#endif
    m_total_send_message_num -= 1;

#ifdef NS3_MTP
    cs.ExitSection();
#endif
}


}