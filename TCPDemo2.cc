#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/trace-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TCPDemo");

void CwndChange(Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream() << Simulator::Now().GetSeconds() << " " << oldCwnd << " " << newCwnd << std::endl;
}

int main(int argc, char *argv[])
{
  // Create two output files for storing CWND traces
  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream("tcp-demo.cwnd");

  // Set up the simulation parameters
  uint16_t port = 5000;
  double simTime = 10.0;
  std::string tcpVariant = "NewReno";

  // Set the TCP variant to NewReno
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpNewReno::GetTypeId()));

  // Create nodes
  NodeContainer nodes;
  nodes.Create(2);

  // Create point-to-point link
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  // Install internet stack
  InternetStackHelper stack;
  stack.Install(nodes);

  // Create devices and install them on nodes
  NetDeviceContainer devices = pointToPoint.Install(nodes);

  // Assign IP addresses
  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  // Set up routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // Create TCP sender and receiver
  Address receiverAddress(InetSocketAddress(interfaces.GetAddress(1), port));
  PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
  ApplicationContainer receiverApps = packetSinkHelper.Install(nodes.Get(1));
  receiverApps.Start(Seconds(0.0));
  receiverApps.Stop(Seconds(simTime));

  OnOffHelper onOffHelper("ns3::TcpSocketFactory", receiverAddress);
  onOffHelper.SetAttribute("PacketSize", UintegerValue(1000));
  onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

  ApplicationContainer senderApps;
  senderApps.Add(onOffHelper.Install(nodes.Get(0)));
  senderApps.Start(Seconds(1.0));
  senderApps.Stop(Seconds(simTime));

  // Trace the congestion window changes
  Config::ConnectWithoutContext("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback(&CwndChange, stream));

  // Run the simulation
  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  Simulator::Destroy();

  // Plot the congestion window over time
  std::string plotCmd = "gnuplot CW.plt";
  system(plotCmd.c_str());

  // Print the behavior of the congestion window under different scenarios
  std::cout << "Congestion window behavior under different scenarios:" << std::endl;

  // Increase the link bandwidth
  std::cout << "a) Increased link bandwidth" << std::endl;
  std::cout << "   - Scenario: Link bandwidth increased to 10Mbps" << std::endl;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
  stack.Install(nodes);
  devices = pointToPoint.Install(nodes);
  address.Assign(devices);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  senderApps.Reset();
  senderApps.Add(onOffHelper.Install(nodes.Get(0)));
  senderApps.Start(Seconds(1.0));
  senderApps.Stop(Seconds(simTime));
  stream->GetStream()->clear();
  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  Simulator::Destroy();
  system(plotCmd.c_str());
  std::cout << std::endl;

  // Increase the link delay
  std::cout << "b) Increased link delay" << std::endl;
  std::cout << "   - Scenario: Link delay increased to 10ms" << std::endl;
  pointToPoint.SetChannelAttribute("Delay", StringValue("10ms"));
  stack.Install(nodes);
  devices = pointToPoint.Install(nodes);
  address.Assign(devices);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  senderApps.Reset();
  senderApps.Add(onOffHelper.Install(nodes.Get(0)));
  senderApps.Start(Seconds(1.0));
  senderApps.Stop(Seconds(simTime));
  stream->GetStream()->clear();
  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  Simulator::Destroy();
  system(plotCmd.c_str());
  std::cout << std::endl;

  // Increase the error rate
  std::cout << "c) Increased error rate" << std::endl;
  std::cout << "   - Scenario: Link error rate increased to 0.1" << std::endl;
  pointToPoint.SetDeviceAttribute("ReceiveErrorModel", StringValue("ns3::RateErrorModel"));
  pointToPoint.SetDeviceAttribute("ErrorRate", DoubleValue(0.1));
  stack.Install(nodes);
  devices = pointToPoint.Install(nodes);
  address.Assign(devices);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  senderApps.Reset();
  senderApps.Add(onOffHelper.Install(nodes.Get(0)));
  senderApps.Start(Seconds(1.0));
  senderApps.Stop(Seconds(simTime));
  stream->GetStream()->clear();
  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  Simulator::Destroy();
  system(plotCmd.c_str());
  std::cout << std::endl;

  return 0;
}
