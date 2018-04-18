#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/position-allocator.h"
#include "ns3/command-line.h"
#include "ns3/core-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/propagation-loss-model.h"

#include "ns3/dsdv-module.h"
#include "ns3/wifi-module.h"

#include "ns3/packet-sink-helper.h"
#include "ns3/inet-socket-address.h"

#include <algorithm>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiRange");

std::string phyMode = "OfdmRate24Mbps";
double dist = 100;

int
main (int argc, char* argv[]){

	/*****************************
  * LOGGING                    *
  *****************************/
	//LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
	LogComponentEnable("PacketSink", LOG_LEVEL_INFO);
	//LogComponentEnable("UdpServer", LOG_LEVEL_INFO);
  //LogComponentEnable ("MobilityTest", LOG_LEVEL_INFO);
  //LogComponentEnable ("MobilityHelper", LOG_LEVEL_DEBUG);
  //LogComponentEnable ("EndDeviceLoraPhy", LOG_LEVEL_INFO);
  //LogComponentEnable ("GatewayLoraPhy", LOG_LEVEL_INFO);
  //LogComponentEnable ("GatewayLoraMac", LOG_LEVEL_INFO);
  //LogComponentEnable ("ConstantVelocityHelper", LOG_LEVEL_DEBUG);
  //LogComponentEnable ("VirtualSprings2d", LOG_LEVEL_DEBUG);
  //LogComponentEnable ("PropagationLossModel", LOG_LEVEL_DEBUG);

	LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnableAll (LOG_PREFIX_TIME);

  /***********************
  * Set params           *
  ***********************/

  CommandLine cmd;
  cmd.AddValue ("Distance", "Distance of nodes", dist);
  cmd.Parse (argc, argv);

  /************************
  *  Create the helpers  *
  ************************/

  NS_LOG_INFO ("Setting up helpers...");

  MobilityHelper mobility;

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue(5.0e+09));
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add an upper mac and disable rate control
  WifiMacHelper wifiMac;
  WifiHelper wifi;

  wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));

  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac", "VhtSupported", BooleanValue(true));

  /**********************
  * Create the nodes *
  **********************/

  NS_LOG_INFO ("Creating Nodes");

  // Create a set of nodes
  NodeContainer nodes;
  nodes.Create (2);

  //Assign mobility model to nodes

  Ptr<ListPositionAllocator> nodesAllocator = CreateObject<ListPositionAllocator> ();
  nodesAllocator->Add(Vector(0,0,0));
  nodesAllocator->Add(Vector(dist,0,0));

  mobility.SetPositionAllocator (nodesAllocator);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  /*******************
  * Install devices **
  *******************/

  NetDeviceContainer wifiDevices = wifi.Install (wifiPhy, wifiMac, nodes);


  /************************
  * Set Routing           *
  ************************/
  
  DsdvHelper dsdv;
  //OlsrHelper aodv;
  InternetStackHelper stack;
  stack.SetRoutingHelper (dsdv);
  stack.Install (nodes);
  

  /*************************
  * Install Internet Stack *
  *************************/
  
  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses to WiFi nodes...");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (wifiDevices);

  /************************
  * Install applications  *
  ************************/

  Ipv4Address serverAddress = i.GetAddress(1);
  NS_LOG_INFO("Server Address:" << serverAddress);

  uint16_t port = 4000;
  PacketSinkHelper server ("ns3::UdpSocketFactory", InetSocketAddress(serverAddress, port));
  ApplicationContainer apps = server.Install (nodes.Get (1));
  apps.Start (Seconds (1.0));
  apps.Stop (Minutes (10.0));

  uint32_t MaxPacketSize = 1024;
  uint32_t maxPacketCount = 320;
  UdpClientHelper client (Address(serverAddress), port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
  apps = client.Install (nodes.Get(0));
  apps.Start (Seconds (2.0));
  apps.Stop (Minutes (10.0));


  /********************
  * RUN SIMULATION    *
  ********************/ 

  Simulator::Stop (Minutes (10));
  Simulator::Run ();

  Simulator::Destroy ();
  

}