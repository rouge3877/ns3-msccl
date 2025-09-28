#include "gpu-threadblock-helper.h"

#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/string.h"
#include "ns3/boolean.h"

namespace ns3
{

ThreadBlockStepHelper::ThreadBlockStepHelper(tinyxml2::XMLElement * config)
{
    m_factory.SetTypeId(ThreadBlockStep::GetTypeId());
    SetAttribute("s",
        UintegerValue(static_cast<uint32_t>(std::stoul(config->Attribute("s")))));    
    SetAttribute("type",
        StringValue(static_cast<std::string>(config->Attribute("type"))));    
    SetAttribute("cnt",
        UintegerValue(static_cast<uint32_t>(std::stoul(config->Attribute("cnt")))));
    
    int depid = static_cast<int>(std::stoi(config->Attribute("depid")));
    int deps = static_cast<int>(std::stoi(config->Attribute("deps")));
    if (depid >-1 && deps > -1)
    {
        SetAttribute("dep", BooleanValue(true));
        SetAttribute("depid", UintegerValue(depid));
        SetAttribute("deps", UintegerValue(deps));
    }
    else
    {
        SetAttribute("dep", BooleanValue(false));
    }

    SetAttribute("hasdep",
        BooleanValue(static_cast<bool>(std::stoi(config->Attribute("hasdep")))));
}

void
ThreadBlockStepHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

Ptr<ThreadBlockStep>
ThreadBlockStepHelper::Install(Ptr<ThreadBlock> tb) const
{
    Ptr<ThreadBlockStep> step = m_factory.Create<ThreadBlockStep>();
    uint32_t index = tb->AddStep(step);
    NS_ASSERT(index==step->GetS());
    return step;
}

ThreadBlockHelper::ThreadBlockHelper(tinyxml2::XMLElement * config)
{
    m_factory.SetTypeId(ThreadBlock::GetTypeId());
    m_config = config;

    SetAttribute("id",
        UintegerValue(static_cast<uint32_t>(std::stoul(m_config->Attribute("id")))));
    SetAttribute("send",
        IntegerValue(static_cast<int32_t>(std::stoi(m_config->Attribute("send")))));
    SetAttribute("recv",
        IntegerValue(static_cast<int32_t>(std::stoi(m_config->Attribute("recv")))));
    SetAttribute("channel",
        IntegerValue(static_cast<int32_t>(std::stoi(m_config->Attribute("chan")))));
}

void
ThreadBlockHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

Ptr<ThreadBlock>
ThreadBlockHelper::Install(Ptr<GPUNode> gpu) const
{
    Ptr<ThreadBlock> tb = m_factory.Create<ThreadBlock>();
    for (auto step = m_config->FirstChildElement("step"); step != nullptr; step = step->NextSiblingElement("step"))
    {
        ThreadBlockStepHelper helper(step);
        helper.Install(tb);
    }
    uint32_t index = gpu->AddThreadBlock(tb);
    NS_ASSERT(index == tb->GetId());
    return tb;
}

GPUThreadBlockHelper::GPUThreadBlockHelper(tinyxml2::XMLElement * config)
{
    m_config = config;
}

ApplicationContainer
GPUThreadBlockHelper::Install(Ptr<GPUNode> gpu) const
{
    ApplicationContainer apps;
    for (auto tb = m_config->FirstChildElement("tb"); tb != nullptr; tb = tb->NextSiblingElement("tb"))
    {
        ThreadBlockHelper tb_helper(tb);
        apps.Add(tb_helper.Install(gpu));
    }
    return apps;
}

}