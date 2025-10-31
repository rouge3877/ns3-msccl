#include "thread-block-step.h"

#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/boolean.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("ThreadBlockStep");

NS_OBJECT_ENSURE_REGISTERED(ThreadBlockStep);

TypeId
ThreadBlockStep::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::ThreadBlockStep")
            .SetParent<Object>()
            .SetGroupName("AllStack")
            .AddConstructor<ThreadBlockStep>()
            .AddAttribute(
                "s",
                "Index of this thread block step",
                UintegerValue(0),
                MakeUintegerAccessor(&ThreadBlockStep::m_s),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute(
                "type",
                "Type of this thread block step",
                EnumValue(ThreadBlockStep::NOP),
                MakeEnumAccessor(&ThreadBlockStep::m_type),
                MakeEnumChecker(
                    ThreadBlockStep::NOP, "nop",
                    ThreadBlockStep::REDUCE, "re", 
                    ThreadBlockStep::SEND, "s", 
                    ThreadBlockStep::RECV, "r",
                    ThreadBlockStep::COPY, "cpy",
                    ThreadBlockStep::RECV_REDUCE_COPY, "rrc",
                    ThreadBlockStep::RECV_REDUCE_COPY_SEND, "rrcs",
                    ThreadBlockStep::RECV_REDUCE_SEND, "rrs",
                    ThreadBlockStep::RECV_COPY_SEND, "rcs"
                ))
            .AddAttribute(
                "cnt",
                "Count of chunks this thread block step has to process",
                UintegerValue(0),
                MakeUintegerAccessor(&ThreadBlockStep::m_count),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute(
                "depid",
                "Id of the depended thread block",
                UintegerValue(0),
                MakeUintegerAccessor(&ThreadBlockStep::m_depid),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute(
                "deps",
                "Id of the depended thread block",
                UintegerValue(0),
                MakeUintegerAccessor(&ThreadBlockStep::m_deps),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute(
                "dep",
                "Whether be depended on other tb",
                BooleanValue(false),
                MakeBooleanAccessor(&ThreadBlockStep::m_dep),
                MakeBooleanChecker())
            .AddAttribute(
                "hasdep",
                "Whether be dependency on other tb",
                BooleanValue(false),
                MakeBooleanAccessor(&ThreadBlockStep::m_hasdep),
                MakeBooleanChecker());
    return tid;
}

ThreadBlockStep::ThreadBlockStep()
{
    NS_LOG_FUNCTION(this);
    m_s = 0;
    m_type = ThreadBlockStep::NOP;
    m_count = 0;
    m_depid = 0;
    m_deps = 0;
    m_dep = false;
    m_hasdep = false;
}

ThreadBlockStep::~ThreadBlockStep()
{
    NS_LOG_FUNCTION(this);
}

uint32_t
ThreadBlockStep::GetS() const
{
    NS_LOG_FUNCTION(this);
    return m_s;
}

ThreadBlockStep::ThreadBlockStepType_t
ThreadBlockStep::GetType() const
{
    NS_LOG_FUNCTION(this);
    return m_type;
}

uint32_t
ThreadBlockStep::GetCount() const
{
    NS_LOG_FUNCTION(this);
    return m_count;
}

uint32_t
ThreadBlockStep::GetDepId() const
{
    NS_LOG_FUNCTION(this);
    return m_depid;
}

uint32_t
ThreadBlockStep::GetDepS() const
{
    NS_LOG_FUNCTION(this);
    return m_deps;
}

bool
ThreadBlockStep::GetDep() const
{
    NS_LOG_FUNCTION(this);
    return m_dep;
}

bool
ThreadBlockStep::GetHasdep() const
{
    NS_LOG_FUNCTION(this);
    return m_hasdep;
}

void
ThreadBlockStep::DoDispose()
{
    NS_LOG_FUNCTION(this);
    Object::DoDispose();
}

}
