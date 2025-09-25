#include "thread-block.h"

#include "ns3/uinteger.h"
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
    std::cout << Simulator::Now().GetSeconds()
              << "\tGPU " << m_node->GetId()
              << "\tThreadBlock " << m_id
              << "\tStep " << step->GetS()
              << std::endl;

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
    Simulator::Schedule(Seconds(0.002), &ThreadBlock::CompleteStep, this);
}

void
ThreadBlock::DoRecv()
{
    NS_LOG_FUNCTION(this);
    Simulator::Schedule(Seconds(0.010), &ThreadBlock::CompleteStep, this);
}

}