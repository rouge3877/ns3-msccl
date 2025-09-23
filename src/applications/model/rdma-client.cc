/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
 *
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
 *
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
 */
#include "rdma-client.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-end-point.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/qbb-net-device.h"
#include "ns3/random-variable.h"
#include "ns3/seq-ts-header.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"
#include <ns3/rdma-driver.h>
#include <stdio.h>
#include <stdlib.h>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RdmaClient");
NS_OBJECT_ENSURE_REGISTERED(RdmaClient);

TypeId RdmaClient::GetTypeId(void) {
  static TypeId tid =
      TypeId("ns3::RdmaClient")
          .SetParent<Application>()
          .AddConstructor<RdmaClient>()
          .AddAttribute("WriteSize", "The number of bytes to write",
                        UintegerValue(10000),
                        MakeUintegerAccessor(&RdmaClient::m_size),
                        MakeUintegerChecker<uint64_t>())
          .AddAttribute("SourceIP", "Source IP", Ipv4AddressValue("0.0.0.0"),
                        MakeIpv4AddressAccessor(&RdmaClient::m_sip),
                        MakeIpv4AddressChecker())
          .AddAttribute("DestIP", "Dest IP", Ipv4AddressValue("0.0.0.0"),
                        MakeIpv4AddressAccessor(&RdmaClient::m_dip),
                        MakeIpv4AddressChecker())
          .AddAttribute("SourcePort", "Source Port", UintegerValue(0),
                        MakeUintegerAccessor(&RdmaClient::m_sport),
                        MakeUintegerChecker<uint16_t>())
          .AddAttribute("DestPort", "Dest Port", UintegerValue(0),
                        MakeUintegerAccessor(&RdmaClient::m_dport),
                        MakeUintegerChecker<uint16_t>())
          .AddAttribute("PriorityGroup", "The priority group of this flow",
                        UintegerValue(0),
                        MakeUintegerAccessor(&RdmaClient::m_pg),
                        MakeUintegerChecker<uint16_t>())
          .AddAttribute("Window", "Bound of on-the-fly packets",
                        UintegerValue(0),
                        MakeUintegerAccessor(&RdmaClient::m_win),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("BaseRtt", "Base Rtt", UintegerValue(0),
                        MakeUintegerAccessor(&RdmaClient::m_baseRtt),
                        MakeUintegerChecker<uint64_t>())
          .AddAttribute("Tag", "Tag", UintegerValue(0),
                        MakeUintegerAccessor(&RdmaClient::tag),
                        MakeUintegerChecker<uint64_t>())
          .AddAttribute("Src", "Src", UintegerValue(0),
                        MakeUintegerAccessor(&RdmaClient::src),
                        MakeUintegerChecker<uint64_t>())
          .AddAttribute("Dest", "Dest", UintegerValue(0),
                        MakeUintegerAccessor(&RdmaClient::dest),
                        MakeUintegerChecker<uint64_t>())
          .AddAttribute("NVLS_enable", "NVLS enable info", UintegerValue(0),
                        MakeUintegerAccessor(&RdmaClient::nvls_enable),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("PassiveDestroy",
                        "Qp passively destroy when no more message",
                        BooleanValue(false),
                        MakeBooleanAccessor(&RdmaClient::SetPassiveDestroy,
                                          &RdmaClient::GetPassiveDestroy),
                        MakeBooleanChecker())
          .AddAttribute("OperationsRun", "If run in operations mode (Must with false PassiveDestroy attr)",
                        BooleanValue(false),
                        MakeBooleanAccessor(&RdmaClient::SetOperationsRun,
                                          &RdmaClient::GetOperationsRun),
                        MakeBooleanChecker());
  return tid;
}

RdmaClient::RdmaClient() { NS_LOG_FUNCTION_NOARGS(); }

RdmaClient::~RdmaClient() { NS_LOG_FUNCTION_NOARGS(); }

void RdmaClient::SetRemote(Ipv4Address ip, uint16_t port) {
  m_dip = ip;
  m_dport = port;
}

void RdmaClient::SetLocal(Ipv4Address ip, uint16_t port) {
  m_sip = ip;
  m_sport = port;
}

void RdmaClient::SetPG(uint16_t pg) { m_pg = pg; }

void RdmaClient::SetSize(uint64_t size) { m_size = size; }

void RdmaClient::Sent() {
}

void RdmaClient::PushMessagetoQp(uint64_t size) {
  m_qp->PushMessage(
      size,
      MakeCallback(&RdmaClient::Finish, this),
      MakeCallback(&RdmaClient::Sent, this));

  Ptr<RdmaDriver> rdma = GetNode()->GetObject<RdmaDriver>();
  Ptr<ns3::QbbNetDevice> nic = rdma->m_rdma->GetNicOfQp(m_qp);
  nic->TriggerTransmit();
}

void RdmaClient::FinishQp() {
  Ptr<Node> node = GetNode();
  Ptr<RdmaDriver> rdma = node->GetObject<RdmaDriver>();
  rdma->FinishQueuePair(m_qp);
}

void RdmaClient::Finish() {
  if(m_passiveDestroy && m_qp->m_messages.size() == 0){
    FinishQp();
  }
}

void RdmaClient::SetFn(void (*msg_handler)(void *fun_arg), void *fun_arg) {
  msg_handler = msg_handler;
  fun_arg = fun_arg;
}

void RdmaClient::DoDispose(void) {
  NS_LOG_FUNCTION_NOARGS();
  Application::DoDispose();
}

void RdmaClient::StartApplication(void) {
  NS_LOG_FUNCTION_NOARGS();
  
  // Validate attribute constraints
  if (m_passiveDestroy && m_operationsRun) {
    NS_FATAL_ERROR("Invalid configuration: PassiveDestroy and OperationsRun cannot both be true");
  }
  
  // get RDMA driver and add up queue pair
  Ptr<Node> node = GetNode();
  Ptr<RdmaDriver> rdma = node->GetObject<RdmaDriver>();
  // setup NVLS
  if(nvls_enable) rdma->EnbaleNVLS();
  else rdma->DisableNVLS();
  m_qp = rdma->AddQueuePair(src, dest, tag, m_size, m_pg, m_sip, m_dip, m_sport,
                     m_dport, m_win, m_baseRtt,
                     MakeCallback(&RdmaClient::Finish, this),
                     MakeCallback(&RdmaClient::Sent, this));

  if (m_operationsRun && !m_passiveDestroy) {
    NS_LOG_INFO("RdmaClient starting in OperationsRun mode.");
    std::cout << "RdmaClient starting in OperationsRun mode." << std::endl;
    m_currentOperationIndex = 0;
    RunNextStep();
  }
}

void RdmaClient::StopApplication() {
  NS_LOG_FUNCTION_NOARGS();
  // TODO stop the queue pair
}

// my additional functions for recv-complete callback

void RdmaClient::AddRxChannel(std::pair<uint64_t, uint32_t> sender_qp_key)
{  
  NS_LOG_FUNCTION_NOARGS();
  // get RDMA driver and add up queue pair
  Ptr<Node> node = GetNode();
  Ptr<RdmaDriver> rdma = node->GetObject<RdmaDriver>();
  rdma->m_rxCompleteCallbacks[sender_qp_key.first][sender_qp_key.second] = MakeCallback(&RdmaClient::HandleRxComplete, this);
}

void RdmaClient::AddTxChannel(uint64_t sender_qp_key){
  
  NS_LOG_FUNCTION_NOARGS();
  // get RDMA driver and add up queue pair
  Ptr<Node> node = GetNode();
  Ptr<RdmaDriver> rdma = node->GetObject<RdmaDriver>();
  rdma->m_txCompleteCallbacks[sender_qp_key] = MakeCallback(&RdmaClient::HandleTxComplete, this);
}

void RdmaClient::AddOperation(const std::string& opType, uint64_t size) {
  if (opType == "SEND") {
    m_operations.emplace_back(RdmaOperation::SEND, size);
    NS_LOG_INFO("Added SEND operation, size = " << size << " .");
  } else if (opType == "RECV") {
    m_operations.emplace_back(RdmaOperation::RECV, 0); // RECV 操作的大小无意义
    NS_LOG_INFO("Added RECV operation.");
  } else {
    NS_LOG_WARN("Unknown operation type specified: " << opType);
  }
}

void RdmaClient::DoSend(uint64_t size) {
    NS_LOG_INFO("Executing SEND operation #" << m_currentOperationIndex << " with size " << size);
    this->PushMessagetoQp(size);
}

void RdmaClient::DoRecv() {
    NS_LOG_INFO("Executing RECV operation #" << m_currentOperationIndex << ". Waiting for message...");
}

void RdmaClient::RunNextStep() {
    // 检查是否所有操作都已完成
    if (m_currentOperationIndex >= m_operations.size()) {
        NS_LOG_INFO("All configured operations have been completed.");
        FinishQp(); // 结束 QP
        return;
    }

    // 获取当前操作并执行
    const RdmaOperation& currentOp = m_operations[m_currentOperationIndex];
    if (currentOp.type == RdmaOperation::SEND) {
        DoSend(currentOp.size);
    } else if (currentOp.type == RdmaOperation::RECV) {
        DoRecv();
    }
}

void RdmaClient::HandleTxComplete() {
    if (m_currentOperationIndex < m_operations.size() &&
        m_operations[m_currentOperationIndex].type == RdmaOperation::SEND) {
        
        NS_LOG_INFO("SEND operation #" << m_currentOperationIndex << " completed.");
        m_currentOperationIndex++;
        RunNextStep();
    } else {
        NS_LOG_WARN("Received an unexpected SendComplete signal.");
    }
}

void RdmaClient::HandleRxComplete() {

    // 检查我们是否真的在等待一个接收完成事件
    if (m_currentOperationIndex < m_operations.size() &&
        m_operations[m_currentOperationIndex].type == RdmaOperation::RECV) {

        NS_LOG_INFO("RECV operation #" << m_currentOperationIndex << " completed.");
        m_currentOperationIndex++; // 指向下一个操作
        RunNextStep();             // 执行下一个操作
    } else {
        // NS_LOG_WARN(m_currentOperationIndex);
        // NS_LOG_WARN(m_operations.size());
        // NS_LOG_WARN(m_operations[m_currentOperationIndex].type);
        // NS_LOG_WARN(RdmaOperation::RECV);
        NS_LOG_WARN("Received an unexpected RecvComplete signal.");
    }
}

// Validation methods for attribute constraints
void RdmaClient::SetPassiveDestroy(bool value) {
    if (value && m_operationsRun)
        NS_FATAL_ERROR("Cannot set PassiveDestroy to true when OperationsRun is true");
    m_passiveDestroy = value;
}

bool RdmaClient::GetPassiveDestroy() const {
    return m_passiveDestroy;
}

void RdmaClient::SetOperationsRun(bool value) {
    if (value && m_passiveDestroy)
        NS_FATAL_ERROR("Cannot set OperationsRun to true when PassiveDestroy is true");
    m_operationsRun = value;
}

bool RdmaClient::GetOperationsRun() const {
    return m_operationsRun;
}

} // Namespace ns3
