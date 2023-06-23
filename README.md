**Throughput Measurement in NS-3**

This repository contains NS-3 simulation code that measures the end-to-end throughput between two nodes in a simple topology with a point-to-point link. The code allows for varying the latency of the link and provides a latency vs throughput plot. The simulation uses UDP client and server applications to generate traffic.

## Table of Contents
- [Introduction](#introduction)
- [Topology Setup](#topology-setup)
- [Internet Stack and IP Address Assignment](#internet-stack-and-ip-address-assignment)
- [Routing Configuration](#routing-configuration)
- [UDP Client and Server Applications](#udp-client-and-server-applications)
- [Flow Monitoring](#flow-monitoring)
- [Simulation Execution](#simulation-execution)
- [Throughput Calculation](#throughput-calculation)
- [Plotting Latency vs Throughput](#plotting-latency-vs-throughput)
- [Summary](#summary)
- [References](#references)

## Introduction
This NS-3 simulation code measures the end-to-end throughput between two nodes in a simple topology with a point-to-point link. The latency of the link is varied, and the corresponding throughput is calculated and plotted. The simulation uses UDP client and server applications to generate traffic.

## Topology Setup
The code sets up a simple topology consisting of two nodes, Node1 and Node2, connected by a point-to-point link. The link is configured with a data rate of 5 Mbps and a delay of 2 ms.

## Internet Stack and IP Address Assignment
The Internet stack is installed on both nodes using the `InternetStackHelper` class. IP addresses are assigned to the network devices using the `Ipv4AddressHelper` class.

## Routing Configuration
The code uses the `Ipv4GlobalRoutingHelper` class to configure global routing in the network. This ensures that packets can be properly routed between the nodes.

## UDP Client and Server Applications
The code sets up a UDP server application on Node2 and a UDP client application on Node1. The client application generates UDP traffic and sends packets to the server at a fixed data rate. The server application receives the packets and measures the throughput.

## Flow Monitoring
To collect statistics about the traffic flows in the network, the code creates a flow monitor using the `FlowMonitorHelper` class. The flow monitor is installed on the network to capture information about packet flows.

## Simulation Execution
The simulation is executed for a duration of 10 seconds using the `Simulator` class. During the simulation, the client application sends UDP packets to the server application, and the flow monitor collects data about the flows.

## Throughput Calculation
After the simulation, the code calculates the average throughput by dividing the total number of received bytes by the simulation duration. The throughput is measured in Mbps (megabits per second).

## Plotting Latency vs Throughput
To visualize the relationship between latency and throughput, the code generates a plot using Gnuplot. The latency values are extracted from the point-to-point link, and the corresponding throughput values are plotted on the graph.

## Summary
This NS-3 simulation code measures the end-to-end throughput between two nodes in a simple point-to-point topology. By varying the latency of the link, the code demonstrates the impact of latency on the achieved throughput. The average throughput is calculated, and a latency vs throughput plot is generated.

## References
- NS-3 Documentation: [https://www.nsnam.org/documentation/](https://www.nsnam.org/documentation/)
- NS-3 Point-to-Point Channel: [https://www.nsnam.org/docs/release/3.34/models/html/point-to-point-channel.html](https://www.nsnam.org/docs/release/3

.34/models/html/point-to-point-channel.html)
- NS-3 UDP Client and Server Applications: [https://www.nsnam.org/docs/release/3.34/models/html/udp-client-server.html](https://www.nsnam.org/docs/release/3.34/models/html/udp-client-server.html)
- NS-3 Flow Monitor: [https://www.nsnam.org/docs/release/3.34/models/html/flow-monitor.html](https://www.nsnam.org/docs/release/3.34/models/html/flow-monitor.html)
- Gnuplot: [http://www.gnuplot.info/](http://www.gnuplot.info/)
