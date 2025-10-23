conf:
	./ns3 configure -d debug --enable-examples --disable-mtp

msccl-perf:
	./ns3 run 'scratch/msccl/main' --command-template='sudo perf record -F 99 -g %s examples/allstack/config.sh'
	sudo perf script | ~/FlameGraph/stackcollapse-perf.pl | ~/FlameGraph/flamegraph.pl > ns3-msccl.svg

msccl-debug:
	./ns3 configure -d debug --enable-examples --disable-mtp
	./ns3 run 'scratch/msccl/main' --command-template='gdb --args %s examples/allstack/config.sh'

msccl-run:
	./ns3 run 'scratch/msccl/main' --command-template='%s examples/allstack/config.sh'

msccl-runs:
	./ns3 run 'scratch/msccl/main' --command-template='mpirun -np 4 %s examples/allstack/config.sh'

.PHONY: conf msccl-perf msccl-debug msccl-run msccl-runs
