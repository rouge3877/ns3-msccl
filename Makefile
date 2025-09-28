# Export all variables to subshells
.EXPORT_ALL_VARIABLES:

# Set default NS_LOG to empty
NS_LOG ?= 

conf:
	./ns3 configure -d debug --enable-examples --disable-mtp

log:
	$(eval NS_LOG=)

log-rdma:
	$(eval NS_LOG=RdmaDriver=level_info|prefix_all:RdmaQueuePair=level_info|prefix_all)

log-scheduler:
	$(eval NS_LOG=DefaultSimulatorImpl=all|prefix_all)


msccl-perf:
	./ns3 run 'scratch/msccl/main' --command-template='sudo perf record -F 99 -g %s examples/allstack/config.sh'
	sudo perf script | ~/FlameGraph/stackcollapse-perf.pl | ~/FlameGraph/flamegraph.pl > ns3-msccl.svg

msccl-debug:
	./ns3 run 'scratch/msccl/main' --command-template='gdb --args %s examples/allstack/config.sh'
	
qpreuse-perf:
	./ns3 run 'scratch/QpReuseNetwork' --command-template='sudo perf record -F 99 -g %s examples/my-rdma-test/config_1to1.sh'
	sudo perf script | ~/FlameGraph/stackcollapse-perf.pl | ~/FlameGraph/flamegraph.pl > qp-reuse.svg

qpreuse-debug:
	./ns3 run 'scratch/QpReuseNetwork' --command-template='gdb --args %s examples/my-rdma-test/config_1to1.sh'

msccl-run:
	./ns3 run 'scratch/msccl/main' --command-template='%s examples/allstack/config.sh'

qpreuse-run:
	./ns3 run 'scratch/QpReuseNetwork' --command-template='%s examples/my-rdma-test/config_1to1.sh'

.PHONY: phony conf log msccl-perf msccl-debug qpreuse-perf qpreuse-debug msccl-run qpreuse-run