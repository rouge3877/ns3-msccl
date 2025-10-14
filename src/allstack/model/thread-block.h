#ifndef THREAD_BLOCK_H
#define THREAD_BLOCK_H

#include "ns3/thread-block-step.h"
#include "ns3/gpu-node.h"
#include "ns3/application.h"
#include "ns3/rdma-client.h"

#define REDUCE_TIME 0.000001        // 1us
#define SEND_TIME   0.000001        // 1us
#define COPY_TIME   0.0000000001    // 1ns
#define RECV_TIME   0.0000000001    // 1ns

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
        int GetSend() const;
        int GetRecv() const;
        int GetChannel() const;
        uint32_t AddStep(Ptr<ThreadBlockStep> step);

        void UpdateTBStatus(uint32_t index, uint32_t step); 

        void BindRdmaClient(Ptr<RdmaClient> client);
        Ptr<RdmaClient> GetRdmaClient();

        void RecvMessageDone();
        void SendMessageDone();
        
        void CompleteThreadBlock();

        ns3::Time m_end_time = Seconds(0);   //!< time when this thread block finished

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
        void DoRecvReduceCopy();
        void DoRecvReduceCopySend(uint32_t chunks);
        void DoRecvReduceSend(uint32_t chunks);
        void DoRecvCopySend(uint32_t chunks);
        void CompleteStep();

        Ptr<GPUNode> m_gpuNode;
        uint32_t m_id;          //!< id of this thread block
        std::vector<Ptr<ThreadBlockStep>> m_steps;
        std::vector<Ptr<ThreadBlockStep>>::iterator m_currentStep;

        bool m_waitDep;
        uint32_t m_depid;
        uint32_t m_deps;

        int m_send;             //!< Send Peer rank
        int m_recv;             //!< Recv Peer rank
        int m_channel;          //!< Channel id
        uint32_t m_chunkSize;   //!< Chunk Size (send size = chunksize * chunknum)

        std::vector<Ptr<RdmaClient>> m_rdma_clients;
        Ptr<RdmaClient> m_rdma_client;

        int m_recv_message_num;   //!< number of recv messages, stay in buffer
        int m_total_send_message_num;   //!< number of messages to send, for synchronization
};

}

#endif /* THREAD_BLOCK_H */