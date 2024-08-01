/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Tim Schubert <ns-3-leo@timschubert.net>
 */

#include <iostream>

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/leo-module.h"
#include "ns3/network-module.h"
#include "ns3/aodv-module.h"
#include "ns3/udp-server.h"
// #include "ns3/epidemic-routing-module.h"

#include "ns3/gpsr-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/v4ping-helper.h"
#include "ns3/udp-echo-server.h"
#include "ns3/udp-echo-client.h"
#include "ns3/udp-echo-helper.h"
#include <cmath>

#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"

#include "ns3/netanim-module.h"

using namespace ns3;
using namespace dsr;

// static void
// EchoTxRx (std::string context, const Ptr< const Packet > packet, const TcpHeader &header, const Ptr< const TcpSocketBase > socket)
// {
//   std::cout << Simulator::Now () << ":" << context << ":" << packet->GetUid() << ":" << socket->GetNode () << ":" << header.GetSequenceNumber () << std::endl;
// }

NS_LOG_COMPONENT_DEFINE("LeoBulkSendTracingExample");

int main(int argc, char *argv[])
{

  CommandLine cmd;
  std::string orbitFile;
  std::string traceFile;
  LeoLatLong source(51.399, 10.536);
  LeoLatLong destination(40.76, -73.96);
  std::string islRate = "2Gbps";
  std::string constellation = "TelesatGateway";
  uint16_t port = 9;
  // uint32_t latGws = 2;
  // uint32_t lonGws = 2;
  uint32_t latGws = 20;
  uint32_t lonGws = 20;
  double duration = 100;
  bool islEnabled = true;
  bool pcap = true;
  uint64_t ttlThresh = 0;
  std::string routingProto = "aodv";

  cmd.AddValue("orbitFile", "CSV file with orbit parameters", orbitFile);
  cmd.AddValue("traceFile", "CSV file to store mobility trace in", traceFile);
  cmd.AddValue("precision", "ns3::LeoCircularOrbitMobilityModel::Precision");
  cmd.AddValue("duration", "Duration of the simulation in seconds", duration);
  cmd.AddValue("source", "Traffic source", source);
  cmd.AddValue("destination", "Traffic destination", destination);
  cmd.AddValue("islRate", "ns3::MockNetDevice::DataRate");
  cmd.AddValue("constellation", "LEO constellation link settings name", constellation);
  cmd.AddValue("routing", "Routing protocol", routingProto);
  cmd.AddValue("islEnabled", "Enable inter-satellite links", islEnabled);
  cmd.AddValue("latGws", "Latitudal rows of gateways", latGws);
  cmd.AddValue("lonGws", "Longitudinal rows of gateways", lonGws);
  cmd.AddValue("ttlThresh", "TTL threshold", ttlThresh);
  cmd.AddValue("destOnly", "ns3::aodv::RoutingProtocol::DestinationOnly");
  cmd.AddValue("routeTimeout", "ns3::aodv::RoutingProtocol::ActiveRouteTimeout");
  cmd.AddValue("pcap", "Enable packet capture", pcap);
  cmd.Parse(argc, argv);

  std::streambuf *coutbuf = std::cout.rdbuf();
  // redirect cout if traceFile
  std::ofstream out;
  out.open(traceFile);
  if (out.is_open())
  {
    std::cout.rdbuf(out.rdbuf());
  }

  LeoOrbitNodeHelper orbit;
  NodeContainer satellites;
  if (!orbitFile.empty())
  {
    satellites = orbit.Install(orbitFile);
  }
  else
  {
    satellites = orbit.Install({LeoOrbit(1200, 20, 32, 16),
                                LeoOrbit(1180, 30, 12, 10)});
  }

  // NS_LOG_UNCOND("--------Satellites----------" << satellites);

  // std::cerr << satellites << std::endl;

  // Display the contents of the satellites variable
  for (uint32_t i = 0; i < satellites.GetN(); ++i)
  {
    Ptr<Node> node = satellites.Get(i);
    std::cerr << "Satellite " << i << ": " << node << std::endl;
  }

  LeoGndNodeHelper ground;
  NodeContainer stations = ground.Install(latGws, lonGws);

  // Display the contents of the stations variable
  for (uint32_t i = 0; i < stations.GetN(); ++i)
  {
    Ptr<Node> node = stations.Get(i);
    std::cerr << "Station " << i << ": " << node << std::endl;
  }

  NodeContainer users = ground.Install(source, destination);
  // Display the contents of the users variable
  for (uint32_t i = 0; i < users.GetN(); ++i)
  {
    Ptr<Node> node = users.Get(i);
    std::cerr << "User " << i << ": " << node << std::endl;
  }

  stations.Add(users);

  LeoChannelHelper utCh;
  utCh.SetConstellation(constellation);
  NetDeviceContainer utNet = utCh.Install(satellites, stations);
  // NetDeviceContainer utNet = utCh.Install(mainNodes);

  //
  // NodeContainer mainNodes;
  // mainNodes.Add(satellites);
  // mainNodes.Add(stations);
  //

  InternetStackHelper stack;

  // // Create an InternetStackHelper for satellites
  // InternetStackHelper satelliteStack;
  // InternetStackHelper groundStack;

  DsdvHelper dsdv;
  DsrHelper dsr;
  DsrMainHelper dsrMain;

  GpsrHelper gpsr;

  // stack.SetRoutingHelper (gpsr);
  // stack.SetRoutingHelper (dsdv);

  AodvHelper aodv;

  aodv.Set("EnableHello", BooleanValue(false));
  // aodv.Set ("HelloInterval", TimeValue (Seconds (10)));
  if (ttlThresh != 0)
  {
    aodv.Set("TtlThreshold", UintegerValue(ttlThresh));
    aodv.Set("NetDiameter", UintegerValue(2 * ttlThresh));
  }
  stack.SetRoutingHelper(aodv);

  // Install internet stack on nodes

  // stack.Install(satellites);
  // dsrMain.Install(dsr, satellites);

  // stack.Install(stations);
  // dsrMain.Install(dsr, stations);

  // stack.Install(mainNodes);
  // dsrMain.Install(dsr, mainNodes);

  stack.Install(satellites);
  stack.Install(stations);

  // satelliteStack.SetRoutingHelper(aodv);
  // satelliteStack.Install(satellites);
  // groundStack.Install(stations);

  Ipv4AddressHelper ipv4;

  ipv4.SetBase("10.1.0.0", "255.255.0.0");
  ipv4.Assign(utNet);

  if (islEnabled)
  {
    std::cerr << "ISL enabled" << std::endl;
    IslHelper islCh;
    NetDeviceContainer islNet = islCh.Install(satellites);
    ipv4.SetBase("10.2.0.0", "255.255.0.0");
    ipv4.Assign(islNet);
  }

  Ipv4Address remote = users.Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
  BulkSendHelper sender("ns3::TcpSocketFactory",
                        // BulkSendHelper sender ("ns3::UdpSocketFactory",
                        InetSocketAddress(remote, port));
  // Set the amount of data to send in bytes.  Zero is unlimited.
  sender.SetAttribute("MaxBytes", UintegerValue(0));
  // sender.SetAttribute ("MaxBytes", UintegerValue(7500000000));
  ApplicationContainer sourceApps = sender.Install(users.Get(0));
  sourceApps.Start(Seconds(0.0));

  //
  // Create a PacketSinkApplication and install it on node 1
  //
  PacketSinkHelper sink("ns3::TcpSocketFactory",
                        // PacketSinkHelper sink ("ns3::UdpSocketFactory",
                        InetSocketAddress(Ipv4Address::GetAny(), port));
  ApplicationContainer sinkApps = sink.Install(users.Get(1));
  sinkApps.Start(Seconds(0.0));

  // Config::Connect ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/Tx",
  // 		   MakeCallback (&EchoTxRx));
  // Config::Connect ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/Rx",
  // 		   MakeCallback (&EchoTxRx));

  //
  // Set up tracing if enabled
  //
  if (pcap)
  {
    AsciiTraceHelper ascii;
    utCh.EnableAsciiAll(ascii.CreateFileStream("tcp-bulk-send.tr"));
    utCh.EnablePcapAll("tcp-bulk-send", false);
  }

  std::cerr << "LOCAL =" << users.Get(0)->GetId() << std::endl;
  std::cerr << "REMOTE=" << users.Get(1)->GetId() << ",addr=" << Ipv4Address::ConvertFrom(remote) << std::endl;

  AnimationInterface anim("leo-bulk-send.anim.xml");

  for (uint32_t i = 0; i < satellites.GetN(); ++i)
  {
    anim.UpdateNodeDescription(satellites.Get(i)->GetId(), "sat"); // Drones
    anim.UpdateNodeColor(satellites.Get(i)->GetId(), 255, 0, 0);
    anim.UpdateNodeSize(satellites.Get(i)->GetId(), 20, 20);
  }
  for (uint32_t i = 0; i < stations.GetN(); ++i)
  {
    anim.UpdateNodeDescription(stations.Get(i)->GetId(), "ST"); // Server
    anim.UpdateNodeColor(stations.Get(i)->GetId(), 0, 255, 0);
    anim.UpdateNodeSize(stations.Get(i)->GetId(), 20, 20);
  }
  for (uint32_t i = 0; i < users.GetN(); ++i)
  {
    anim.UpdateNodeDescription(users.Get(i)->GetId(), "USR"); // Server
    anim.UpdateNodeColor(users.Get(i)->GetId(), 0, 0, 255);
    anim.UpdateNodeSize(users.Get(i)->GetId(), 20, 20);
  }

  anim.EnablePacketMetadata(); // Optional

  NS_LOG_INFO("Run Simulation.");
  Simulator::Stop(Seconds(duration));
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  Ptr<PacketSink> sink1 = DynamicCast<PacketSink>(sinkApps.Get(0));
  std::cout << users.Get(0)->GetId() << ":" << users.Get(1)->GetId() << ": " << sink1->GetTotalRx() << std::endl;

  out.close();
  std::cout.rdbuf(coutbuf);

  return 0;
}
