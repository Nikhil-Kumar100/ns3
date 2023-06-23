#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThroughputMeasurement");

double totalRxBytes = 0;

void CalculateThroughput(Ptr<FlowMonitor> flowMonitor, Ptr<Ipv4FlowClassifier> classifier)
{
  FlowMonitor::FlowStatsContainer stats = flowMonitor->GetFlowStats();
  for (auto it = stats.begin(); it != stats.end(); ++it)
  {
    Ipv4FlowClassifier::FiveTuple tuple = classifier->FindFlow(it->first);
    std::cout << "Flow: Source IP=" << tuple.sourceAddress << " Source Port=" << tuple.sourcePort
              << " Destination IP=" << tuple.destinationAddress << " Destination Port=" << tuple.destinationPort
              << std::endl;
    std::cout << "  Tx Bytes: " << it->second.txBytes << std::endl;
    std::cout << "  Rx Bytes: " << it->second.rxBytes << std::endl;

    totalRxBytes += it->second.rxBytes;
  }
}

int main(int argc, char *argv[])
{
  // Create two nodes
  NodeContainer nodes;
  nodes.Create(2);

  // Create point-to-point link
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  // Install internet stack on nodes
  InternetStackHelper stack;
  stack.Install(nodes);

  // Create devices and install them on nodes
  NetDeviceContainer devices = pointToPoint.Install(nodes);

  // Assign IP addresses to devices
  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  // Set up routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // Create UDP client and server applications
  uint16_t serverPort = 5000;
  UdpServerHelper serverHelper(serverPort);
  ApplicationContainer serverApp = serverHelper.Install(nodes.Get(1));
  serverApp.Start(Seconds(0.0));
  serverApp.Stop(Seconds(10.0));

  uint16_t clientPort = 5001;
  Address serverAddress(interfaces.GetAddress(1), serverPort);
  UdpClientHelper clientHelper(serverAddress, clientPort);
  clientHelper.SetAttribute("Interval", TimeValue(Seconds(0.01)));
  clientHelper.SetAttribute("PacketSize", UintegerValue(1024));
  clientHelper.SetAttribute("MaxPackets", UintegerValue(10000));
  ApplicationContainer clientApp = clientHelper.Install(nodes.Get(0));
  clientApp.Start(Seconds(1.0));
  clientApp.Stop(Seconds(10.0));

  // Create a flow monitor
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowMonitorHelper;
  flowMonitor = flowMonitorHelper.InstallAll();

  // Run the simulation
  Simulator::Stop(Seconds(10.0));
  Simulator::Run();
  Simulator::Destroy();

  // Calculate and print the average throughput
  double duration = 10.0; // simulation duration in seconds
  double throughput = totalRxBytes * 8 / duration / 1000000; // Mbps
  std::cout << "Average Throughput: " << throughput << " Mbps" << std::endl;

  // Clean up flow monitor
  flowMonitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowMonitorHelper.GetClassifier());
  CalculateThroughput(flowMonitor, classifier);

  // Plot latency vs throughput graph
  Gnuplot plot("latency_vs_throughput.png");
  plot.SetTitle("Latency vs Throughput");
  plot.SetTerminal("png");
  plot.SetLegend("Latency", "Throughput");
  plot.AppendExtra("set xlabel 'Latency (ms)'");
  plot.AppendExtra("set ylabel 'Throughput (Mbps)'");
  plot.AppendExtra("set grid");
  plot.AppendExtra("plot '-' with linespoints");
  plot.AppendData(pointToPoint.GetChannel()->GetAttribute("Delay").Get<std::string>(), std::to_string(throughput));
  plot.GenerateOutput(std::ofstream("latency_vs_throughput.plt"));

  return 0;
}
