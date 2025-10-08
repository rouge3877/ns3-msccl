#ifndef THREAD_BLOCK_STEP_H
#define THREAD_BLOCK_STEP_H

#include "ns3/object.h"

namespace ns3
{


class ThreadBlockStep : public Object
{
    public:
        /**
         * \brief Get the type ID.
         * \return the object TypeId
         */
        static TypeId GetTypeId();

        ThreadBlockStep();
        ~ThreadBlockStep() override;

        
        enum ThreadBlockStepType_t
        {
            NOP,        //!< no to do
            REDUCE,     //!< compute
            SEND,       //!< send to peer
            RECV,       //!< receive from peer
            RECV_REDUCE_COPY,       //!< receive and reduce and copy to peer
            RECV_REDUCE_COPY_SEND,  //!< receive and reduce and copy to peer and send to next
            RECV_REDUCE_SEND,       //!< receive and reduce and send to next
            RECV_COPY_SEND,         //!< receive and copy to peer and send to next
            LAST_TYPE   //!< Used only in debug messages
        };
        uint32_t GetS() const;
        ThreadBlockStep::ThreadBlockStepType_t GetType() const;
        uint32_t GetCount() const;
        uint32_t GetDepId() const;
        uint32_t GetDepS() const;
        bool GetDep() const;
        bool GetHasdep() const;

    protected:
        void DoDispose() override;

    private:
        uint32_t m_s;                   //!< step index
        ThreadBlockStepType_t m_type;   //!< step type
        uint32_t m_count;                 //!< count of chunks
        uint32_t m_depid;               //!< the id of depended threadblock
        uint32_t m_deps;                //!< the step index of depended threadblock
        bool m_dep;                   //!< whether be depended on one other threadblock
        bool m_hasdep;                  //!< whether be dependency for other threadblocks
};

}

#endif /* THREAD_BLOCK_STEP_H */