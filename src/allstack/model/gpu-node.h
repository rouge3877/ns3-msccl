#ifndef GPU_NODE_H
#define GPU_NODE_H

#include "ns3/node.h"
#include "ns3/thread-block.h"
#include "ns3/nstime.h"

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

        uint32_t AddThreadBlock(Ptr<ThreadBlock> tb);
        void UpdateTBStatus(uint32_t index, uint32_t step);
        int GetTBStatus(uint32_t index);

        int GetRank() const;
        void SetRank(int rank);
        Ptr<ThreadBlock> GetThreadBlock(uint32_t index) const;
        uint32_t GetNThreadBlocks() const;

        int FinishedTBCallback();

        ns3::Time m_end_time = Seconds(0);   //!< time when this GPU node finished all TBs

    protected:
        void DoDispose() override;

    private:
        int m_rank;      //!< rank of this GPU node
        std::vector<int>m_tb_status;
        std::vector<Ptr<ThreadBlock>> m_threadblocks;
        int m_tb_complete_count = 0;
};

}

#endif /* GPU_NODE_H */