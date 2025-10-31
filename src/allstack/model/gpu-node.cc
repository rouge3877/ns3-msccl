#include "gpu-node.h"


#include "ns3/integer.h"
#include "ns3/simulator.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("GPUNode");

NS_OBJECT_ENSURE_REGISTERED(GPUNode);

TypeId
GPUNode::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::GPUNode")
            .SetParent<Node>()
            .SetGroupName("AllStack")
            .AddConstructor<GPUNode>()
            .AddAttribute(
                "rank",
                "The rank of this GPU node",
                IntegerValue(-1),
                MakeIntegerAccessor(&GPUNode::m_rank),
                MakeIntegerChecker<int32_t>());
    return tid;
}

GPUNode::GPUNode()
    : Node(), m_rank(-1)
{
    NS_LOG_FUNCTION(this);
}

Ptr<ThreadBlock>
GPUNode::GetThreadBlock(uint32_t index) const
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(index < m_threadblocks.size(), "Get ThreadBlock Index out of range");
    return m_threadblocks[index];
}

uint32_t
GPUNode::GetNThreadBlocks() const
{
    NS_LOG_FUNCTION(this);
    return m_threadblocks.size();
}

int
GPUNode::GetRank() const
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(m_rank >= 0, "GPUNode rank not set");
    return m_rank;
}

void
GPUNode::SetRank(int rank)
{
    NS_LOG_FUNCTION(this);
    m_rank = rank;
}

uint32_t
GPUNode::AddThreadBlock(Ptr<ThreadBlock> tb)
{
    NS_LOG_FUNCTION(this << tb);
    uint32_t index = Node::AddApplication(tb);
    m_threadblocks.push_back(tb);
    m_tb_status.push_back(-1);
    m_tb_complete_count += 1;
    tb->SetGPUNode(this);
    return m_threadblocks.size() - 1;
}

int
GPUNode::FinishedTBCallback()
{
    NS_LOG_FUNCTION(this);
    m_tb_complete_count -= 1;
    if (m_tb_complete_count == 0)
    {
        for (auto i = m_threadblocks.begin(); i != m_threadblocks.end(); i++)
        {
            Ptr<ThreadBlock> tb = *i;
            // Ptr<RdmaClient> rdmaClient = tb->GetRdmaClient();
            // rdmaClient->FinishQp();

            if (m_end_time < tb->m_end_time)
            {
                m_end_time = tb->m_end_time;
            }
        }
        NS_LOG_INFO("GPUNode " << GetId() << " finished MPI #" << m_mpi_step_idx << " 's all ThreadBlocks");
        this->CompleteMPIStep();
        return 1;
    }
    return 0;
}

void
GPUNode::UpdateTBStatus(uint32_t index, uint32_t step)
{
    NS_LOG_FUNCTION(this);
    m_tb_status[index] = step;

    NS_LOG_INFO("GPU " << GetId()
                 << " ThreadBlock " << index
                 << " Step " << step
                 << " Notified!");

    for (auto i = m_threadblocks.begin(); i != m_threadblocks.end(); i++)
    {
        Ptr<ThreadBlock> tb = *i;
        tb->UpdateTBStatus(index, step);
    }
}

int
GPUNode::GetTBStatus(uint32_t index)
{
    NS_LOG_FUNCTION(this);
    return m_tb_status[index];
}

void
GPUNode::DoDispose()
{
    m_tb_status.clear();
    m_threadblocks.clear();
    Node::DoDispose();
}

std::vector<MPIStep> *
GPUNode::GetMPISteps() const
{
    NS_LOG_FUNCTION(this);
    std::vector<MPIStep>* steps = const_cast<std::vector<MPIStep>*>(&m_mpi_steps);
    return steps;
}

void
GPUNode::AddMPIStep(MPIStep &step, double interval)
{
    NS_LOG_FUNCTION(this);
    MPIStep mpi_step;
    mpi_step.name = step.name;
    mpi_step.total_size_bytes = step.total_size_bytes;
    mpi_step.ranks = step.ranks;
    mpi_step.interval = interval;

    // for all gpu elements in algo, use id as ranks index to get the real real rank
    auto gpu = step.algo->FirstChildElement("gpu");
    while (gpu != nullptr)
    {
        int rank_index = gpu->IntAttribute("id");
        int real_rank = step.ranks[rank_index];
        
        if (real_rank == m_rank)
        {
            mpi_step.algo = gpu;
            break;
        }
        gpu = gpu->NextSiblingElement("gpu");
    }
    NS_ASSERT_MSG(mpi_step.algo != nullptr, "No matching GPU element found for rank " << m_rank << " in MPI Step " << mpi_step.name);

    m_mpi_steps.push_back(mpi_step);
}

void
GPUNode::DoMPIStep()
{
    NS_LOG_FUNCTION(this);

    if (m_mpi_step_idx >= m_mpi_steps.size())
    {
        NS_LOG_INFO("All MPI Steps completed for GPUNode " << GetId());
        return;
    }

    MPIStep &step = m_mpi_steps[m_mpi_step_idx];
    NS_LOG_DEBUG("Starting MPI Step #" << m_mpi_step_idx << " " << step.name << " for GPUNode " << GetId() << ", total size (bytes): " << step.total_size_bytes);

    tinyxml2::XMLElement* gpu_xml = step.algo;
    // check gpu_xml is <gpu ...> element but not <algo ...>
    NS_ASSERT_MSG(std::string(gpu_xml->Name()) == "gpu", "Expected <gpu> element, got <" << gpu_xml->Name() << ">");
    ApplicationContainer apps;

    // TODO: use nchunksperloop but not i_chunks
    uint32_t chunks_number = static_cast<uint32_t>(std::stoul(gpu_xml->Attribute("i_chunks")));
    for (auto tb_xml = gpu_xml->FirstChildElement("tb"); tb_xml != nullptr; tb_xml = tb_xml->NextSiblingElement("tb"))
    {
        ObjectFactory gpu_factory;
        gpu_factory.SetTypeId(ThreadBlock::GetTypeId());
        uint32_t id = static_cast<uint32_t>(std::stoul(tb_xml->Attribute("id")));
        int32_t origin_send = static_cast<int32_t>(std::stoi(tb_xml->Attribute("send")));
        int32_t origin_recv = static_cast<int32_t>(std::stoi(tb_xml->Attribute("recv")));

        int32_t mapped_send = (origin_send == -1) ? -1 : step.ranks[origin_send];
        int32_t mapped_recv = (origin_recv == -1) ? -1 : step.ranks[origin_recv];

        gpu_factory.Set("id", UintegerValue(id));
        gpu_factory.Set("send", IntegerValue(mapped_send));
        gpu_factory.Set("recv", IntegerValue(mapped_recv));
        gpu_factory.Set("channel", IntegerValue(static_cast<int32_t>(std::stoi(tb_xml->Attribute("chan")))));

        // TODO: how to calc chunk size when chunks number is too large
        uint32_t chunk_size = (step.total_size_bytes + chunks_number - 1) / chunks_number;
        NS_LOG_DEBUG("ThreadBlock " << id << " chunk size (bytes): " << chunk_size << ", its total size (bytes): " << step.total_size_bytes);
        gpu_factory.Set("chunkSize", UintegerValue(chunk_size));

        Ptr<ThreadBlock> tb = gpu_factory.Create<ThreadBlock>();
        for (auto step_xml = tb_xml->FirstChildElement("step"); step_xml != nullptr; step_xml = step_xml->NextSiblingElement("step"))
        {
            ObjectFactory step_factory;
            step_factory.SetTypeId(ThreadBlockStep::GetTypeId());
            step_factory.Set("s", UintegerValue(static_cast<uint32_t>(std::stoul(step_xml->Attribute("s")))));
            step_factory.Set("type", StringValue(static_cast<std::string>(step_xml->Attribute("type"))));
            step_factory.Set("cnt", UintegerValue(static_cast<uint32_t>(std::stoul(step_xml->Attribute("cnt")))));

            // TODO: whats the depid meanings? Node or ThreadBlock?
            int depid = static_cast<int>(std::stoi(step_xml->Attribute("depid")));
            int deps = static_cast<int>(std::stoi(step_xml->Attribute("deps")));
            if (depid >-1 && deps > -1)
            {
                step_factory.Set("dep", BooleanValue(true));
                step_factory.Set("depid", UintegerValue(depid));
                step_factory.Set("deps", UintegerValue(deps));
            }
            else
            {
                step_factory.Set("dep", BooleanValue(false));
            }
            step_factory.Set("hasdep", BooleanValue(static_cast<bool>(std::stoi(step_xml->Attribute("hasdep")))));

            Ptr<ThreadBlockStep> tb_step = step_factory.Create<ThreadBlockStep>();
            uint32_t index = tb->AddStep(tb_step);
            NS_ASSERT(index==tb_step->GetS());
        }
        uint32_t tb_index = this->AddThreadBlock(tb);
        NS_ASSERT_MSG(tb_index == tb->GetId(), "ThreadBlock index mismatch, expected " << tb_index << ", got " << tb->GetId());
        apps.Add(tb);

        Ptr<RdmaClient> to_bind_send_client = nullptr;
        Ptr<RdmaClient> to_bind_recv_client = nullptr;
        if (mapped_send != -1)
        {
            ConnectionInfo send_conn(this->GetRank(), mapped_send, tb->GetChannel());
            auto it = m_connections.find(send_conn);
            NS_ASSERT_MSG(it != m_connections.end(), "No RdmaClient found for send connection: "
                                                    << "src_rank=" << send_conn.src_rank
                                                    << ", dst_rank=" << send_conn.dst_rank
                                                    << ", channel=" << send_conn.channel);
            to_bind_send_client = it->second;
        }
        if (mapped_recv != -1)
        {
            ConnectionInfo recv_conn(mapped_recv, this->GetRank(), tb->GetChannel());
            auto it = m_connections.find(recv_conn);
            NS_ASSERT_MSG(it != m_connections.end(), "No RdmaClient found for recv connection: "
                                                    << "src_rank=" << recv_conn.src_rank
                                                    << ", dst_rank=" << recv_conn.dst_rank
                                                    << ", channel=" << recv_conn.channel);
            to_bind_recv_client = it->second;
        }
        tb->BindRdmaClients(to_bind_send_client, to_bind_recv_client);
    }

    NS_LOG_DEBUG("Starting all ThreadBlocks for MPI Step #" << m_mpi_step_idx << " " << step.name << " for GPUNode " << GetId());
    apps.Start(Seconds(0));  // TODO
}

void
GPUNode::StartMPIStep()
{
    NS_LOG_FUNCTION(this);
    DoMPIStep();
}

void
GPUNode::CompleteMPIStep()
{
    NS_LOG_FUNCTION(this);

    m_mpi_step_idx += 1;
    // clear all thread blocks
    ApplicationContainer tb_apps;
    for (auto tb : m_threadblocks)
        tb_apps.Add(tb);
    tb_apps.Stop(Seconds(0)); // stop immediately 
    m_threadblocks.clear();
    m_tb_status.clear();
    m_tb_complete_count = 0;

    if (m_mpi_step_idx < m_mpi_steps.size())
    {
        DoMPIStep();
    }
    else
    {
        NS_LOG_INFO("All MPI Steps completed for GPUNode " << GetId());
        // Ptr<RdmaClient> rdmaClient = tb->GetRdmaClient();
        // rdmaClient->FinishQp();
    }
}

void
GPUNode::AddConnection(ConnectionInfo conn, Ptr<RdmaClient> client)
{
    NS_LOG_FUNCTION(this);
    if (m_connections.find(conn) != m_connections.end())
    {
        NS_LOG_WARN("Connection already exists in GPUNode " << GetId() << ": "
                                                          << "src_rank=" << conn.src_rank
                                                          << ", dst_rank=" << conn.dst_rank
                                                          << ", channel=" << conn.channel);
    }
    m_connections[conn] = client;
}

}