#include <execinfo.h>
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/allstack-module.h"

#ifdef NS3_MTP
#include "ns3/mtp-interface.h"
#endif
#ifdef NS3_MPI
#include <mpi.h>
#include "ns3/mpi-interface.h"
#endif

#include "common.h"

using namespace std;
using namespace ns3;

extern uint32_t node_num, switch_num, link_num, trace_num, nvswitch_num, gpus_per_server;
extern tinyxml2::XMLDocument algo_xml_doc;
extern std::unordered_map<uint32_t, unordered_map<uint32_t, uint16_t>> portNumber;
extern double simulator_start_time;
extern double simulator_stop_time;
extern double collective_start_time;


void my_qp_finish(FILE *fout, Ptr<RdmaQueuePair> q)
{
    uint32_t sid = ip_to_node_id(q->sip), did = ip_to_node_id(q->dip);
#ifdef NS3_MTP
    MtpInterface::explicitCriticalSection cs;
#endif
    Ptr<Node> dstNode = n.Get(did);
    Ptr<RdmaDriver> rdma = dstNode->GetObject<RdmaDriver>();
    rdma->m_rdma->DeleteRxQp(q->sip.Get(), q->m_pg, q->sport);

    NS_LOG_INFO("QP FINISH: src " << sid << " did " << did << " port " << q->sport);

    cancel_monitor();
#ifdef NS3_MTP
    cs.ExitSection();
#endif
}

void my_send_finish(FILE *fout, Ptr<RdmaQueuePair> q)
{
    // Seems that the occasion to call this function is error-prone !!! (rouge)
    // rouge: this function is called when send is finished (without ack) ! (rouge after)
#ifdef NS3_MTP
    MtpInterface::explicitCriticalSection cs;
#endif
    // Currently do nothing
    // uint32_t sid = ip_to_node_id(q->sip), did = ip_to_node_id(q->dip);
#ifdef NS3_MTP
    cs.ExitSection();
#endif
}

void my_message_finish(FILE *fout, Ptr<RdmaQueuePair> q, uint64_t msgSize)
{
#ifdef NS3_MTP
    MtpInterface::explicitCriticalSection cs;
#endif
    uint32_t sid = ip_to_node_id(q->sip), did = ip_to_node_id(q->dip);
    uint64_t base_rtt = pairRtt[sid][did], b = pairBw[sid][did];
    uint32_t packet_payload_size =
        get_config_value_ns3<uint64_t>("ns3::RdmaHw::Mtu");
    uint64_t size = msgSize;
    uint32_t total_bytes = size +
                           ((size - 1) / packet_payload_size + 1) *
                               (CustomHeader::GetStaticWholeHeaderSize() -
                                IntHeader::GetStaticSize()); // translate to the minimum bytes
                                                             // required (with header but no INT)
    uint64_t standalone_fct = base_rtt + total_bytes * 8000000000lu / b;
    fprintf(
        fout,
        "%08x %08x %u %u %lu %lu %lu %lu\n",
        q->sip.Get(),
        q->dip.Get(),
        q->sport,
        q->dport,
        size,
        q->startTime.GetTimeStep(),
        (Simulator::Now() - q->startTime).GetTimeStep(),
        standalone_fct);
    fflush(fout);

    NS_LOG_INFO("MESSAGE FINISH: src " << sid << " did " << did << " port " << q->sport << " size " << size);
    // Ptr<Node> dstNode = n.Get(did);
    // Ptr<RdmaDriver> rdma = dstNode->GetObject<RdmaDriver>();
    // rdma->m_rdma->DeleteRxQp(q->sip.Get(), q->m_pg, q->sport);
#ifdef NS3_MTP
    cs.ExitSection();
#endif
}

int main(int argc, char *argv[])
{
#ifdef NS3_MTP
    MtpInterface::Enable(16);
#endif
#ifdef NS3_MPI
    ns3::MpiInterface::Enable(&argc, &argv);
#endif

    // LogComponentEnable("MSCCL", LOG_LEVEL_INFO);
    LogComponentEnable("MSCCL", LOG_LEVEL_ERROR);

    CommandLine cmd;
    cmd.Parse(argc, argv);
    if (!ReadConf(argc, argv))
        return -1;
    SetConfig();
    SetupNetwork(my_qp_finish, my_send_finish, my_message_finish);
    fflush(stdout);

    //////////////////////////////////////////////////////////////////////////////////////////////
    NS_LOG_INFO(" //////////////////// Start Simulation. //////////////////// ");
    //////////////////////////////////////////////////////////////////////////////////////////////


    Simulator::Run();
    Simulator::Stop(Seconds(simulator_stop_time));
    Simulator::Destroy();

    for (uint32_t i = 0; i < n.GetN(); i++)
    {
        if (n.Get(i)->GetNodeType() == 0)
        {
            // GPU node
            Ptr<GPUNode> gpu = DynamicCast<GPUNode>(n.Get(i));
            if (gpu->m_end_time > Seconds(0))
            {
                std::cout << "GPU rank = " << gpu->GetRank() << " finished, time = "
                << (gpu->m_end_time.GetSeconds() - collective_start_time) << " s" << std::endl;
            }
            else
            {
                // std::cout << "GPU Node " << gpu->GetId() << " (rank " << gpu->GetRank() << ") did not finish all TBs\n";
            }
        }
    }

    return 0;
}
