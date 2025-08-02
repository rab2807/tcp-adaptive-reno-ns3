# TCP Adaptive Reno Implementation

A network simulation project implementing and evaluating TCP Adaptive Reno congestion control algorithm using NS-3.

## Overview

This project implements a novel TCP congestion control algorithm that adaptively adjusts its behavior based on network conditions. TCP Adaptive Reno extends TCP Westwood+ with RTT-based congestion estimation and dynamic window sizing.

## Features

- **TCP Adaptive Reno Algorithm**: Custom congestion control with adaptive behavior
- **Network Simulation**: Dumbbell topology for comparing TCP algorithms
- **Performance Analysis**: Throughput, fairness, and congestion window tracking
- **Configurable Parameters**: Data rates, error rates, and network delays

## Usage

```bash
chmod +x runner.sh
./runner.sh
```

### Parameters
- `bottleneckRate`: Bottleneck bandwidth (Mbps)
- `errorRateExp`: Packet loss rate exponent (10^-x)
- `tcpAlgo1/tcpAlgo2`: TCP algorithms to compare
- `filename`: Output file prefix

## Output Files

- `*_datarate_*.dat`: Throughput vs. data rate analysis
- `*_errorRate_*.dat`: Throughput vs. error rate analysis
- `*_congestion_*.dat`: Congestion window evolution
- `*_jainIndex*.dat`: Fairness analysis results

## Performance Metrics

- **Throughput**: Data transmission rate (Kbps)
- **Fairness**: Jain's Fairness Index between competing flows
- **Congestion Window**: Dynamic window size evolution
