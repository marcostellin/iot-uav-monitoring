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

std::string phyMode = "DsssRate1Mbps";
double dist = 200;
bool inRange = false;
int nRecv = 0;
std::string filename;

bool fexists(const std::string& filename) {
  std::ifstream ifile(filename.c_str());
  return (bool)ifile;
}

void
RxPacket(Ptr<Packet const> packet, const Address &addr)
{
	NS_LOG_INFO("In Range");
	if (!inRange){
		inRange = !inRange;
	}

  nRecv ++;
}

int
main (int argc, char* argv[]){

	/*****************************
  * LOGGING                    *
  *****************************/
	//LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
	//LogComponentEnable("PacketSink", LOG_LEVEL_INFO);
	//LogComponentEnable("UdpServer", LOG_LEVEL_INFO);
  //LogComponentEnable ("MobilityTest", LOG_LEVEL_INFO);
  //LogComponentEnable ("MobilityHelper", LOG_LEVEL_DEBUG);
  //LogComponentEnable ("EndDeviceLoraPhy", LOG_LEVEL_INFO);
  //LogComponentEnable ("GatewayLoraPhy", LOG_LEVEL_INFO);
  //LogComponentEnable ("GatewayLoraMac", LOG_LEVEL_INFO);
  //LogComponentEnable ("ConstantVelocityHelper", LOG_LEVEL_DEBUG);
  //LogComponentEnable ("VirtualSprings2d", LOG_LEVEL_DEBUG);
  //LogComponentEnable ("PropagationLossModel", LOG_LEVEL_DEBUG);
  //LogComponentEnable ("WifiPhy", LOG_LEVEL_DEBUG);

	LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnableAll (LOG_PREFIX_TIME);

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));

  Config::SetDefault ("ns3::WifiPhy::RxGain", DoubleValue (-10));
  Config::SetDefault ("ns3::WifiPhy::TxGain", DoubleValue (0));

  /***********************
  * Set params           *
  ***********************/

  CommandLine cmd;
  cmd.AddValue ("Distance", "Distance of nodes", dist);
  cmd.AddValue("DataMode", "Mode Used", phyMode);
  cmd.Parse (argc, argv);


  /***********************
  * Print shit           *
  ***********************/
  filename = "wifiRange.out.txt"; 
  bool isFileExistent = fexists(filename);
  std::ofstream out(filename, std::ios_base::app);
  std::streambuf *coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(out.rdbuf());

  /************************
  *  Create the helpers  *
  ************************/
  // Config::SetDefault("ns3::WifiPhy::TxPowerEnd", DoubleValue(14));
  // Config::SetDefault("ns3::WifiPhy::TxPowerStart", DoubleValue(14));
  // Config::SetDefault("ns3::WifiPhy::EnergyDetectionThreshold", DoubleValue(-81));
  // Config::SetDefault("ns3::WifiPhy::ConCcaMode1Threshold", DoubleValue(-84));
  

  NS_LOG_INFO ("Setting up helpers...");

  MobilityHelper mobility;

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // wifiPhy.Set ("RxGain", DoubleValue (-10) );
  // wifiPhy.Set ("TxGain", DoubleValue (-10));
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue(2.4e+09));
  //wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  //wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel", "Exponent", DoubleValue(2.04));
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add an upper mac and disable rate control
  WifiMacHelper wifiMac;
  WifiHelper wifi;

  //wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
  wifi.SetStandard(WIFI_PHY_STANDARD_80211g);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));

  // Set it to adhoc mode
  //wifiMac.SetType ("ns3::AdhocWifiMac", "VhtSupported", BooleanValue(true));
  wifiMac.SetType ("ns3::AdhocWifiMac");

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

  NodeContainer otherNodes;
  otherNodes.Create (4);
  Ptr<UniformDiscPositionAllocator> otherAlloc = CreateObject<UniformDiscPositionAllocator> ();
  otherAlloc -> SetRho (100);

  mobility.SetPositionAllocator (otherAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (otherNodes);

  NodeContainer allNodes;
  allNodes.Add (nodes);
  allNodes.Add (otherNodes);


  /*******************
  * Install devices **
  *******************/

  NetDeviceContainer wifiDevices = wifi.Install (wifiPhy, wifiMac, allNodes);


  /************************
  * Set Routing           *
  ************************/
  
  DsdvHelper dsdv;
  //OlsrHelper aodv;
  InternetStackHelper stack;
  stack.SetRoutingHelper (dsdv);
  stack.Install (allNodes);
  

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

  Ptr<PacketSink> sink = DynamicCast<PacketSink>(apps.Get(0));
  sink -> TraceConnectWithoutContext("Rx", MakeCallback(&RxPacket));

  uint32_t MaxPacketSize = 1024;
  uint32_t maxPacketCount = 100;
  UdpClientHelper client (Address(serverAddress), port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
  apps = client.Install (nodes.Get(0));
  apps.Start (Seconds (2.0));
  apps.Stop (Minutes (10.0));

  apps = client.Install (otherNodes);
  apps.Start (Seconds (2.0));
  apps.Stop (Minutes (10.0));

  /********************
  * RUN SIMULATION    *
  ********************/ 

  Simulator::Stop (Minutes (10));
  Simulator::Run ();

  Simulator::Destroy ();

  if (!isFileExistent){    
      //std::cout << "SF\tnNodes\nGateways\ttInterval\tPDR\tnUnderSens\tnInterf\tnNoRecv" << std::endl;
      std::cout << "DataMode,Dist,InRange" << std::endl;
  }
  
  std::cout << phyMode << "," << std::to_string(dist) << "," << std::to_string(inRange) << "," << nRecv << std::endl;
  std::cout.rdbuf(coutbuf);

}