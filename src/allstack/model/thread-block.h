#ifndef THREAD_BLOCK_H
#define THREAD_BLOCK_H

#include "ns3/thread-block-step.h"
#include "ns3/gpu-node.h"
#include "ns3/application.h"

namespace ns3
{

class ThreadBlockStep;
class GPUNode;

class ThreadBlock : public Application
{
    public:
        /**
         * \brief Get the type ID.
         * \return the object TypeId
         */
        static TypeId GetTypeId();

        ThreadBlock();
        ~ThreadBlock() override;

        Ptr<GPUNode> GetGPUNode() const;
        void SetGPUNode(Ptr<GPUNode> node);

        uint32_t GetId() const;
        uint32_t AddStep(Ptr<ThreadBlockStep> step);

        void UpdateTBStatus(uint32_t index, uint32_t step); 

    protected:
        void DoDispose() override;

    private:
        void StartApplication() override;
        void StopApplication() override;
        void StartStep();
        bool CheckDep();
        void DoStep();
        void DoReduce();
        void DoSend(uint32_t chunks);
        void DoRecv();                
        void CompleteStep();

        Ptr<GPUNode> m_gpuNode;
        uint32_t m_id;          //!< id of this thread block
        std::vector<Ptr<ThreadBlockStep>> m_steps;
        std::vector<Ptr<ThreadBlockStep>>::iterator m_currentStep;

        bool m_waitDep;
        uint32_t m_depid;
        uint32_t m_deps;
};

}

#endif /* THREAD_BLOCK_H */