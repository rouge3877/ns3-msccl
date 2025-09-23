#include <execinfo.h>
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include "common.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#ifdef NS3_MTP
#include "ns3/mtp-interface.h"
#endif
#ifdef NS3_MPI
#include <mpi.h>
#include "ns3/mpi-interface.h"
#endif

using namespace std;
using namespace ns3;

extern uint32_t node_num, switch_num, link_num, trace_num, nvswitch_num, gpus_per_server;
extern std::unordered_map<uint32_t, unordered_map<uint32_t, uint16_t>> portNumber;
extern std::ifstream flowf;
extern FlowInput flow_input;

uint32_t flow_num_finished = 0;

void qp_finish_normal(FILE *fout, Ptr<RdmaQueuePair> q)
{
    uint32_t sid = ip_to_node_id(q->sip), did = ip_to_node_id(q->dip);
#ifdef NS3_MTP
    MtpInterface::explicitCriticalSection cs;
#endif
    Ptr<Node> dstNode = n.Get(did);
    Ptr<RdmaDriver> rdma = dstNode->GetObject<RdmaDriver>();
    rdma->m_rdma->DeleteRxQp(q->sip.Get(), q->m_pg, q->sport);
    std::cout << "at " << Simulator::Now().GetNanoSeconds() << "ns, qp finish, src: " << sid << " did: " << did
              << " port: " << q->sport << std::endl;
#ifdef NS3_MTP
    cs.ExitSection();
#endif
}

void send_finish_normal(FILE *fout, Ptr<RdmaQueuePair> q)
{
    // Seems that the occasion to call this function is error-prone !!! (rouge)
#ifdef NS3_MTP
    MtpInterface::explicitCriticalSection cs;
#endif
    // Currently do nothing
    // uint32_t sid = ip_to_node_id(q->sip), did = ip_to_node_id(q->dip);
#ifdef NS3_MTP
    cs.ExitSection();
#endif
}

void message_finish_normal(FILE *fout, Ptr<RdmaQueuePair> q, uint64_t msgSize)
{
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

    std::cout << "at " << Simulator::Now().GetNanoSeconds() << "ns, message finish, src: " << sid << " did: " << did
              << " port: " << q->sport << " total bytes: " << size << std::endl;
    Ptr<Node> dstNode = n.Get(did);
    Ptr<RdmaDriver> rdma = dstNode->GetObject<RdmaDriver>();
    rdma->m_rdma->DeleteRxQp(q->sip.Get(), q->m_pg, q->sport);
    flow_num_finished++;
    if (flow_num_finished == flow_num)
    {
        cancel_monitor();
    }
}

int main(int argc, char *argv[])
{
#ifdef NS3_MTP
    MtpInterface::Enable(16);
#endif
#ifdef NS3_MPI
    ns3::MpiInterface::Enable(&argc, &argv);
#endif

    LogComponentEnable("GENERIC_SIMULATION", LOG_LEVEL_INFO);
    // LogComponentEnable("RdmaClient", LOG_LEVEL_INFO);
    // LogComponentEnable("RdmaDriver", LOG_LEVEL_INFO);
    // LogComponentEnable("RdmaHw", LOG_LEVEL_INFO);
    // LogComponentEnable("QbbNetDevice", LOG_LEVEL_INFO);

    CommandLine cmd;
    cmd.Parse(argc, argv);
    if (!ReadConf(argc, argv))
        return -1;
    SetConfig();
    SetupNetwork(qp_finish_normal, send_finish_normal, message_finish_normal);
    fflush(stdout);

    //////////////////////////////////////////////////////////////////////////////////////////////
    NS_LOG_INFO(" //////////////////// Running Send And Recv Callback Test. //////////////////// ");
    //////////////////////////////////////////////////////////////////////////////////////////////

    RdmaClientHelper clientHelper0(
        (uint16_t)3, serverAddress[0], serverAddress[1], 10000, 100,
        0, 0, maxRtt, nullptr, nullptr, 1, 0, 1, false, true);
    RdmaClientHelper clientHelper1(
        (uint16_t)3, serverAddress[1], serverAddress[0], 10000, 100,
        0, 0, maxRtt, nullptr, nullptr, 1, 0, 1, false, true);

    ApplicationContainer appCon0 = clientHelper0.Install(n.Get(0));
    ApplicationContainer appCon1 = clientHelper1.Install(n.Get(1));
    uint32_t node0_sip = serverAddress[0].Get();
    uint32_t node1_sip = serverAddress[1].Get();
    uint64_t node0_sender_qp_key = ((uint64_t)node1_sip << 32) | ((uint64_t)10000 << 16) | (uint64_t)3;
    uint64_t node1_sender_qp_key = ((uint64_t)node0_sip << 32) | ((uint64_t)10000 << 16) | (uint64_t)3;

    Ptr<RdmaClient> client0 = DynamicCast<RdmaClient>(appCon0.Get(0));
    Ptr<RdmaClient> client1 = DynamicCast<RdmaClient>(appCon1.Get(0));

    client0->AddTxChannel(node0_sender_qp_key);
    client1->AddTxChannel(node1_sender_qp_key);
    client0->AddRxChannel(std::make_pair(node1_sender_qp_key, node1_sip));
    client1->AddRxChannel(std::make_pair(node0_sender_qp_key, node0_sip));
    
    client0->AddOperation("SEND", 40000000);
    // client0->AddOperation("SEND", 40000);
    // client0->AddOperation("RECV", 0);
    // client1->AddOperation("RECV", 0);
    client1->AddOperation("RECV", 0);
    // client1->AddOperation("SEND", 40000000);
    appCon0.Start(Time(Seconds(2)));
    appCon1.Start(Time(Seconds(2)));

    Simulator::Run();
    Simulator::Stop(Seconds(2000000000));
    Simulator::Destroy();

    return 0;
}
