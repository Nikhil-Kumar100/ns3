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
  Ptr<OutputStreamWrapper> newRenoStream = asciiTraceHelper.CreateFileStream("newreno-demo.cwnd");
  Ptr<OutputStreamWrapper> cubicStream = asciiTraceHelper.CreateFileStream("cubic-demo.cwnd");

  // Set up the simulation parameters
  uint16_t port = 5000;
  double simTime = 10.0;
  std::string tcpVariant;

  // Set the TCP variant to NewReno
  tcpVariant = "NewReno";
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

  // Create TCP sender and receiver (NewReno)
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

  // Trace the congestion window changes for NewReno
  Config::ConnectWithoutContext("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback(&CwndChange, newRenoStream));

  // Run the simulation with NewReno
  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  Simulator::Destroy();

  // Set the TCP variant to CUBIC
  tcpVariant = "CUBIC";
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpCubic::GetTypeId()));

  // Reset the network
  stack.Reset();
  devices = pointToPoint.Install(nodes);
  address.Assign(devices);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // Create TCP sender and receiver (CUBIC)
  receiverApps = packetSinkHelper.Install(nodes.Get(1));
  receiverApps.Start(Seconds(0.0));
  receiverApps.Stop(Seconds(simTime));

  senderApps.Get(0)->SetAttribute("Remote", AddressValue(receiverAddress));

  // Trace the congestion window changes for CUBIC
  Config::ConnectWithoutContext("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback(&CwndChange, cubicStream));

  // Run the simulation with CUBIC
  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  Simulator::Destroy();

  // Compare the congestion window changes
  std::cout << "TCP Variant: " << tcpVariant << std::endl;
  std::cout << "Number of times cwnd reduced: " << std::endl;

  if (tcpVariant == "NewReno") {
    Ptr<InputStreamWrapper> inputStream = asciiTraceHelper.CreateFileStream("newreno-demo.cwnd");
    uint32_t count = 0;
    double time, oldCwnd, newCwnd;
    while (*inputStream->GetStream() >> time >> oldCwnd >> newCwnd) {
      if (newCwnd < oldCwnd)
        count++;
    }
    std::cout << count << std::endl;
  } else if (tcpVariant == "CUBIC") {
    Ptr<InputStreamWrapper> inputStream = asciiTraceHelper.CreateFileStream("cubic-demo.cwnd");
    uint32_t count = 0;
    double time, oldCwnd, newCwnd;
    while (*inputStream->GetStream() >> time >> oldCwnd >> newCwnd) {
      if (newCwnd < oldCwnd)
        count++;
    }
    std::cout << count << std::endl;
  }

  return 0;
}

