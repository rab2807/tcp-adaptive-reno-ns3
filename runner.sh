#!/bin/bash

rm ./1905024/*

start_time=$(date +%s)

./ns3 run "scratch/1905024_1 --filename=congestion --tcpAlgo2=TcpAdaptiveReno"
gnuplot <<- EOF
	set terminal png size 640,640
	set output "./1905024/tcpNewReno_tcpAdaptiveReno_congestion_vs_time.png"
	set xlabel "Time (s)"
	set ylabel "Congestion Window Size (bytes)"
	plot "./1905024/TcpNewReno_TcpAdaptiveReno_congestion_1.dat" using 1:2 title "TcpNewReno" with linespoints, \
		"./1905024/TcpNewReno_TcpAdaptiveReno_congestion_2.dat" using 1:2 title "TcpAdaptiveReno" with linespoints
	exit
EOF

echo !!!!!! running with varying bottleneck data rate...
for ((i=1; i<=256; i*=2)); do
	./ns3 run "scratch/1905024_1 --bottleneckRate=$i --filename=datarate --tcpAlgo2=TcpAdaptiveReno"
done

gnuplot <<- EOF
	set terminal png size 640,640
	set output "./1905024/tcpNewReno_tcpAdaptiveReno_thrpt_vs_datarate.png"
	set xlabel "Bottleneck Datarate (Mbps)"
	set ylabel "Throughput (Kbps)"
	plot "./1905024/TcpNewReno_TcpAdaptiveReno_datarate_1.dat" using 1:2 title "TcpNewReno" with linespoints, \
		"./1905024/TcpNewReno_TcpAdaptiveReno_datarate_2.dat" using 1:2 title "TcpAdaptiveReno" with linespoints
EOF

echo !!!!!! running with varying error rate...
for ((i=2; i<=6; i+=1)); do
	./ns3 run "scratch/1905024_1 --errorRateExp=$i --filename=errorRate --tcpAlgo2=TcpAdaptiveReno"
done

gnuplot <<- EOF
	set terminal png size 640,640
	set output "./1905024/tcpNewReno_tcpAdaptiveReno_thrpt_vs_errorRate.png"
	set xlabel "Packet Loss Rate"
	set ylabel "Throughput (Kbps)"
	plot "./1905024/TcpNewReno_TcpAdaptiveReno_errorRate_1.dat" using 1:2 title "TcpNewReno" with linespoints, \
		"./1905024/TcpNewReno_TcpAdaptiveReno_errorRate_2.dat" using 1:2 title "TcpAdaptiveReno" with linespoints
	exit
EOF

echo !!!!!! jain index with varying bottleneck data rate...
for ((i=1; i<=256; i*=2)); do
	./ns3 run "scratch/1905024_1 --bottleneckRate=$i --filename=jainIndexDatarate --tcpAlgo2=TcpAdaptiveReno"
done

gnuplot <<- EOF
	set terminal png size 640,640
	set output "./1905024/tcpNewReno_tcpAdaptiveReno_jainIndex_vs_datarate.png"
	set xlabel "Bottleneck Datarate (Mbps)"
	set ylabel "Throughput (Kbps)"
	plot "./1905024/TcpNewReno_TcpAdaptiveReno_jainIndexDatarate_1.dat" using 1:2 title "" with linespoints
EOF

echo !!!!!! jain index with varying error rate...
for ((i=2; i<=6; i+=1)); do
	./ns3 run "scratch/1905024_1 --errorRateExp=$i --filename=jainIndexErrorRate --tcpAlgo2=TcpAdaptiveReno"
done

gnuplot <<- EOF
	set terminal png size 640,640
	set output "./1905024/tcpNewReno_tcpAdaptiveReno_jainIndex_vs_errorRate.png"
	set xlabel "Bottleneck Datarate (Mbps)"
	set ylabel "Throughput (Kbps)"
	plot "./1905024/TcpNewReno_TcpAdaptiveReno_jainIndexErrorRate_1.dat" using 1:2 title "" with linespoints
EOF

# ---

./ns3 run "scratch/1905024_1 --filename=congestion --tcpAlgo2=TcpHighSpeed"
gnuplot <<- EOF
	set terminal png size 640,640
	set output "./1905024/tcpNewReno_tcpHighSpeed_congestion_vs_time.png"
	set xlabel "Time (s)"
	set ylabel "Congestion Window Size (bytes)"
	plot "./1905024/TcpNewReno_TcpHighSpeed_congestion_1.dat" using 1:2 title "TcpNewReno" with linespoints, \
		"./1905024/TcpNewReno_TcpHighSpeed_congestion_2.dat" using 1:2 title "TcpHighSpeed" with linespoints
	exit
EOF

echo !!!!!! running with varying bottleneck data rate...
for ((i=1; i<=256; i*=2)); do
	./ns3 run "scratch/1905024_1 --bottleneckRate=$i --filename=datarate --tcpAlgo2=TcpHighSpeed"
done

gnuplot <<- EOF
	set terminal png size 640,640
	set output "./1905024/tcpNewReno_tcpHighSpeed_thrpt_vs_datarate.png"
	set xlabel "Bottleneck Datarate (Mbps)"
	set ylabel "Throughput (Kbps)"
	plot "./1905024/TcpNewReno_TcpHighSpeed_datarate_1.dat" using 1:2 title "TcpNewReno" with linespoints, \
		"./1905024/TcpNewReno_TcpHighSpeed_datarate_2.dat" using 1:2 title "TcpHighSpeed" with linespoints
EOF

echo !!!!!! running with varying error rate...
for ((i=2; i<=6; i+=1)); do
	./ns3 run "scratch/1905024_1 --errorRateExp=$i --filename=errorRate --tcpAlgo2=TcpHighSpeed"
done

gnuplot <<- EOF
	set terminal png size 640,640
	set output "./1905024/tcpNewReno_tcpHighSpeed_thrpt_vs_errorRate.png"
	set xlabel "Packet Loss Rate"
	set ylabel "Throughput (Kbps)"
	plot "./1905024/TcpNewReno_TcpHighSpeed_errorRate_1.dat" using 1:2 title "TcpNewReno" with linespoints, \
		"./1905024/TcpNewReno_TcpHighSpeed_errorRate_2.dat" using 1:2 title "TcpHighSpeed" with linespoints
	exit
EOF

echo !!!!!! jain index with varying bottleneck data rate...
for ((i=1; i<=256; i*=2)); do
	./ns3 run "scratch/1905024_1 --bottleneckRate=$i --filename=jainIndexDatarate --tcpAlgo2=TcpHighSpeed"
done

gnuplot <<- EOF
	set terminal png size 640,640
	set output "./1905024/tcpNewReno_tcpHighSpeed_jainIndex_vs_datarate.png"
	set xlabel "Bottleneck Datarate (Mbps)"
	set ylabel "Index"
	plot "./1905024/TcpNewReno_TcpHighSpeed_jainIndexDatarate_1.dat" using 1:2 title "" with linespoints
EOF

echo !!!!!! jain index with varying error rate...
for ((i=2; i<=6; i+=1)); do
	./ns3 run "scratch/1905024_1 --errorRateExp=$i --filename=jainIndexErrorRate --tcpAlgo2=TcpHighSpeed"
done

gnuplot <<- EOF
	set terminal png size 640,640
	set output "./1905024/tcpNewReno_tcpHighSpeed_jainIndex_vs_errorRate.png"
	set xlabel "Bottleneck Datarate (Mbps)"
	set ylabel "Index"
	plot "./1905024/TcpNewReno_TcpHighSpeed_jainIndexErrorRate_1.dat" using 1:2 title "" with linespoints
EOF

end_time=$(date +%s)
difference=$(( end_time - start_time ))
echo "Total time taken (in seconds) : $difference s"
