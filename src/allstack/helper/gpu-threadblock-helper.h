#ifndef GPU_THREADBLOCK_HELPER_H
#define GPU_THREADBLOCK_HELPER_H

#include"ns3/tinyxml2.h"

#include "ns3/application-container.h"
#include "ns3/object-factory.h"
#include "ns3/gpu-node.h"
#include "ns3/thread-block-step.h"


namespace ns3
{

/**
 * \ingroup gputhreadblock
 * \brief Create a treadblock application to one GPU node to excute the msccl xml file
 */
class ThreadBlockStepHelper
{
    public:
        ThreadBlockStepHelper(tinyxml2::XMLElement * config);
        void SetAttribute(std::string name, const AttributeValue& value);
        Ptr<ThreadBlockStep> Install(Ptr<ThreadBlock> tb) const;
    private:
        ObjectFactory m_factory;
};

class ThreadBlockHelper
{
    public:
        ThreadBlockHelper(tinyxml2::XMLElement * config);
        void SetAttribute(std::string name, const AttributeValue& value);
        Ptr<ThreadBlock> Install(Ptr<GPUNode> gpu) const;

    private:
        ObjectFactory m_factory;
        tinyxml2::XMLElement * m_config;    //<! xml for threadblock config
};

class GPUThreadBlockHelper
{
    public:
        GPUThreadBlockHelper(tinyxml2::XMLElement * config);
        ApplicationContainer Install(Ptr<GPUNode> gpu) const;

    private:
        tinyxml2::XMLElement * m_config;    //<! xml for gpu config
};


}

#endif /* GPU_THREADBLOCK_HELPER_H */