/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#undef PGO_TRAINING
#define PATH_TO_PGO_CONFIG "path_to_pgo_config"

#include <fstream>
#include <iostream>
#include <time.h>
#include <unordered_map>
#include <vector>

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/error-model.h"
#include "ns3/global-route-manager.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/packet.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/qbb-helper.h"
#include "ns3/allstack-module.h"

#include "ns3/thread-block-step.h"
#include "ns3/thread-block.h"
#include "ns3/gpu-node.h"

#include <ns3/rdma-client-helper.h>
#include <ns3/rdma-client.h>
#include <ns3/rdma-driver.h>
#include <ns3/rdma.h>
#include <ns3/sim-setting.h>
#include <ns3/switch-node.h>
#include <ns3/nvswitch-node.h>
#include <atomic>

#include "config.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("MSCCL");

class ConfigBase;
extern std::unordered_map<std::string, std::unique_ptr<ConfigBase>> config_map;
extern std::unordered_map<std::string, std::unique_ptr<ConfigBase>> config_map_ns3; // 存储用户配置的ns3配置用于读取

/************************************************
 * Config varibles
 ***********************************************/
double simulator_start_time = 2.0;
double simulator_stop_time = 3.01;
double collective_start_time = simulator_start_time;
std::string data_rate, link_delay, topology_file, algo_file, trace_file;
tinyxml2::XMLDocument algo_xml_doc;
std::string trace_output_file = "mix.tr";
std::string fct_output_file = "fct.txt";
std::string pfc_output_file = "pfc.txt";
std::string send_output_file = ""; // default no send output
std::string comm_domain_file = ""; // Communication domain configuration file
std::string mpi_flow_file = ""; // MPI flow configuration file

double error_rate_per_link = 0.0;
uint32_t has_win = 1;
uint32_t global_t = 1;
double pint_log_base = 1.05;
uint32_t int_multi = 1;
uint32_t ack_high_prio = 0;
vector<uint64_t> link_down; // link down time, link down A, link down B
uint32_t enable_trace = 1;
uint32_t buffer_size = 16; // MB

uint32_t qp_mon_interval = 100;     // us, qp_cnp_interval and qp_rate_interval
uint32_t bw_mon_interval = 10000;   // us, bandwidth monitor interval
uint32_t qlen_mon_interval = 10000; // us, queue length monitor interval
uint64_t mon_start = 0, mon_end = 2100000000;

string qlen_mon_file;
string bw_mon_file;
string rate_mon_file;
string cnp_mon_file;

unordered_map<uint64_t, uint32_t> rate2kmax, rate2kmin;
unordered_map<uint64_t, double> rate2pmax;

void InitConfigMap()
{
    /************************************************
     * Value Config
     ***********************************************/
    config_map["SIMULATOR_START_TIME"] = std::make_unique<ConfigVar<double>>(simulator_start_time);
    config_map["SIMULATOR_STOP_TIME"] = std::make_unique<ConfigVar<double>>(simulator_stop_time);
    config_map["COLLECTIVE_START_TIME"] = std::make_unique<ConfigVar<double>>(collective_start_time);
    config_map["DATA_RATE"] = std::make_unique<ConfigVar<std::string>>(data_rate);
    config_map["LINK_DELAY"] = std::make_unique<ConfigVar<std::string>>(link_delay);
    config_map["TOPOLOGY_FILE"] = std::make_unique<ConfigVar<std::string>>(topology_file);
    config_map["ALGO_FILE"] = std::make_unique<ConfigVar<std::string>>(algo_file);
    config_map["TRACE_FILE"] = std::make_unique<ConfigVar<std::string>>(trace_file);
    config_map["COMM_DOMAIN_FILE"] = std::make_unique<ConfigVar<std::string>>(comm_domain_file);
    config_map["MPI_FLOW_FILE"] = std::make_unique<ConfigVar<std::string>>(mpi_flow_file);
    config_map["TRACE_OUTPUT_FILE"] = std::make_unique<ConfigVar<std::string>>(trace_output_file);
    config_map["FCT_OUTPUT_FILE"] = std::make_unique<ConfigVar<std::string>>(fct_output_file);
    config_map["PFC_OUTPUT_FILE"] = std::make_unique<ConfigVar<std::string>>(pfc_output_file);
    config_map["SEND_OUTPUT_FILE"] = std::make_unique<ConfigVar<std::string>>(send_output_file);
    config_map["ERROR_RATE_PER_LINK"] = std::make_unique<ConfigVar<double>>(error_rate_per_link);
    config_map["HAS_WIN"] = std::make_unique<ConfigVar<uint32_t>>(has_win);
    config_map["GLOBAL_T"] = std::make_unique<ConfigVar<uint32_t>>(global_t);
    config_map["PINT_LOG_BASE"] = std::make_unique<ConfigVar<double>>(pint_log_base);
    config_map["INT_MULTI"] = std::make_unique<ConfigVar<uint32_t>>(int_multi);
    config_map["ACK_HIGH_PRIO"] = std::make_unique<ConfigVar<uint32_t>>(ack_high_prio);
    config_map["LINK_DOWN"] = std::make_unique<ConfigVar<vector<uint64_t>>>(link_down);
    config_map["ENABLE_TRACE"] = std::make_unique<ConfigVar<uint32_t>>(enable_trace);
    config_map["BUFFER_SIZE"] = std::make_unique<ConfigVar<uint32_t>>(buffer_size);
    config_map["QP_MON_INTERVAL"] = std::make_unique<ConfigVar<uint32_t>>(qp_mon_interval);
    config_map["BW_MON_INTERVAL"] = std::make_unique<ConfigVar<uint32_t>>(bw_mon_interval);
    config_map["QLEN_MON_INTERVAL"] = std::make_unique<ConfigVar<uint32_t>>(qlen_mon_interval);
    config_map["MON_START"] = std::make_unique<ConfigVar<uint64_t>>(mon_start);
    config_map["MON_END"] = std::make_unique<ConfigVar<uint64_t>>(mon_end);
    config_map["QLEN_MON_FILE"] = std::make_unique<ConfigVar<string>>(qlen_mon_file);
    config_map["BW_MON_FILE"] = std::make_unique<ConfigVar<string>>(bw_mon_file);
    config_map["RATE_MON_FILE"] = std::make_unique<ConfigVar<string>>(rate_mon_file);
    config_map["CNP_MON_FILE"] = std::make_unique<ConfigVar<string>>(cnp_mon_file);
    config_map["KMAX_MAP"] = std::make_unique<ConfigVar<unordered_map<uint64_t, uint32_t>>>(rate2kmax);
    config_map["KMIN_MAP"] = std::make_unique<ConfigVar<unordered_map<uint64_t, uint32_t>>>(rate2kmin);
    config_map["PMAX_MAP"] = std::make_unique<ConfigVar<unordered_map<uint64_t, double>>>(rate2pmax);

    /************************************************
     * NS3 Config
     ***********************************************/
    // QbbNetDevice
    config_map["CHUNK_SIZE"] = std::make_unique<ConfigNs3<uint32_t>>("ns3::ThreadBlock::chunkSize", 1024);
    config_map["ENABLE_QCN"] = std::make_unique<ConfigNs3<bool>>("ns3::QbbNetDevice::QcnEnabled", true);
    config_map["USE_DYNAMIC_PFC_THRESHOLD"] = std::make_unique<ConfigNs3<bool>>("ns3::QbbNetDevice::DynamicThreshold", true);
    config_map["PAUSE_TIME"] = std::make_unique<ConfigNs3<uint32_t>>("ns3::QbbNetDevice::PauseTime", 5);
    // RdmaHw
    config_map["CC_MODE"] = std::make_unique<ConfigNs3<uint32_t>>("ns3::RdmaHw::CcMode", 1);
    config_map["PACKET_PAYLOAD_SIZE"] = std::make_unique<ConfigNs3<uint32_t>>("ns3::RdmaHw::Mtu", 1000);
    config_map["L2_CHUNK_SIZE"] = std::make_unique<ConfigNs3<uint32_t>>("ns3::RdmaHw::L2ChunkSize", 0);
    config_map["L2_ACK_INTERVAL"] = std::make_unique<ConfigNs3<uint32_t>>("ns3::RdmaHw::L2AckInterval", 0);
    config_map["L2_BACK_TO_ZERO"] = std::make_unique<ConfigNs3<bool>>("ns3::RdmaHw::L2BackToZero", false);
    config_map["RATE_AI"] = std::make_unique<ConfigNs3<std::string>>("ns3::RdmaHw::RateAI");
    config_map["RATE_HAI"] = std::make_unique<ConfigNs3<std::string>>("ns3::RdmaHw::RateHAI");
    config_map["MIN_RATE"] = std::make_unique<ConfigNs3<std::string>>(
        "ns3::RdmaHw::MinRate", "100Mb/s");
    config_map["VAR_WIN"] = std::make_unique<ConfigNs3<bool>>("ns3::RdmaHw::VarWin", false);
    config_map["RATE_BOUND"] = std::make_unique<ConfigNs3<bool>>("ns3::RdmaHw::RateBound", true);
    // slightly less than finish time without inefficiency in us
    config_map["NIC_TOTAL_PAUSE_TIME"] = std::make_unique<ConfigNs3<uint32_t>>("ns3::RdmaHw::TotalPauseTime", 0);
    // MellanoxDcqcn
    config_map["ALPHA_RESUME_INTERVAL"] = std::make_unique<ConfigNs3<double>>("ns3::MellanoxDcqcn::AlphaResumInterval", 55.0);
    config_map["RP_TIMER"] = std::make_unique<ConfigNs3<double>>("ns3::MellanoxDcqcn::RPTimer", 0.01);
    config_map["EWMA_GAIN"] = std::make_unique<ConfigNs3<double>>("ns3::MellanoxDcqcn::EwmaGain", 1 / 16);
    config_map["RATE_DECREASE_INTERVAL"] = std::make_unique<ConfigNs3<double>>("ns3::MellanoxDcqcn::RateDecreaseInterval", 4);
    config_map["FAST_RECOVERY_TIMES"] = std::make_unique<ConfigNs3<uint32_t>>("ns3::MellanoxDcqcn::FastRecoveryTimes", 5);
    config_map["CLAMP_TARGET_RATE"] = std::make_unique<ConfigNs3<bool>>("ns3::MellanoxDcqcn::ClampTargetRate", false);
    // Dctcp
    config_map["DCTCP_RATE_AI"] = std::make_unique<ConfigNs3<std::string>>("ns3::Dctcp::DctcpRateAI", "1000Mb/s");
    // Hpcc
    config_map["MI_THRESH"] = std::make_unique<ConfigNs3<uint32_t>>("ns3::Hpcc::MiThresh", 5);
    config_map["FAST_REACT"] = std::make_unique<ConfigNs3<bool>>("ns3::Hpcc::FastReact", true);
    config_map["MULTI_RATE"] = std::make_unique<ConfigNs3<bool>>("ns3::Hpcc::MultiRate", true);
    config_map["SAMPLE_FEEDBACK"] = std::make_unique<ConfigNs3<bool>>("ns3::Hpcc::SampleFeedback", false);
    config_map["U_TARGET"] = std::make_unique<ConfigNs3<double>>("ns3::Hpcc::TargetUtil", 0.95);
    // HpccPint
    config_map["PINT_PROB"] = std::make_unique<ConfigNs3<double>>("ns3::HpccPint::PintProb", 1.0);
}

/************************************************
 * read-from-file varibles
 ***********************************************/
uint32_t node_num, switch_num, link_num, trace_num, nvswitch_num, gpus_per_server;
std::string gpu_type;
std::vector<int> NVswitchs;
std::vector<std::vector<int>> all_gpus;
int ngpus_per_node;

/************************************************
 * Communication Domain configuration
 ***********************************************/
struct CommunicationDomain
{
    uint32_t domain_id;              // 通信域ID
    std::string domain_name;         // 通信域名称（可选，用于可读性）
    std::vector<uint32_t> gpu_ranks; // 包含的GPU rank列表
    uint32_t num_gpus;               // GPU数量

    CommunicationDomain() : domain_id(0), gpu_ranks({}), num_gpus(0) {}
};

std::unordered_map<uint32_t, CommunicationDomain> comm_domains; // domain_id -> domain

/************************************************
 * MPI Flow configuration
 ***********************************************/
struct MPIFlow
{
    uint32_t flow_id;           // 集合通信编号
    std::string algo_file_path; // 算法文件路径
    int depend_on;              // 依赖的flow编号，-1表示立即执行
    uint32_t comm_domain_id;    // 通信域ID
    double start_time;          // 实际开始时间（调度后计算得出）
    bool completed;             // 是否已完成

    MPIFlow() : flow_id(0), depend_on(-1), comm_domain_id(0), start_time(-1.0), completed(false) {}
};

std::vector<MPIFlow> mpi_flows;

/************************************************
 * Connnection
 ***********************************************/
struct Connection // uni-directional connection
{
    uint32_t src;
    uint32_t dst;
    int channel_id; // channel ID for multi-channel links

    Ptr<RdmaClient> sender_client; // RdmaClient for sender
    Ptr<RdmaClient> receiver_client; // RdmaClient for receiver
};

std::string getConnKey(const Connection& conn)
{
    return std::to_string(conn.src) + "->" + std::to_string(conn.dst) +
           " chan:" + std::to_string(conn.channel_id);
}

std::string getConnKey(uint32_t src, uint32_t dst, int channel_id)
{
    return std::to_string(src) + "->" + std::to_string(dst) +
           " chan:" + std::to_string(channel_id);
}

std::vector<Connection> all_connections;
std::map<std::string, uint32_t> key2conn_id; // connection key -> connection ID
uint16_t priority_group = 3;

// Forward declarations (after struct definitions)
CommunicationDomain* GetCommDomain(uint32_t domain_id);
bool SetupAlgorithmFromXML(const std::string& xml_file_path, double start_time, const std::vector<uint32_t>& gpu_ranks);
void SetupConnection(Connection& conn);
extern NodeContainer n;

void InitAllConnections()
{
    all_connections.clear();
    key2conn_id.clear();
}

Connection AddConnection(uint32_t src, uint32_t dst, int channel_id)
{

    if (key2conn_id.find(getConnKey(src, dst, channel_id)) != key2conn_id.end())
    {
        NS_LOG_INFO("Connection " << getConnKey(src, dst, channel_id) << " already exists!");
        return all_connections[key2conn_id[getConnKey(src, dst, channel_id)]];
    }

    NS_LOG_INFO("Adding Connection " << getConnKey(src, dst, channel_id));

    Connection conn;
    conn.src = src;
    conn.dst = dst;
    conn.channel_id = channel_id;
    all_connections.push_back(conn);
    uint32_t conn_idx = all_connections.size() - 1;
    key2conn_id[getConnKey(conn)] = conn_idx;
    NS_LOG_INFO("New connection ID: " << conn_idx << ", of conn key: " << getConnKey(conn));

    SetupConnection(all_connections[conn_idx]);
    return all_connections[conn_idx];
}


/**
 * \brief Callback when an MPI flow completes
 * \param flow_id The ID of the completed flow
 */
void OnMPIFlowComplete(uint32_t flow_id)
{
    NS_LOG_INFO("MPI Flow " << flow_id << " completed at time " << Simulator::Now().GetSeconds() << "s");
    
    if (flow_id >= mpi_flows.size())
    {
        NS_LOG_ERROR("Invalid flow_id: " << flow_id);
        return;
    }
    
    mpi_flows[flow_id].completed = true;
    
    // Check if any flows are waiting for this flow to complete
    for (uint32_t i = 0; i < mpi_flows.size(); i++)
    {
        if (mpi_flows[i].depend_on == (int)flow_id && !mpi_flows[i].completed && mpi_flows[i].start_time < 0)
        {
            // Get communication domain for this flow
            CommunicationDomain* domain = GetCommDomain(mpi_flows[i].comm_domain_id);
            if (domain == nullptr)
            {
                NS_LOG_ERROR("Communication domain " << mpi_flows[i].comm_domain_id << " not found for Flow " << i);
                continue;
            }
            
            // Schedule this dependent flow to start immediately
            double start_time = Simulator::Now().GetSeconds();
            mpi_flows[i].start_time = start_time;
            NS_LOG_INFO("Scheduling dependent MPI Flow " << i << " at time " << start_time 
                       << "s (comm_domain=" << mpi_flows[i].comm_domain_id << ", " << domain->domain_name << ")");

            bool success = SetupAlgorithmFromXML(mpi_flows[i].algo_file_path, start_time, domain->gpu_ranks);
            if (!success)
            {
                NS_LOG_ERROR("Failed to setup MPI Flow " << i);
            }
        }
    }
}

/**
 * \brief Setup all MPI flows based on dependencies
 * \return true if setup successful, false otherwise
 */
bool SetupMPIFlows()
{
    if (mpi_flows.empty())
    {
        NS_LOG_INFO("No MPI flows to setup");
        return false;
    }
    
    NS_LOG_INFO("Setting up " << mpi_flows.size() << " MPI flows");
    
    for (uint32_t i = 0; i < mpi_flows.size(); i++)
    {
        if (mpi_flows[i].depend_on == -1)
        {
            // Get communication domain for this flow
            CommunicationDomain* domain = GetCommDomain(mpi_flows[i].comm_domain_id);
            if (domain == nullptr)
            {
                NS_LOG_ERROR("Communication domain " << mpi_flows[i].comm_domain_id << " not found for Flow " << i);
                return false;
            }

            mpi_flows[i].start_time = collective_start_time;
            NS_LOG_INFO("Scheduling independent MPI Flow " << i << " at time " << collective_start_time 
                       << "s (comm_domain=" << mpi_flows[i].comm_domain_id << ", " << domain->domain_name << ")");

            bool success = SetupAlgorithmFromXML(mpi_flows[i].algo_file_path, collective_start_time, domain->gpu_ranks);
            if (!success)
            {
                NS_LOG_ERROR("Failed to setup MPI Flow " << i);
                return false;
            } else {
                NS_LOG_INFO("MPI Flow " << i << " setup successfully");
            }
        }
    }
    
    return true;
}

/**
 * \brief Read MPI flow configuration file
 * \param mpi_flow_file_path Path to the MPI flow configuration file
 * \return true if read successful, false otherwise
 */
bool ReadMPIFlowFile(const std::string& mpi_flow_file_path)
{
    if (mpi_flow_file_path.empty())
    {
        NS_LOG_INFO("No MPI flow file specified, using single XML mode");
        return false;
    }

    std::ifstream mpi_flow_stream;
    mpi_flow_stream.open(mpi_flow_file_path.c_str());
    if (!mpi_flow_stream.is_open())
    {
        NS_LOG_ERROR("Failed to open MPI flow file: " << mpi_flow_file_path);
        return false;
    }

    uint32_t num_flows;
    mpi_flow_stream >> num_flows;
    
    NS_LOG_INFO("Reading " << num_flows << " MPI flows from: " << mpi_flow_file_path);
    
    mpi_flows.clear();
    mpi_flows.reserve(num_flows);

    for (uint32_t i = 0; i < num_flows; i++)
    {
        MPIFlow flow;
        mpi_flow_stream >> flow.flow_id >> flow.algo_file_path >> flow.depend_on >> flow.comm_domain_id;
        
        if (mpi_flow_stream.fail())
        {
            NS_LOG_ERROR("Failed to parse MPI flow at line " << (i + 2));
            mpi_flow_stream.close();
            return false;
        }

        if (flow.flow_id != i)
        {
            NS_LOG_WARN("Flow ID " << flow.flow_id << " does not match expected index " << i);
        }

        // 验证依赖关系
        if (flow.depend_on >= (int)flow.flow_id && flow.depend_on != -1)
        {
            NS_LOG_ERROR("Invalid dependency: Flow " << flow.flow_id << " depends on Flow " << flow.depend_on 
                        << " (circular or forward dependency)");
            mpi_flow_stream.close();
            return false;
        }

        // 验证通信域是否存在
        CommunicationDomain* domain = GetCommDomain(flow.comm_domain_id);
        if (domain == nullptr)
        {
            NS_LOG_ERROR("Invalid comm_domain_id: " << flow.comm_domain_id << " for Flow " << flow.flow_id);
            mpi_flow_stream.close();
            return false;
        }

        mpi_flows.push_back(flow);
        NS_LOG_INFO("MPI Flow " << flow.flow_id << ": algo=" << flow.algo_file_path 
                   << ", depend=" << flow.depend_on << ", comm_domain=" << flow.comm_domain_id 
                   << " (" << domain->domain_name << ")");
    }

    mpi_flow_stream.close();
    return true;
}

/**
 * \brief Setup algorithm configuration from XML file
 * \param xml_file_path Path to the XML algorithm file
 * \param start_time The start time for collective operations (in seconds)
 * \param gpu_ranks List of GPU ranks involved in this algorithm
 * \return true if setup successful, false otherwise
 * 
 * This function loads an XML algorithm file and configures:
 * 1. GPU nodes with their ranks (with offset)
 * 2. GPUThreadBlock applications
 * 3. RDMA client connections for communication
 */
bool SetupAlgorithmFromXML(const std::string& xml_file_path, double start_time = collective_start_time, const std::vector<uint32_t>& gpu_ranks = {})
{
    NS_LOG_INFO("Setting up algorithm from XML file: " << xml_file_path);

    // Load XML file
    tinyxml2::XMLDocument xml_doc;
    auto xml_load_result = xml_doc.LoadFile(xml_file_path.c_str());
    if (xml_load_result != tinyxml2::XML_SUCCESS)
    {
        NS_LOG_ERROR("Failed to open algo xml file: " << xml_file_path);
        return false;
    }

    tinyxml2::XMLElement *algo = xml_doc.FirstChildElement("algo");
    if (algo == nullptr)
    {
        NS_LOG_ERROR("No <algo> element found in algo xml file: " << xml_file_path);
        return false;
    }
    
    // First pass: build a map of GPU rank -> XMLElement
    std::unordered_map<uint32_t, tinyxml2::XMLElement*> rank_to_gpu_xml;
    auto gpu = algo->FirstChildElement("gpu");
    while (gpu != nullptr)
    {
        uint32_t gpu_rank = gpu->IntAttribute("id");
        rank_to_gpu_xml[gpu_rank] = gpu;
        gpu = gpu->NextSiblingElement("gpu");
    }

    // Second pass: map rank -> Node
    //      Setup Communication Domain
    std::unordered_map<uint32_t, std::pair<Ptr<GPUNode>, uint32_t>> rank2node_peer; // rank -> (Node, node_id)
    for (auto i = gpu_ranks.begin(); i != gpu_ranks.end(); ++i)
    {
        uint32_t rank = *i;
        uint32_t node_id = rank;  // In this system, rank directly corresponds to node_id
        Ptr<Node> node = n.Get(node_id);
        Ptr<GPUNode> gpu_node = DynamicCast<GPUNode>(node);
        rank2node_peer[rank] = make_pair(gpu_node, node_id);
        if (gpu_node == nullptr)
        {
            NS_LOG_ERROR("Invalid node_id " << node_id << " for rank " << rank << " (node is not a GPUNode)");
            return false;
        } 
    }

    // Third pass: assign ranks to GPU nodes and install applications with rank offset
    NodeContainer gpu_nodes{0};
    for (uint32_t i = 0; i < node_num; i++)
    {
        if (n.Get(i)->GetNodeType() == 0)
        {
            gpu_nodes = NodeContainer(gpu_nodes, n.Get(i));
        }
    }

    // Fourth pass: Install applications based on XML configuration
    for (auto it = rank_to_gpu_xml.begin(); it != rank_to_gpu_xml.end(); ++it)
    {
        uint32_t xml_rank = it->first;  // Rank defined in XML file (0, 1, 2, ...)
        tinyxml2::XMLElement* gpu_xml = it->second;

        uint32_t rank_node_id = rank2node_peer[xml_rank].second;

        Ptr<GPUNode> gpu_node = DynamicCast<GPUNode>(gpu_nodes.Get(rank_node_id));
        gpu_node->SetRank(xml_rank);

        // Find the actual node ID in the global node container
        uint32_t node_id = gpu_node->GetId();

        GPUThreadBlockHelper helper(gpu_xml);
        ApplicationContainer apps = helper.Install(gpu_node);
        apps.Start(Seconds(start_time));
    }

    NS_LOG_INFO("Algorithm has been configured from XML: " << xml_file_path);

    // Setup RDMA connections for GPU nodes with reuse support
    // Using global priority_group variable

    for (uint32_t i = 0; i < gpu_nodes.GetN(); i++)
    {
        Ptr<GPUNode> this_gpu = DynamicCast<GPUNode>(gpu_nodes.Get(i));
        
        // Skip GPU nodes without thread blocks
        if (this_gpu->GetNThreadBlocks() == 0)
            continue;

        // List all threadblock in this gpu
        for (uint32_t j = 0; j < this_gpu->GetNThreadBlocks(); j++)
        {
            Ptr<ThreadBlock> tb = this_gpu->GetThreadBlock(j);
            int send_peer = tb->GetSend();
            int recv_peer = tb->GetRecv();
            int channel = tb->GetChannel();

            uint32_t send_peer_node = (send_peer == -1) ? UINT32_MAX : rank2node_peer[send_peer].second;
            uint32_t recv_peer_node = (recv_peer == -1) ? UINT32_MAX : rank2node_peer[recv_peer].second;
            uint32_t this_node = rank2node_peer[this_gpu->GetRank()].second;

            Connection send_conn = AddConnection(this_node, send_peer_node, channel);
            Connection recv_conn = AddConnection(recv_peer_node, this_node, channel);
            
            NS_LOG_INFO("Binding TB (rank " << this_gpu->GetRank() << ", node " << this_node 
                        << ") send to rank " << send_peer << " (node " << send_peer_node 
                        << "), recv from rank " << recv_peer << " (node " << recv_peer_node 
                        << "), channel " << channel);
            NS_LOG_INFO("  Send Conn Key: " << getConnKey(send_conn) << " , Sender Client Ptr: " << send_conn.sender_client);
            NS_LOG_INFO("  Recv Conn Key: " << getConnKey(recv_conn) << " , Receiver Client Ptr: " << recv_conn.receiver_client);
            tb->BindRdmaClients(send_conn.sender_client, recv_conn.receiver_client);          
        }
    }

    NS_LOG_INFO("RDMA connections have been setup for algorithm from XML: " << xml_file_path);
    return true;
}

/**
 * \brief Read communication domain configuration file
 * \param comm_domain_file_path Path to the communication domain configuration file
 * \return true if read successful, false otherwise
 * 
 * File format:
 * <num_domains>
 * <domain_id> <domain_name> <num_gpus> <rank_0> <rank_1> ... <rank_n>
 * or
 * <domain_id> <domain_name> <rank_start>-<rank_end>
 */
bool ReadCommDomainFile(const std::string& comm_domain_file_path)
{
    if (comm_domain_file_path.empty())
    {
        NS_LOG_INFO("No communication domain file specified, using default single domain");
        return false;
    }

    std::ifstream comm_domain_stream;
    comm_domain_stream.open(comm_domain_file_path.c_str());
    if (!comm_domain_stream.is_open())
    {
        NS_LOG_ERROR("Failed to open communication domain file: " << comm_domain_file_path);
        return false;
    }

    uint32_t num_domains;
    comm_domain_stream >> num_domains;

    NS_LOG_INFO("Reading " << num_domains << " communication domains from: " << comm_domain_file_path);
    
    comm_domains.clear();

    for (uint32_t i = 0; i < num_domains; i++)
    {
        CommunicationDomain domain;
        comm_domain_stream >> domain.domain_id >> domain.domain_name;
        
        std::string gpu_spec;
        comm_domain_stream >> gpu_spec;

        // start-end :=> [start_time, end_time]
        if (gpu_spec.find('-') != std::string::npos)
        {
            size_t dash_pos = gpu_spec.find('-');
            uint32_t start_rank = std::stoi(gpu_spec.substr(0, dash_pos));
            uint32_t end_rank = std::stoi(gpu_spec.substr(dash_pos + 1));
            for (uint32_t rank = start_rank; rank <= end_rank; rank++)
            {
                domain.gpu_ranks.push_back(rank);
            }
            domain.num_gpus = end_rank - start_rank + 1;
            
            
            NS_LOG_INFO("Communication Domain " << domain.domain_id << " (" << domain.domain_name 
                       << "): ranks " << start_rank << "-" << end_rank 
                       << " (" << domain.num_gpus << " GPUs)");
        }
        else
        {
            // list: num_gpus rank0 rank1 rank2 ...
            domain.num_gpus = std::stoi(gpu_spec);
            domain.gpu_ranks.reserve(domain.num_gpus);
            
            for (uint32_t j = 0; j < domain.num_gpus; j++)
            {
                uint32_t rank;
                comm_domain_stream >> rank;
                domain.gpu_ranks.push_back(rank);
            }
            
            NS_LOG_INFO("Communication Domain " << domain.domain_id << " (" << domain.domain_name 
                       << "): " << domain.num_gpus << " GPUs, starting at rank " << domain.gpu_ranks[0]);
        }
        
        if (comm_domain_stream.fail())
        {
            NS_LOG_ERROR("Failed to parse communication domain at line " << (i + 2));
            comm_domain_stream.close();
            return false;
        }

        if (comm_domains.find(domain.domain_id) != comm_domains.end())
        {
            NS_LOG_ERROR("Duplicate domain_id: " << domain.domain_id);
            comm_domain_stream.close();
            return false;
        }

        comm_domains[domain.domain_id] = domain;
    }

    comm_domain_stream.close();
    return true;
}

/**
 * \brief Get communication domain by ID
 * \param domain_id The domain ID to lookup
 * \return Pointer to the domain, or nullptr if not found
 */
CommunicationDomain* GetCommDomain(uint32_t domain_id)
{
    auto it = comm_domains.find(domain_id);
    if (it != comm_domains.end())
    {
        return &(it->second);
    }
    return nullptr;
}


/************************************************
 * monitor varibles
 ***********************************************/
// seems useless, but actually exsits in Original source
// string total_algo_file = "";
// FILE *total_algo_output = nullptr;

EventId monitor_qlen_event;
EventId monitor_bw_event;
EventId monitor_qp_rate_event;
EventId monitor_qp_cnp_num_event;

/************************************************
 * Runtime varibles
 ***********************************************/
std::ifstream topof, algof, tracef;

NodeContainer n;

uint64_t nic_rate;

uint64_t maxRtt, maxBdp;

std::vector<Ipv4Address> serverAddress;

// maintain port number for each host pair
std::unordered_map<uint32_t, unordered_map<uint32_t, uint16_t>> portNumber;

struct Interface
{
    uint32_t idx;
    bool up;
    uint64_t delay;
    uint64_t bw;

    Interface() : idx(0), up(false) {}
};
map<Ptr<Node>, map<Ptr<Node>, Interface>> nbr2if;
// Mapping destination to next hop for each node: <node, <dest, <nexthop0, ...>
// > >
map<Ptr<Node>, map<Ptr<Node>, vector<Ptr<Node>>>> nextHop;
map<Ptr<Node>, map<Ptr<Node>, uint64_t>> pairDelay;
map<Ptr<Node>, map<Ptr<Node>, uint64_t>> pairTxDelay;
map<uint32_t, map<uint32_t, uint64_t>> pairBw;
map<Ptr<Node>, map<Ptr<Node>, uint64_t>> pairBdp;
map<uint32_t, map<uint32_t, uint64_t>> pairRtt;

map<int, Ptr<Node>> rank2node;
// map<int, pair<Ptr<Node>, uint32_t>> rank2node_peer; // pair<node, number>

Ipv4Address node_id_to_ip(uint32_t id)
{
    return Ipv4Address(0x0b000001 + ((id / 256) * 0x00010000) +
                       ((id % 256) * 0x00000100));
}

uint32_t ip_to_node_id(Ipv4Address ip) { return (ip.Get() >> 8) & 0xffff; }

void get_pfc(FILE *fout, Ptr<QbbNetDevice> dev, uint32_t type)
{
    fprintf(fout, "%lu %u %u %u %u\n", Simulator::Now().GetTimeStep(),
            dev->GetNode()->GetId(), dev->GetNode()->GetNodeType(),
            dev->GetIfIndex(), type);
}

struct QlenDistribution
{
    vector<uint32_t>
        cnt; // cnt[i] is the number of times that the queue len is i KB

    void add(uint32_t qlen)
    {
        uint32_t kb = qlen / 1000;
        if (cnt.size() < kb + 1)
            cnt.resize(kb + 1);
        cnt[kb]++;
    }
};

void monitor_qlen(FILE *qlen_output, NodeContainer *n)
{
    for (uint32_t i = 0; i < n->GetN(); i++)
    {
        if (n->Get(i)->GetNodeType() == 1)
        { // is switch
            Ptr<SwitchNode> sw = DynamicCast<SwitchNode>(n->Get(i));
            sw->PrintSwitchQlen(qlen_output);
        }
        else if (n->Get(i)->GetNodeType() == 2)
        { // is nvswitch
            Ptr<NVSwitchNode> sw = DynamicCast<NVSwitchNode>(n->Get(i));
            sw->PrintSwitchQlen(qlen_output);
        }
    }
    if (qlen_mon_interval + Simulator::Now().GetMicroSeconds() < mon_end)
    {
        monitor_qlen_event = Simulator::Schedule(MicroSeconds(qlen_mon_interval), &monitor_qlen, qlen_output, n);
    }
}
void monitor_bw(FILE *bw_output, NodeContainer *n)
{
    for (uint32_t i = 0; i < n->GetN(); i++)
    {
        if (n->Get(i)->GetNodeType() == 1)
        { // is switch
            Ptr<SwitchNode> sw = DynamicCast<SwitchNode>(n->Get(i));
            sw->PrintSwitchBw(bw_output, bw_mon_interval);
        }
        else if (n->Get(i)->GetNodeType() == 2)
        { // is nvswitch
            Ptr<NVSwitchNode> sw = DynamicCast<NVSwitchNode>(n->Get(i));
            sw->PrintSwitchBw(bw_output, bw_mon_interval);
        }
        else
        { // is host
            Ptr<Node> host = n->Get(i);
            host->GetObject<RdmaDriver>()->m_rdma->PrintHostBW(bw_output, bw_mon_interval);
        }
    }
    if (bw_mon_interval + Simulator::Now().GetMicroSeconds() < mon_end)
    {
        monitor_bw_event = Simulator::Schedule(MicroSeconds(bw_mon_interval), &monitor_bw, bw_output, n);
    }
}
void monitor_qp_rate(FILE *rate_output, NodeContainer *n)
{
    for (uint32_t i = 0; i < n->GetN(); i++)
    {
        if (n->Get(i)->GetNodeType() == 0)
        { // is host
            Ptr<Node> host = n->Get(i);
            host->GetObject<RdmaDriver>()->m_rdma->PrintQPRate(rate_output);
        }
    }
    if (qlen_mon_interval + Simulator::Now().GetMicroSeconds() < mon_end)
    {
        monitor_qp_rate_event = Simulator::Schedule(MicroSeconds(qp_mon_interval), &monitor_qp_rate, rate_output, n);
    }
}
void monitor_qp_cnp_number(FILE *cnp_output, NodeContainer *n)
{
    for (uint32_t i = 0; i < n->GetN(); i++)
    {
        if (n->Get(i)->GetNodeType() == 0)
        { // is host
            Ptr<Node> host = n->Get(i);
            host->GetObject<RdmaDriver>()->m_rdma->PrintQPCnpNumber(cnp_output);
        }
    }
    if (qp_mon_interval + Simulator::Now().GetMicroSeconds() < mon_end)
    {
        monitor_qp_cnp_num_event = Simulator::Schedule(MicroSeconds(qp_mon_interval), &monitor_qp_cnp_number, cnp_output, n);
    }
}
void schedule_monitor()
{
    // queue length monitor
    FILE *qlen_output = fopen(qlen_mon_file.c_str(), "w");
    NS_ASSERT_MSG(qlen_output != nullptr, "qlen_output is nullptr");
    fprintf(qlen_output, "%s, %s, %s, %s, %s, %s\n", "time", "sw_id", "port_id", "q_id", "q_len", "port_len");
    fflush(qlen_output);
    monitor_qlen_event = Simulator::Schedule(MicroSeconds(mon_start), &monitor_qlen, qlen_output, &n);

    // bandwidth monitor
    FILE *bw_output = fopen(bw_mon_file.c_str(), "w");
    NS_ASSERT_MSG(bw_output != nullptr, "bw_output is nullptr");
    fprintf(bw_output, "%s, %s, %s, %s\n", "time", "node_id", "port_id", "bandwidth");
    fflush(bw_output);
    monitor_bw_event = Simulator::Schedule(MicroSeconds(mon_start), &monitor_bw, bw_output, &n);

    // qp send rate monitor
    FILE *rate_output = fopen(rate_mon_file.c_str(), "w");
    NS_ASSERT_MSG(rate_output != nullptr, "rate_output is nullptr");
    fprintf(rate_output, "%s, %s, %s, %s, %s, %s, %s\n", "time", "src", "dst", "sport", "dport", "size", "curr_rate");
    fflush(rate_output);
    monitor_qp_rate_event = Simulator::Schedule(MicroSeconds(mon_start), &monitor_qp_rate, rate_output, &n);

    // the number of cnp monitor
    FILE *cnp_output = fopen(cnp_mon_file.c_str(), "w");
    NS_ASSERT_MSG(cnp_output != nullptr, "cnp_output is nullptr");
    fprintf(cnp_output, "%s, %s, %s, %s, %s, %s, %s\n", "time", "src", "dst", "sport", "dport", "size", "cnp_number");
    fflush(cnp_output);
    monitor_qp_cnp_num_event = Simulator::Schedule(MicroSeconds(mon_start), &monitor_qp_cnp_number, cnp_output, &n);
}

std::unordered_map<std::string, Ptr<RdmaClient>> connection_map; // key is src_dst_pg_channel
inline std::string getHashKey(uint32_t src, uint32_t dst, uint16_t pg, int channel)
{
    return std::to_string(src) + '_' + std::to_string(dst) + '_' + std::to_string(pg) + '_' + std::to_string(channel);
}

void cancel_monitor()
{
    Simulator::Cancel(monitor_qlen_event);
    Simulator::Cancel(monitor_bw_event);
    Simulator::Cancel(monitor_qp_rate_event);
    Simulator::Cancel(monitor_qp_cnp_num_event);
}

void CalculateRoute(Ptr<Node> host)
{
    // queue for the BFS.
    vector<Ptr<Node>> q;
    // Distance from the host to each node.
    map<Ptr<Node>, int> dis;
    map<Ptr<Node>, uint64_t> delay;
    map<Ptr<Node>, uint64_t> txDelay;
    map<Ptr<Node>, uint64_t> bw;
    // init BFS.
    q.push_back(host);
    dis[host] = 0;
    delay[host] = 0;
    txDelay[host] = 0;
    bw[host] = 0xfffffffffffffffflu;
    uint32_t payload = get_config_value_ns3<uint64_t>("ns3::RdmaHw::Mtu");
    // BFS.
    // std::cout << endl << "***************** CALCULATING HOST: " << host->GetId() << "'s ROUTING ENTRIES! *****************" << std::endl;
    for (int i = 0; i < (int)q.size(); i++)
    {
        Ptr<Node> now = q[i];
        int d = dis[now];
        for (auto it = nbr2if[now].begin(); it != nbr2if[now].end(); it++)
        {
            // skip down link
            if (!it->second.up)
                continue;
            Ptr<Node> next = it->first; // neighbor
            // std::cout << "Now: " << now->GetId() << " , next: " << next->GetId() << std::endl;
            if (dis.find(next) == dis.end())
            {
                dis[next] = d + 1;
                delay[next] = delay[now] + it->second.delay;
                txDelay[next] =
                    txDelay[now] + payload * 1000000000lu * 8 / it->second.bw;
                bw[next] = std::min(bw[now], it->second.bw);
                // 因为会给每个NodeType() == 0的节点计算一次CalculateRoute()，所以不需要加入队列了
                // 这样只会考虑中间设备全部都为交换机
                if (next->GetNodeType() == 1 || next->GetNodeType() == 2)
                {
                    // std::cout << "push back node: " << next->GetId() << "and node type is: " << next->GetNodeType() << std::endl;
                    q.push_back(next);
                }
            }
            bool via_nvswitch = false;
            if (d + 1 == dis[next])
            {
                // std::cout << "nexthop push back node: " <<  now->GetId() << std::endl;
                // host ----> now -> next
                for (auto x : nextHop[next][host])
                {
                    if (x->GetNodeType() == 2)
                        via_nvswitch = true;
                }
                if (via_nvswitch == false)
                {
                    if (now->GetNodeType() == 2)
                    {
                        // 路由表优先
                        while (nextHop[next][host].size() != 0)
                            nextHop[next][host].pop_back();
                    }
                    nextHop[next][host].push_back(now);
                }
                else if (via_nvswitch == true && now->GetNodeType() == 2)
                {
                    nextHop[next][host].push_back(now);
                }
                // add to switch routing entries
                if (next->GetNodeType() == 0 && nextHop[next][now].size() == 0)
                {
                    nextHop[next][now].push_back(now);
                    pairBw[next->GetId()][now->GetId()] = pairBw[now->GetId()][next->GetId()] = it->second.bw;
                }
            }
        }
    }
    for (auto it : delay)
    {
        // std::cout << "pairDelay first "<< it.first->GetId() << " host " <<
        // host->GetId() << " delay " << it.second << std::endl;
        pairDelay[it.first][host] = it.second;
    }
    for (auto it : txDelay)
        pairTxDelay[it.first][host] = it.second;
    for (auto it : bw)
    {
        // std::cout << "pairBw first "<< it.first->GetId() << " host " <<
        // host->GetId() << " bw " << it.second << std::endl;
        pairBw[it.first->GetId()][host->GetId()] = it.second;
    }

    // std::cout << endl << "***************** DONE CALCULATING HOST: " << host << "'s ROUTING ENTRIES! *****************" << std::endl;
}

void CalculateRoutes(NodeContainer &n)
{
    for (int i = 0; i < (int)n.GetN(); i++)
    {
        Ptr<Node> node = n.Get(i);
        if (node->GetNodeType() == 0)
            CalculateRoute(node);
    }
}

void SetRoutingEntries()
{
    for (auto i = nextHop.begin(); i != nextHop.end(); i++)
    {
        Ptr<Node> node = i->first;
        auto &table = i->second;
        for (auto j = table.begin(); j != table.end(); j++)
        {
            Ptr<Node> dst = j->first;
            Ipv4Address dstAddr = dst->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
            vector<Ptr<Node>> nexts = j->second;
            for (int k = 0; k < (int)nexts.size(); k++)
            {
                Ptr<Node> next = nexts[k];
                uint32_t interface = nbr2if[node][next].idx;
                if (node->GetNodeType() == 1)
                {
                    // ECMP per flow
                    DynamicCast<SwitchNode>(node)->AddTableEntry(dstAddr, interface);
                }
                else if (node->GetNodeType() == 2)
                {
                    DynamicCast<NVSwitchNode>(node)->AddTableEntry(dstAddr, interface);
                    // add nvswitch rdma driver routing table entries
                    node->GetObject<RdmaDriver>()->m_rdma->AddTableEntry(dstAddr, interface, true);
                }
                else
                {
                    // node->GetNodeType() == 0
                    bool is_nvswitch = false;
                    if (next->GetNodeType() == 2)
                    { // the next hop is nvswitch
                        is_nvswitch = true;
                    }
                    node->GetObject<RdmaDriver>()->m_rdma->AddTableEntry(dstAddr, interface, is_nvswitch);
                    // let GPU know NVSwitch id.
                    if (next->GetId() == dst->GetId())
                    {
                        // std::cout << "src: " << node->GetId() << " to dst:  " << dst->GetId() << " via " << next->GetId() << std::endl;
                        node->GetObject<RdmaDriver>()->m_rdma->add_nvswitch(dst->GetId());
                    }
                }
            }
        }
    }
}

void printRoutingEntries()
{
    map<uint32_t, string> types;
    types[0] = "HOST";
    types[1] = "SWITCH";
    types[2] = "NVSWITCH";
    // vector<Node> NVSwitch;
    // vector<Node> ASW, PSW;
    map<Ptr<Node>, map<Ptr<Node>, vector<pair<Ptr<Node>, uint32_t>>>> NVSwitch, NetSwitch, Host; // src -> dst: vector<nextHop, port number>
    for (auto i = nextHop.begin(); i != nextHop.end(); i++)
    {
        Ptr<Node> src = i->first;
        auto &table = i->second;
        for (auto j = table.begin(); j != table.end(); j++)
        {
            Ptr<Node> dst = j->first;
            Ipv4Address dstAddr = dst->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
            vector<Ptr<Node>> nexts = j->second;
            for (int k = 0; k < (int)nexts.size(); k++)
            {
                // nexts are the first hop swtich, maybe nvswitch or asw
                Ptr<Node> firstHop = nexts[k];
                uint32_t interface = nbr2if[src][firstHop].idx;
                if (src->GetNodeType() == 0)
                {
                    // host
                    Host[src][dst].push_back(pair<Ptr<Node>, uint32_t>(firstHop, interface));
                }
                else if (src->GetNodeType() == 1)
                {
                    // asw | psw
                    NetSwitch[src][dst].push_back(pair<Ptr<Node>, uint32_t>(firstHop, interface));
                }
                else if (src->GetNodeType() == 2)
                {
                    NVSwitch[src][dst].push_back(pair<Ptr<Node>, uint32_t>(firstHop, interface));
                }
            }
        }
    }

    cout << "*********************    PRINT SWITCH ROUTING TABLE    *********************" << endl
         << endl
         << endl;
    for (auto it = NetSwitch.begin(); it != NetSwitch.end(); ++it)
    {
        Ptr<Node> src = it->first;
        auto table = it->second;
        cout << "SWITCH: " << src->GetId() << "'s routing entries are as follows:" << endl;
        for (auto j = table.begin(); j != table.end(); ++j)
        {
            Ptr<Node> dst = j->first;
            auto entries = j->second;
            for (auto k = entries.begin(); k != entries.end(); ++k)
            {
                Ptr<Node> nextHop = k->first;
                uint32_t interface = k->second;
                cout << "To " << dst->GetId() << "[" << types[dst->GetNodeType()] << "] via " << nextHop->GetId() << "[" << types[nextHop->GetNodeType()] << "]" << " from port: " << interface << endl;
            }
        }
    }

    cout << "*********************    PRINT NVSWITCH ROUTING TABLE    *********************" << endl
         << endl
         << endl;
    for (auto it = NVSwitch.begin(); it != NVSwitch.end(); ++it)
    {
        Ptr<Node> src = it->first;
        auto table = it->second;
        cout << "NVSWITCH: " << src->GetId() << "'s routing entries are as follows:" << endl;
        for (auto j = table.begin(); j != table.end(); ++j)
        {
            Ptr<Node> dst = j->first;
            auto entries = j->second;
            for (auto k = entries.begin(); k != entries.end(); ++k)
            {
                Ptr<Node> nextHop = k->first;
                uint32_t interface = k->second;
                cout << "To " << dst->GetId() << "[" << types[dst->GetNodeType()] << "] via " << nextHop->GetId() << "[" << types[nextHop->GetNodeType()] << "]" << " from port: " << interface << endl;
            }
        }
    }

    cout << "*********************    HOST ROUTING TABLE    *********************" << endl
         << endl
         << endl;
    for (auto it = Host.begin(); it != Host.end(); ++it)
    {
        Ptr<Node> src = it->first;
        auto table = it->second;
        cout << "HOST: " << src->GetId() << "'s routing entries are as follows:" << endl;
        for (auto j = table.begin(); j != table.end(); ++j)
        {
            Ptr<Node> dst = j->first;
            auto entries = j->second;
            for (auto k = entries.begin(); k != entries.end(); ++k)
            {
                Ptr<Node> nextHop = k->first;
                uint32_t interface = k->second;
                cout << "To " << dst->GetId() << "[" << types[dst->GetNodeType()] << "] via " << nextHop->GetId() << "[" << types[nextHop->GetNodeType()] << "]" << " from port: " << interface << endl;
            }
        }
    }
}

bool validateRoutingEntries()
{

    return false;
}

// take down the link between a and b, and redo the routing
void TakeDownLink(NodeContainer n, Ptr<Node> a, Ptr<Node> b)
{
    if (!nbr2if[a][b].up)
        return;
    // take down link between a and b
    nbr2if[a][b].up = nbr2if[b][a].up = false;
    nextHop.clear();
    CalculateRoutes(n);
    // clear routing tables
    for (uint32_t i = 0; i < n.GetN(); i++)
    {
        if (n.Get(i)->GetNodeType() == 1)
            DynamicCast<SwitchNode>(n.Get(i))->ClearTable();
        else if (n.Get(i)->GetNodeType() == 2)
            DynamicCast<NVSwitchNode>(n.Get(i))->ClearTable();
        else
            n.Get(i)->GetObject<RdmaDriver>()->m_rdma->ClearTable();
    }
    DynamicCast<QbbNetDevice>(a->GetDevice(nbr2if[a][b].idx))->TakeDown();
    DynamicCast<QbbNetDevice>(b->GetDevice(nbr2if[b][a].idx))->TakeDown();
    // reset routing table
    SetRoutingEntries();

    // redistribute qp on each host
    for (uint32_t i = 0; i < n.GetN(); i++)
    {
        if (n.Get(i)->GetNodeType() == 0)
            n.Get(i)->GetObject<RdmaDriver>()->m_rdma->RedistributeQp();
    }
}

#define CONFIG_PREFIX "config_"
/**
 * \brief change output file name by config name.
 * For example: if config file is "mix/config_ABC.cfg" and output file is "output/trace.tr",
 * then the return value is "output/trace_ABC.tr"
 */
string get_output_file_name(string config_file, string output_file)
{
    if (config_file.empty() || output_file.empty())
    {
        std::cerr << "Error: Empty file path" << ", (config_file: " << config_file << ", output_file: " << output_file << ")" << std::endl;
        return "";
    }

    size_t config_last_slash = config_file.find_last_of('/');
    size_t config_last_dot = config_file.find_last_of('.');
    size_t output_last_dot = output_file.find_last_of('.');

    if (config_last_dot == string::npos || output_last_dot == string::npos)
    {
        std::cerr << "Error: Invalid file format" << std::endl;
        return "";
    }
    size_t config_id_start = (config_last_slash == string::npos) ? strlen(CONFIG_PREFIX) : config_last_slash + 1 + strlen(CONFIG_PREFIX);

    return output_file.substr(0, output_last_dot) + "_" +
           config_file.substr(config_id_start, config_last_dot - config_id_start) +
           output_file.substr(output_last_dot);
}

uint64_t get_nic_rate(NodeContainer &n)
{
    for (uint32_t i = 0; i < n.GetN(); i++)
        if (n.Get(i)->GetNodeType() == 0)
            return DynamicCast<QbbNetDevice>(n.Get(i)->GetDevice(1))
                ->GetDataRate()
                .GetBitRate();
    return 0; // Return 0 if no suitable node is found
}

bool ReadConf(int argc, char *argv[])
{

#ifndef PGO_TRAINING
    if (argc > 1)
#else
    if (true)
#endif
    {
        // Read the configuration file
        std::ifstream conf;
        string config_file = argv[1];
        InitConfigMap();
#ifndef PGO_TRAINING
        conf.open(config_file);
        if (!conf.is_open())
        {
            std::cerr << "Error: Failed to open config file, please check the path: " << config_file << std::endl;
            return false;
        }
#else
        conf.open(PATH_TO_PGO_CONFIG);
#endif
        string line;
        GroupConfig group_config;
        bool in_group = false;
        while (!conf.eof())
        {
            std::getline(conf, line);
            if (line == "" || line == "\n")
            {
                continue;
            }
            if (line[0] == '#')
            { // comment. Now only support comment at the beginning
                continue;
            }
            std::istringstream iss(line);
            std::string key;
            if (!(iss >> key))
            {
                continue;
            }

            if (key == "GROUP_START{")
            {
                in_group = true;
                continue;
            }
            else if (key == "}GROUP_END")
            {
                in_group = false;
                group_config.Parse();
                group_config.Print();
                group_config.Clear();
                continue;
            }
            std::string value;
            std::getline(iss, value);
            if (!value.empty() && value.front() == ' ')
            {
                value.erase(0, 1);
            }
            // 去除value结尾的\r
            if (value.back() == '\r')
            {
                value.pop_back();
            }

            if (in_group)
            {
                if (key == "TYPE")
                {
                    group_config.type = value;
                }
                else if (key == "NODES")
                {
                    group_config.nodes_str = value;
                }
                else
                {
                    group_config.configs.push_back(pair<string, string>(key, value));
                }
            }
            else if (config_map.find(key) != config_map.end())
            {
                config_map[key]->set_value(value);
            }
            else if (key.substr(0, 5) == "ns3::")
            {
                // 如果key以ns3::开头，则尝试直接设置
                Ns3ConfigMethods::ParseAndSetConfigDefault(key, value);
            }
            else
            {
                std::cout << "Error: key not found: " << key << std::endl;
            }
            fflush(stdout);
        }
        NS_LOG_INFO("read config done!");
        conf.close();
        /*******************
         * set special filename
         ******************/
        // if (argc > 2) {
        //   trace_output_file = trace_output_file + std::string(argv[2]);
        // }
        trace_output_file = get_output_file_name(config_file, trace_output_file);
        fct_output_file = get_output_file_name(config_file, fct_output_file);
        pfc_output_file = get_output_file_name(config_file, pfc_output_file);
        send_output_file = get_output_file_name(config_file, send_output_file);
        qlen_mon_file = get_output_file_name(config_file, qlen_mon_file);
        bw_mon_file = get_output_file_name(config_file, bw_mon_file);
        rate_mon_file = get_output_file_name(config_file, rate_mon_file);
        cnp_mon_file = get_output_file_name(config_file, cnp_mon_file);
        return true;
    }
    else
    {
        std::cout << "Error: require a config file\n";
        fflush(stdout);
        return false;
    }
}

void SetConfig()
{
    // set int_multi
    IntHop::multi = int_multi;
    uint32_t cc_mode = get_config_value_ns3<uint64_t>("ns3::RdmaHw::CcMode");
    // IntHeader::mode
    if (cc_mode == 7 || cc_mode == 11) // timely or swift, use ts
        IntHeader::mode = IntHeader::TS;
    else if (cc_mode == 3) // hpcc, use int
        IntHeader::mode = IntHeader::NORMAL;
    else if (cc_mode == 10) // hpcc-pint
        IntHeader::mode = IntHeader::PINT;
    else // others, no extra header
        IntHeader::mode = IntHeader::NONE;

    // Set Pint
    if (cc_mode == 10)
    {
        Pint::set_log_base(pint_log_base);
        IntHeader::pint_bytes = Pint::get_n_bytes();
        printf("PINT bits: %d bytes: %d\n", Pint::get_n_bits(),
               Pint::get_n_bytes());
    }
}

/**
 * \brief Setup RdmaClient and it's callbacks for a given Connection
 * \param conn The Connection to setup
 * \param tb The ThreadBlock to bind the RdmaClient
 */
void SetupConnection(Connection& conn)
{
    uint32_t dst = conn.dst;
    uint32_t src = conn.src;

    uint16_t node_sport;
    uint16_t remote_node_sport;
    RdmaClientHelper my_clientHelper;

    if (dst != UINT32_MAX){
        node_sport = portNumber[src][dst]++;
        uint32_t dport = 100;
        my_clientHelper = RdmaClientHelper(
            priority_group, serverAddress[src], serverAddress[dst], node_sport, dport,
            0, // create a qp w/o message
            has_win ? (global_t == 1 ? maxBdp : pairBdp[n.Get(src)][n.Get(dst)]) : 0,
            global_t == 1 ? maxRtt : pairRtt[src][dst],
            nullptr, nullptr,
            1, src, dst,
            false, false); // passive destroy, not operations run

        ApplicationContainer app = my_clientHelper.Install(n.Get(src));
        NS_LOG_INFO("Create a send connection from " << src << " to " << dst << ", src port: " << node_sport << ", dst port: " << dport << " @node " << src);
        app.Start(Seconds(simulator_start_time));

        conn.sender_client = DynamicCast<RdmaClient>(app.Get(0));
        conn.sender_client -> AddTxChannel((uint64_t)dst << 32 | (uint64_t)node_sport << 16 |  (uint64_t)priority_group);
    }else{
        NS_LOG_INFO("Not a send node, create a loopback connection");
        my_clientHelper = RdmaClientHelper(
            priority_group, serverAddress[src], serverAddress[src], 100, 100,
            0, 0, 0, nullptr, nullptr, 1, src, src, false, false);

        ApplicationContainer appThisChannel = my_clientHelper.Install(n.Get(src));
        appThisChannel.Start(Seconds(simulator_start_time));
        
        conn.sender_client = DynamicCast<RdmaClient>(appThisChannel.Get(0));
        node_sport = 1;
    }

    NS_LOG_INFO("Create a recv connection any way @node " << dst << ", dst "<< dst << ", sport: " << node_sport);
    my_clientHelper = RdmaClientHelper(
        priority_group, serverAddress[dst], serverAddress[dst], 100, 100,
        0, 0, 0, nullptr, nullptr, 1, src, src, false, false);

    ApplicationContainer app = my_clientHelper.Install(n.Get(dst));
    app.Start(Seconds(simulator_start_time));

    conn.receiver_client = DynamicCast<RdmaClient>(app.Get(0));
    conn.receiver_client -> AddRxChannel(std::make_pair(
        (uint64_t)dst << 32 | (uint64_t)node_sport << 16 |  (uint64_t)priority_group,
        src));

    return;
}


void SetupNetwork(
    void (*qp_finish)(FILE *, Ptr<RdmaQueuePair>),
    void (*send_finish)(FILE *, Ptr<RdmaQueuePair>),
    void (*message_finish)(FILE *, Ptr<RdmaQueuePair>, uint64_t))
{
    topof.open(topology_file.c_str());
    algof.open(algo_file.c_str());
    tracef.open(trace_file.c_str());
    if (!topof.is_open() || !algof.is_open() || !tracef.is_open())
    {
        NS_LOG_ERROR("Failed to open topology file, algo file, or trace file: "
                     << ", topo: " << topology_file << ", algo: " << algo_file << ", trace: " << trace_file);
        return;
    }

    /////////////////////////////////////////////////////
    // Topology configuration
    /////////////////////////////////////////////////////

    topof >> node_num >> gpus_per_server >> nvswitch_num >> switch_num >> link_num >> gpu_type >> ngpus_per_node;

    // Stage 1: Create nodes
    // 0: server, 1: switch, 2: nvswitch
    std::vector<uint32_t> node_type_list(node_num, 0);
    for (uint32_t i = 0; i < nvswitch_num; i++)
    {
        uint32_t sid;
        topof >> sid;
        node_type_list[sid] = 2;
    }
    for (uint32_t i = 0; i < switch_num; i++)
    {
        uint32_t sid;
        topof >> sid;
        node_type_list[sid] = 1;
    }
    for (uint32_t i = 0; i < node_num; i++)
    {
        switch (node_type_list[i])
        {
        case 3: ///////////!!!!!!!!!!!!!!!!
        {
            n.Add(CreateObject<Node>());
            break;
        }
        case 1:
        {
            Ptr<SwitchNode> sw = CreateObject<SwitchNode>();
            n.Add(sw);
            sw->SetAttribute("EcnEnabled", BooleanValue(get_config_value_ns3<bool>("ns3::QbbNetDevice::QcnEnabled")));
            break;
        }
        case 2:
        {
            Ptr<NVSwitchNode> sw = CreateObject<NVSwitchNode>();
            n.Add(sw);
            break;
        }
        case 0:
        {
            Ptr<GPUNode> m_gpu = CreateObject<GPUNode>();
            n.Add(m_gpu);
            break;
        }
        default:
            NS_ABORT_MSG("Unknown node type: " << node_type_list[i]);
        }
    }

    // maintain port number for each host pair
    for (uint32_t i = 0; i < node_num; i++)
    {
        if (n.Get(i)->GetNodeType() == 0 || n.Get(i)->GetNodeType() == 2)
            for (uint32_t j = 0; j < node_num; j++)
            {
                if (n.Get(j)->GetNodeType() == 0 || n.Get(j)->GetNodeType() == 2)
                    // each host pair use port number from 10000
                    portNumber[i][j] = 10000;
            }
    }

    NS_LOG_INFO("Nodes have been created, total number: " << n.GetN());

    // Stage 2: Create links

    // Install the internet stack on the nodes
    InternetStackHelper internet;
    internet.Install(n);

    // global error model
    Ptr<RateErrorModel> rem = CreateObject<RateErrorModel>();
    Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
    rem->SetRandomVariable(uv);
    uv->SetStream(50);
    rem->SetAttribute("ErrorRate", DoubleValue(error_rate_per_link));
    rem->SetAttribute("ErrorUnit", StringValue("ERROR_UNIT_PACKET"));

    FILE *pfc_file = fopen(pfc_output_file.c_str(), "w");

    QbbHelper qbb;
    Ipv4AddressHelper ipv4;

    // Set server IP addresses
    for (uint32_t i = 0; i < node_num; i++)
    {
        if (n.Get(i)->GetNodeType() == 0)
        {
            serverAddress.resize(i + 1);
            serverAddress[i] = node_id_to_ip(i);
        }
        else if (n.Get(i)->GetNodeType() == 2)
        {
            serverAddress.resize(i + 1);
            serverAddress[i] = node_id_to_ip(i);
        }
    }

    // create links
    for (uint32_t i = 0; i < link_num; i++)
    {
        uint32_t src, dst;
        std::string data_rate, link_delay;
        double error_rate;

        topof >> src >> dst >> data_rate >> link_delay >> error_rate;
        Ptr<Node> snode = n.Get(src), dnode = n.Get(dst);

        qbb.SetDeviceAttribute("DataRate", StringValue(data_rate));
        qbb.SetChannelAttribute("Delay", StringValue(link_delay));

        if (error_rate > 0)
        {
            Ptr<RateErrorModel> rem = CreateObject<RateErrorModel>();
            Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
            rem->SetRandomVariable(uv);
            uv->SetStream(50);
            rem->SetAttribute("ErrorRate", DoubleValue(error_rate));
            rem->SetAttribute("ErrorUnit", StringValue("ERROR_UNIT_PACKET"));
            qbb.SetDeviceAttribute("ReceiveErrorModel", PointerValue(rem));
        }
        else
        {
            // use the global error model
            qbb.SetDeviceAttribute("ReceiveErrorModel", PointerValue(rem));
        }

        // fflush(stdout);

        // Assigne server IP
        // Note: this should be before the automatic assignment below
        // (ipv4.Assign(d)), because we want our IP to be the primary IP (first in
        // the IP address list), so that the global routing is based on our IP
        NetDeviceContainer d = qbb.Install(snode, dnode);
        if (snode->GetNodeType() == 0 || snode->GetNodeType() == 2)
        {
            Ptr<Ipv4> ipv4 = snode->GetObject<Ipv4>();
            ipv4->AddInterface(d.Get(0));
            ipv4->AddAddress(
                1, Ipv4InterfaceAddress(serverAddress[src], Ipv4Mask(0xff000000)));
        }
        if (dnode->GetNodeType() == 0 || dnode->GetNodeType() == 2)
        {
            Ptr<Ipv4> ipv4 = dnode->GetObject<Ipv4>();
            ipv4->AddInterface(d.Get(1));
            ipv4->AddAddress(
                1, Ipv4InterfaceAddress(serverAddress[dst], Ipv4Mask(0xff000000)));
        }

        // used to create a graph of the topology
        nbr2if[snode][dnode].idx = DynamicCast<QbbNetDevice>(d.Get(0))->GetIfIndex();
        nbr2if[snode][dnode].up = true;
        nbr2if[snode][dnode].delay = DynamicCast<QbbChannel>(
                                         DynamicCast<QbbNetDevice>(d.Get(0))->GetChannel())
                                         ->GetDelay()
                                         .GetTimeStep();
        nbr2if[snode][dnode].bw = DynamicCast<QbbNetDevice>(d.Get(0))->GetDataRate().GetBitRate();

        nbr2if[dnode][snode].idx = DynamicCast<QbbNetDevice>(d.Get(1))->GetIfIndex();
        nbr2if[dnode][snode].up = true;
        nbr2if[dnode][snode].delay = DynamicCast<QbbChannel>(
                                         DynamicCast<QbbNetDevice>(d.Get(1))->GetChannel())
                                         ->GetDelay()
                                         .GetTimeStep();
        nbr2if[dnode][snode].bw = DynamicCast<QbbNetDevice>(d.Get(1))->GetDataRate().GetBitRate();

        // This is just to set up the connectivity between nodes. The IP addresses
        // are useless
        char ipstring[16];
        sprintf(ipstring, "10.%d.%d.0", i / 254 + 1, i % 254 + 1);
        ipv4.SetBase(ipstring, "255.255.255.0");
        ipv4.Assign(d);

        // setup PFC trace
        DynamicCast<QbbNetDevice>(d.Get(0))->TraceConnectWithoutContext(
            "QbbPfc", MakeBoundCallback(&get_pfc, pfc_file,
                                        DynamicCast<QbbNetDevice>(d.Get(0))));
        DynamicCast<QbbNetDevice>(d.Get(1))->TraceConnectWithoutContext(
            "QbbPfc", MakeBoundCallback(&get_pfc, pfc_file,
                                        DynamicCast<QbbNetDevice>(d.Get(1))));
    }

    NS_LOG_INFO("Links have been created, Total " << link_num << " links.");

    // Stage 3: Configure switches and nvswitches
    nic_rate = get_nic_rate(n);

    uint32_t payload = get_config_value_ns3<uint64_t>("ns3::RdmaHw::Mtu");
    // config switch
    for (uint32_t i = 0; i < node_num; i++)
    {
        if (n.Get(i)->GetNodeType() == 1)
        { // is switch
            Ptr<SwitchNode> sw = DynamicCast<SwitchNode>(n.Get(i));
            uint32_t shift = 3; // by default 1/8

            // std::cout << "sw Ndevices is: " << sw->GetNDevices() << std::endl;
            for (uint32_t j = 1; j < sw->GetNDevices(); j++)
            {
                Ptr<QbbNetDevice> dev = DynamicCast<QbbNetDevice>(sw->GetDevice(j));
                // set ecn
                uint64_t rate = dev->GetDataRate().GetBitRate();
                NS_ASSERT_MSG(rate2kmin.find(rate) != rate2kmin.end(),
                              "must set kmin for each link speed");
                NS_ASSERT_MSG(rate2kmax.find(rate) != rate2kmax.end(),
                              "must set kmax for each link speed");
                NS_ASSERT_MSG(rate2pmax.find(rate) != rate2pmax.end(),
                              "must set pmax for each link speed");
                sw->m_mmu->ConfigEcn(j, rate2kmin[rate], rate2kmax[rate],
                                     rate2pmax[rate]);
                // set pfc
                uint64_t delay = DynamicCast<QbbChannel>(dev->GetChannel())
                                     ->GetDelay()
                                     .GetTimeStep();
                // uint32_t headroom = 150000; // rate * delay / 8 / 1000000000 * 3;
                uint32_t headroom = rate * delay / 8 / 1000000000 * 2 + payload * 2; // BDP + 2 packet
                sw->m_mmu->ConfigHdrm(j, headroom);

                // set pfc alpha, proportional to link bw
                sw->m_mmu->pfc_a_shift[j] = shift;
                while (rate > nic_rate && sw->m_mmu->pfc_a_shift[j] > 0)
                {
                    sw->m_mmu->pfc_a_shift[j]--;
                    rate /= 2;
                }
            }
            sw->m_mmu->ConfigNPort(sw->GetNDevices() - 1);
            sw->m_mmu->ConfigBufferSize(buffer_size * 1024 * 1024);
            sw->m_mmu->node_id = sw->GetId();
        }
        else if (n.Get(i)->GetNodeType() == 2)
        { // is nvswitch
            Ptr<NVSwitchNode> sw = DynamicCast<NVSwitchNode>(n.Get(i));
            uint32_t shift = 3; // by default 1/8
            for (uint32_t j = 1; j < sw->GetNDevices(); j++)
            {
                Ptr<QbbNetDevice> dev = DynamicCast<QbbNetDevice>(sw->GetDevice(j));
                // set ecn
                uint64_t rate = dev->GetDataRate().GetBitRate();
                // set pfc
                uint64_t delay = DynamicCast<QbbChannel>(dev->GetChannel())
                                     ->GetDelay()
                                     .GetTimeStep();
                // uint32_t headroom = 150000; // rate * delay / 8 / 1000000000 * 3;
                uint32_t headroom = rate * delay / 8 / 1000000000 * 3;
                sw->m_mmu->ConfigHdrm(j, headroom);

                // set pfc alpha, proportional to link bw
                sw->m_mmu->pfc_a_shift[j] = shift;
                while (rate > nic_rate && sw->m_mmu->pfc_a_shift[j] > 0)
                {
                    sw->m_mmu->pfc_a_shift[j]--;
                    rate /= 2;
                }
            }
            sw->m_mmu->ConfigNPort(sw->GetNDevices() - 1);
            sw->m_mmu->ConfigBufferSize(buffer_size * 1024 * 1024);
            sw->m_mmu->node_id = sw->GetId();
        }
    }
    NS_LOG_INFO("Switches and NVSwtiches have been configured, total number of switches: " << switch_num
                                                                                           << ", nvswitches: " << nvswitch_num);

#if ENABLE_QP

    // Stage 4: Configure RDMA and install RDMA driver

    FILE *fct_output = fopen(fct_output_file.c_str(), "w");
    FILE *send_output = send_output_file.empty() ? nullptr : fopen(send_output_file.c_str(), "w");
    NS_LOG_INFO("QP is enabled");

    for (uint32_t i = 0; i < node_num; i++)
    {
        // is server or nvls nvswitch
        if (n.Get(i)->GetNodeType() == 0 || n.Get(i)->GetNodeType() == 2)
        {
            // create RdmaHw
            Ptr<RdmaHw> rdmaHw = CreateObject<RdmaHw>();
            // check if i is in rdmaHw_config_map, if so, set the group attribute
            if (rdmaHw_config_map.find(i) != rdmaHw_config_map.end())
            {
                for (auto configEntry : *(rdmaHw_config_map[i]))
                {
                    std::cout << "node " << i << " set attribute " << configEntry.first << std::endl;
                    if (configEntry.first.compare(0, 5, "CC::") == 0)
                    {
                        rdmaHw->m_cc_configs.push_back(std::make_pair(configEntry.first.substr(5), configEntry.second));
                    }
                    rdmaHw->SetAttribute(configEntry.first, *(configEntry.second));
                }
            }
            // create and install RdmaDriver
            Ptr<RdmaDriver> rdma = CreateObject<RdmaDriver>();
            Ptr<Node> node = n.Get(i);
            rdma->SetNode(node);
            rdma->SetRdmaHw(rdmaHw);

            node->AggregateObject(rdma);
            rdma->Init();
            rdma->TraceConnectWithoutContext("QpComplete", MakeBoundCallback(qp_finish, fct_output));
            rdma->TraceConnectWithoutContext("MessageComplete", MakeBoundCallback(message_finish, fct_output));
            rdma->TraceConnectWithoutContext("SendComplete", MakeBoundCallback(send_finish, send_output));
            // ramd-driver->rdma-hw
        }
    }

    NS_LOG_INFO("RdmaDriver and RdmaHw have been installed.");
#endif

    // Stage 5: Misc configuration
    // priority, routing, BDP, CC mode

    // set ACK priority on hosts
    RdmaEgressQueue::ack_q_idx = ack_high_prio ? 0 : 3;

    // setup routing
    CalculateRoutes(n);
    SetRoutingEntries();

    // print routing entries
    // printRoutingEntries();

    // get BDP and delay
    maxRtt = maxBdp = 0;
    for (uint32_t i = 0; i < node_num; i++)
    {
        if (n.Get(i)->GetNodeType() != 0)
            continue;
        for (uint32_t j = 0; j < node_num; j++)
        {
            if (n.Get(j)->GetNodeType() != 0)
                continue;
            uint64_t delay = pairDelay[n.Get(i)][n.Get(j)];
            uint64_t txDelay = pairTxDelay[n.Get(i)][n.Get(j)];
            uint64_t rtt = delay * 2 + txDelay;
            uint64_t bw = pairBw[i][j];
            uint64_t bdp = rtt * bw / 1000000000 / 8;
            pairBdp[n.Get(i)][n.Get(j)] = bdp;
            pairRtt[i][j] = rtt;
            if (bdp > maxBdp)
                maxBdp = bdp;
            if (rtt > maxRtt)
                maxRtt = rtt;
        }
    }
    NS_LOG_INFO("Get maxRtt and maxBdp, maxRtt: " << maxRtt << " ns, maxBdp: " << maxBdp << " Bytes");

    // setup switch CC
    for (uint32_t i = 0; i < node_num; i++)
    {
        if (n.Get(i)->GetNodeType() == 1)
        { // switch
            Ptr<SwitchNode> sw = DynamicCast<SwitchNode>(n.Get(i));
            sw->SetAttribute("CcMode", UintegerValue(get_config_value_ns3<uint64_t>("ns3::RdmaHw::CcMode")));
            sw->SetAttribute("MaxRtt", UintegerValue(maxRtt));
        }
    }

    /////////////////////////////////////////////////////
    // Algorithm configuration
    /////////////////////////////////////////////////////

    bool comm_domain_read = ReadCommDomainFile(comm_domain_file);
    NS_ASSERT_MSG(comm_domain_read, "Failed to read communication domain file: " << comm_domain_file);

    bool mpi_flow_read = ReadMPIFlowFile(mpi_flow_file);
    NS_ASSERT_MSG(mpi_flow_read, "Failed to read MPI flow file: " << mpi_flow_file);

    bool mpi_setup_success = SetupMPIFlows();
    NS_ASSERT_MSG(mpi_setup_success, "Failed to setup MPI flows from file: " << mpi_flow_file);

    /////////////////////////////////////////////////////
    // Trace configuration
    /////////////////////////////////////////////////////

    tracef >> trace_num;
    NodeContainer trace_nodes;
    for (uint32_t i = 0; i < trace_num; i++)
    {
        uint32_t nid;
        tracef >> nid;
        if (nid >= n.GetN())
        {
            continue;
        }
        trace_nodes = NodeContainer(trace_nodes, n.Get(nid));
    }

    FILE *trace_output = nullptr;
    if (enable_trace)
    {
        trace_output = fopen(trace_output_file.c_str(), "w");
        qbb.EnableTracing(trace_output, trace_nodes);
        // dump link speed to trace file
        SimSetting sim_setting;
        for (auto i : nbr2if)
        {
            for (auto j : i.second)
            {
                uint16_t node = i.first->GetId();
                uint8_t intf = j.second.idx;
                uint64_t bps =
                    DynamicCast<QbbNetDevice>(i.first->GetDevice(j.second.idx))
                        ->GetDataRate()
                        .GetBitRate();
                sim_setting.port_speed[node][intf] = bps;
            }
        }
        sim_setting.win = maxBdp;
        sim_setting.Serialize(trace_output);
    }

    NS_LOG_INFO("Trace has been configured, total number of traced nodes: " << trace_nodes.GetN());

    Time interPacketInterval = Seconds(0.0000005 / 2); // 0.5us / 2

    topof.close();
    algof.close();
    tracef.close();

    // schedule link down
    if (link_down[0] > 0)
    {
        Simulator::Schedule(
            Seconds(simulator_start_time) + MicroSeconds(link_down[0]),
            &TakeDownLink,
            n,
            n.Get(link_down[1]),
            n.Get(link_down[2]));
    }

    // schedule buffer monitor
    // FILE *qlen_output = fopen(qlen_mon_file.c_str(), "w");
    // Simulator::Schedule(NanoSeconds(qlen_mon_start), &monitor_buffer, qlen_output,
    //                     &n);
    // schedule monitor

    // schedule_monitor(); // TODO: End time is 210000, its hard coded

    NS_LOG_INFO("Network setup done.");
}
#endif
