#include "gpu-node.h"


#include "ns3/integer.h"
#include "ns3/simulator.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("GPUNode");

NS_OBJECT_ENSURE_REGISTERED(GPUNode);

TypeId
GPUNode::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::GPUNode")
            .SetParent<Node>()
            .SetGroupName("AllStack")
            .AddConstructor<Node>()
            .AddAttribute(
                "rank",
                "The rank of this GPU node",
                IntegerValue(-1),
                MakeIntegerAccessor(&GPUNode::m_rank),
                MakeIntegerChecker<int32_t>());
    return tid;
}

Ptr<ThreadBlock>
GPUNode::GetThreadBlock(uint32_t index) const
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(index < m_threadblocks.size(), "Get ThreadBlock Index out of range");
    return m_threadblocks[index];
}

uint32_t
GPUNode::GetNThreadBlocks() const
{
    NS_LOG_FUNCTION(this);
    return m_threadblocks.size();
}

int
GPUNode::GetRank() const
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(m_rank >= 0, "GPUNode rank not set");
    return m_rank;
}

void
GPUNode::SetRank(int rank)
{
    NS_LOG_FUNCTION(this);
    m_rank = rank;
}

uint32_t
GPUNode::AddThreadBlock(Ptr<ThreadBlock> tb)
{
    NS_LOG_FUNCTION(this << tb);
    uint32_t index = Node::AddApplication(tb);
    m_threadblocks.push_back(tb);
    m_tb_status.push_back(-1);
    m_tb_complete_count += 1;
    tb->SetGPUNode(this);
    return index;
}

int
GPUNode::FinishedTBCallback()
{
    NS_LOG_FUNCTION(this);
#ifdef NS3_MTP
    MtpInterface::explicitCriticalSection cs;
#endif
    m_tb_complete_count -= 1;
#ifdef NS3_MTP
    cs.ExitSection();
#endif
    if (m_tb_complete_count == 0)
    {
        for (auto i = m_threadblocks.begin(); i != m_threadblocks.end(); i++)
        {
            Ptr<ThreadBlock> tb = *i;
            Ptr<RdmaClient> rdmaClient = tb->GetRdmaClient();
            rdmaClient->FinishQp();

            if (m_end_time < tb->m_end_time)
            {
                m_end_time = tb->m_end_time;
            }
        }
        return 1;
    }
    return 0;
}

void
GPUNode::UpdateTBStatus(uint32_t index, uint32_t step)
{
    NS_LOG_FUNCTION(this);
    m_tb_status[index] = step;

    std::cout << Simulator::Now().GetSeconds()
              << "\tGPU " << GetId()
              << "\tThreadBlock " << index
              << "\tStep " << step
              << "\tNotified!"
              << std::endl;

    for (auto i = m_threadblocks.begin(); i != m_threadblocks.end(); i++)
    {
        Ptr<ThreadBlock> tb = *i;
        tb->UpdateTBStatus(index, step);
    }
}

int
GPUNode::GetTBStatus(uint32_t index)
{
    NS_LOG_FUNCTION(this);
    return m_tb_status[index];
}

void
GPUNode::DoDispose()
{
    m_tb_status.clear();
    m_threadblocks.clear();
    Node::DoDispose();
}

}