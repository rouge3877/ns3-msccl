#ifndef GPU_NODE_H
#define GPU_NODE_H

#include <fstream>
#include "ns3/tinyxml2.h"

#include "ns3/application-container.h"
#include "ns3/object-factory.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/node.h"
#include "ns3/thread-block.h"
#include "ns3/nstime.h"

#include "ns3/rdma-client.h"

struct MPIStep{
    std::string name;
    tinyxml2::XMLElement* algo;
    size_t total_size_bytes;
    std::vector<int> ranks;
    double interval; // time interval to start this MPI step after previous step completed
};

struct ConnectionInfo{
    uint32_t src_rank;
    uint32_t dst_rank;
    int channel;

    ConnectionInfo(uint32_t s, uint32_t d, int c)
        : src_rank(s), dst_rank(d), channel(c) {}

    ConnectionInfo() : src_rank(0), dst_rank(0), channel(0) {}

    bool operator<(const ConnectionInfo& other) const {
        if (src_rank != other.src_rank)
            return src_rank < other.src_rank;
        if (dst_rank != other.dst_rank)
            return dst_rank < other.dst_rank;
        return channel < other.channel;
    }
};

namespace ns3
{
class ThreadBlock;

class GPUNode : public Node
{
    public:
        /**
         * \brief Get the type ID.
         * \return the object TypeId
         */
        static TypeId GetTypeId();

        GPUNode();

        uint32_t AddThreadBlock(Ptr<ThreadBlock> tb);
        void UpdateTBStatus(uint32_t index, uint32_t step);
        int GetTBStatus(uint32_t index);

        int GetRank() const;
        void SetRank(int rank);
        Ptr<ThreadBlock> GetThreadBlock(uint32_t index) const;
        uint32_t GetNThreadBlocks() const;

        int FinishedTBCallback();

        void AddMPIStep(MPIStep &step, double interval);
        void DoMPIStep();
        void StartMPIStep();
        void CompleteMPIStep();
        std::vector<MPIStep>* GetMPISteps() const;

        void AddConnection(ConnectionInfo conn, Ptr<RdmaClient> client);

        ns3::Time m_end_time = Seconds(0);   //!< time when this GPU node finished all TBs

    protected:
        void DoDispose() override;

    private:
        int m_rank;      //!< rank of this GPU node
        std::vector<int>m_tb_status;
        std::vector<Ptr<ThreadBlock>> m_threadblocks;
        int m_tb_complete_count = 0;

        std::vector<MPIStep> m_mpi_steps;
        uint32_t m_mpi_step_idx = 0;

        // connections about this GPU node
        std::map<ConnectionInfo, Ptr<RdmaClient>> m_connections;
};

}

#endif /* GPU_NODE_H */