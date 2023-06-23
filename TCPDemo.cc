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
  // Write the time, old cwnd, and new cwnd values to the trace file
  *stream->GetStream() << Simulator::Now().GetSeconds() << " " << oldCwnd << " " << newCwnd << std::endl;
}

int main(int argc, char *argv[])
{
  // Set the TCP variant to NewReno
  // Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpNewReno::GetTypeId()));

  // Set the TCP variant to CUBIC
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpCubic::GetTypeId()));

  // Enable logging
  LogComponentEnable("TCPDemo", LOG_LEVEL_INFO);

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
  uint16_t port = 9;
  Address receiverAddress(InetSocketAddress(interfaces.GetAddress(1), port));
  PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
  ApplicationContainer receiverApps = packetSinkHelper.Install(nodes.Get(1));
  receiverApps.Start(Seconds(0.0));
  receiverApps.Stop(Seconds(10.0));

  OnOffHelper onOffHelper("ns3::TcpSocketFactory", receiverAddress);
  onOffHelper.SetAttribute("PacketSize", UintegerValue(1000));
  onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

  ApplicationContainer senderApps;
  senderApps.Add(onOffHelper.Install(nodes.Get(0)));
  senderApps.Start(Seconds(1.0));
  senderApps.Stop(Seconds(10.0));

  // Create trace file for congestion window
  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream("tcp-demo.cwnd");
  Config::ConnectWithoutContext("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback(&CwndChange, stream));

  // Run simulation
  Simulator::Stop(Seconds(10.0));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
