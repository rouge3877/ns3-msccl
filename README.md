# NS3 - MSCCL

Based on [ns-3-alibabacloud](https://github.com/aliyun/ns-3-alibabacloud)

## Run

```bash
make conf
make log-rdma msccl-perf
make log-scheduler qpreuse-debug
# ...
# see Makefile for more
```

<details>

<summary>or Run:</summary>


```bash
./ns3 configure -d default --enable-examples --disable-mtp

# RdmaClient run in Operations Mode: Add a set of options to configure the simulation
NS_LOG="RdmaClient=all|prefix_all:RdmaDriver=info|prefix_all:RdmaHw=info|prefix_all:RdmaQueuePair=info|prefix_all" \
./ns3 run 'scratch/OpSendRecv examples/my-rdma-test/config_1to1.sh'

# Allstack
NS_LOG="MSCCL=all|prefix_all:ThreadBlock=all|prefix_all:RdmaClient=info|prefix_all:RdmaDriver=info|prefix_all" \
./ns3 run 'scratch/msccl/main examples/allstack/config.sh'
```

Debug:
```bash
# ns3-msccl
./ns3 configure -d debug --enable-examples --disable-mtp
NS_LOG="" ./ns3 run 'scratch/msccl/main' --command-template='gdb --args %s examples/allstack/config.sh'

# QpReuseNetwork
./ns3 configure -d debug --enable-examples --disable-mtp
NS_LOG="" ./ns3 run 'scratch/QpReuseNetwork' --command-template='gdb --args %s examples/my-rdma-test/config_1to1.sh'
```

Perf:

```bash
# ns3-msccl
./ns3 configure -d debug --enable-examples --disable-mtp
./ns3 run 'scratch/msccl/main' --command-template='sudo perf record -F 99 -g %s examples/allstack/config.sh'
sudo perf script | ~/FlameGraph/stackcollapse-perf.pl | ~/FlameGraph/flamegraph.pl > ns3-msccl.svg

# QpReuseNetwork
./ns3 configure -d debug --enable-examples --disable-mtp
./ns3 run 'scratch/QpReuseNetwork' --command-template='sudo perf record -F 99 -g %s examples/my-rdma-test/config_1to1.sh'
sudo perf script | ~/FlameGraph/stackcollapse-perf.pl | ~/FlameGraph/flamegraph.pl > QpReuseNetwork.svg
```

</details>

## TODO List

- [x] Debug 模式下构建失败

- [x] `send_finish_normal` (such as `scratch/NormalNetwork.cc`):
    - **Q**： `send_finish_normal` is called unexpectedly
    - **A**：
        - `send_finish_normal` 在消息的最后一个数据包发送时被调用，而没有考虑消息是否成功接收（ACK）
        - 相反，`message_finish_normal` 在消息成功接收（收到 ACK）时被调用

- [x] 冗余的队列对（QP）创建
    - **Q**：当使用多个相同的流定义调用 `ScheduleFlowInputs` 时，系统会为每个输入错误地创建一个新的、独立的队列对，而不是复用现有的 QP。
    - **A**：`#define MAX_QPS 2`

- [x] `msg.m_startSeq` 被重复更新
    - **Q**：成员变量 `msg.m_startSeq` 在处理流程中被更新了两次
        - 一次由 `RdmaQueuePair::PushMessage()` 更新
        - 另一次由 `RdmaHw::QpCompleteMessage()` 更新。
    - **A**：通过引入一个标志位（`msg.m_setupd`）来确保序列号在每个消息的生命周期中只被设置一次，问题已解决。

## Performance Issues

- [ ] `flow` with different start timestamps (`QpReuseNetwork.cc`)
    - **Q**：当`flow_input` 文件中多个流之间存在显著的开始时间差异时，整体模拟时间会增加
    - **Perf**：`ns3::QbbNetDevice::DequeueAndTransmit` 占用大量 CPU 时间

- [ ] Message Size 较大时模拟时间过长 (`msccl/main.cc`)
    - **Q**：在处理大规模数据传输时，模拟时间不成比例地增加，表明存在扩展性瓶颈。
    - **Perf**：`ns3::QbbNetDevice::DequeueAndTransmit` 占用大量 CPU 时间

- [ ] 自动化队列对（QP）的销毁
    - **Q**：目前需要手动调用 `RdmaClient::FinishQp` 来释放队列对资源。


## Design Issues

- **MSCCL 同时 Send 的处理机制**
    - **背景**：在 AllReduce 等环形算法中，一个 GPU 可能需要在同一个逻辑步骤中同时执行 `send` 和 `recv` 操作。比如对于下面的算法：
    ```xml
    <algo name="allreduce_ring_inplace" proto="Simple" nchannels="2" nchunksperloop="2" ngpus="2" coll="allreduce" inplace="1" outofplace="0" minBytes="0" maxBytes="0">
    <gpu id="0" i_chunks="2" o_chunks="0" s_chunks="0">
        <tb id="0" send="1" recv="1" chan="0">
            <step s="0" type="s" srcbuf="i" srcoff="0" dstbuf="i" dstoff="0" cnt="1" depid="-1" deps="-1" hasdep="0"/>
            <step s="1" type="r" srcbuf="i" srcoff="0" dstbuf="i" dstoff="0" cnt="1" depid="-1" deps="-1" hasdep="0"/>
        </tb>
    </gpu>
    <gpu id="1" i_chunks="2" o_chunks="0" s_chunks="0">
        <tb id="0" send="0" recv="0" chan="0">
            <step s="0" type="s" srcbuf="i" srcoff="1" dstbuf="i" dstoff="1" cnt="1" depid="-1" deps="-1" hasdep="0"/>
            <step s="1" type="r" srcbuf="i" srcoff="1" dstbuf="i" dstoff="1" cnt="1" depid="-1" deps="-1" hasdep="0"/>
        </tb>
    </gpu>
    </algo>
    ```
    - **Q**：MSCCL 使用一个 8MB 的缓冲区进行非阻塞发送，这意味着从应用层来看，只要数据被拷贝到该缓冲区，`send` 操作就可以完成。这引出了一个关键问题：在这种机制下，底层的传输协议（RDMA）如何处理缓冲并防止死锁或竞争条件？例如，如果 GPU0 的发送缓冲区已满，而 GPU1 尚未准备好接收，会发生什么？我们需要厘清确切的数据流和同步机制。
        - 两个GPU同时send和recv，那么：
            - **MSCCL会为Send操作预留8MB大小的缓冲区，在缓冲区未满的情况下Send操作是非阻塞的 （此时tb会把数据直接发送到缓冲区，而不必等待对方接收数据）**
        - 如果GPU0的send的数据在GPU1的recv之前已经到达GPU0，如何解决？

