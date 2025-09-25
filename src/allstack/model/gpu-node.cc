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
            .AddConstructor<Node>();
    return tid;
}

uint32_t
GPUNode::AddThreadBlock(Ptr<ThreadBlock> tb)
{
    NS_LOG_FUNCTION(this << tb);
    uint32_t index = Node::AddApplication(tb);
    m_threadblocks.push_back(tb);
    m_tb_status.push_back(-1);
    tb->SetGPUNode(this);
    return index;
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