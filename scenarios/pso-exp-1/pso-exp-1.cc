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

#include "ns3/constant-position-mobility-model.h"
#include "ns3/firemen-mobility-model.h"

#include "ns3/end-device-lora-phy.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/end-device-lora-mac.h"
#include "ns3/gateway-lora-mac.h"
#include "ns3/lora-helper.h"
#include "ns3/periodic-sender-helper.h"

#include "ns3/internet-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/wifi-module.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/olsr-helper.h"
#include "ns3/olsr-routing-protocol.h"

#include "ns3/udp-relay-server-helper.h"

#include "ns3/pso-manager.h"

#include <algorithm>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include <utility>
#include <unordered_set>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("PsoExp1");

std::string filename;

std::string phyMode ("ErpOfdmRate12Mbps");
uint32_t numTeams = 3;
uint32_t numUavs = 15;
uint32_t numMembers = 20;
double runtime = 3000;


struct PacketStatus {
  Ptr<Packet const> packet;
  uint32_t senderId;
  bool recv;
  //std::vector<enum PacketOutcome> outcomes;
};

std::map<Ptr<Packet const>, PacketStatus> packetTracker;
std::map<uint64_t, PacketStatus> packetRelayedTracker;
std::map<uint32_t, uint32_t> droppedPacketsTracker;


/************************************
* Callbacks                         *
************************************/


bool
ReceiveFromLora (Ptr<NetDevice> loraNetDevice, Ptr<const Packet> packet, uint16_t protocol, const Address& sender)
{
  //NS_LOG_DEBUG("ReceiveFromLora Callback!");
  //NS_LOG_DEBUG(Simulator::Now().GetSeconds());
  
  Ptr<Node> node = loraNetDevice ->GetNode();
  Ptr<UdpRelay> relay = DynamicCast<UdpRelay>(node -> GetApplication(0));
  Ptr<Packet> relayedPkt = packet -> Copy();

  //NS_LOG_DEBUG ("Original Packet UID " << packet -> GetUid ());
  //NS_LOG_DEBUG ("Copied Packet UID " << relayedPkt -> GetUid ());

  PacketStatus status;
  status.packet = packet;
  status.recv = false;

  packetRelayedTracker.insert (std::pair<uint64_t, PacketStatus> (packet -> GetUid (), status));

  bool sent = relay -> Send(relayedPkt);

  if (!sent)
  {
  	std::map<uint32_t, uint32_t>::iterator it;
  	it = droppedPacketsTracker.find (node->GetId ());

  	if ( it != droppedPacketsTracker.end () )
  	{
  		(*it).second ++;
  	} 
  	else 
  	{
  		droppedPacketsTracker.insert (std::pair<uint32_t, uint32_t> (node -> GetId (), 1 ));
  	}
  }

  return true;
}


void
ReceiveFromBs(Ptr<const Packet> packet, const Address &addr)
{
  //NS_LOG_UNCOND("TestRx");
  std::map<uint64_t, PacketStatus>::iterator it = packetRelayedTracker.find (packet -> GetUid ());
  (*it).second.recv = true;
}


/********************
/ Utility functions *
********************/


int
main (int argc, char* argv[]){

  /*****************************
  * LOGGING                    *
  *****************************/
	//LogComponentEnable("UdpRelay", LOG_DEBUG);
	//LogComponentEnable("UdpServer", LOG_LEVEL_INFO);
	//LogComponentEnable ("PacketSink", LOG_INFO);
  //LogComponentEnable ("FiremenMobilityModel", LOG_LEVEL_DEBUG);
  //LogComponentEnable ("SeedsManager", LOG_INFO);
  //LogComponentEnable ("VsfExp3", LOG_INFO);

  LogComponentEnable ("PsoManager", LOG_INFO);
  LogComponentEnable ("Particle", LOG_INFO);

	LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnableAll (LOG_PREFIX_TIME);

  RngSeedManager::SetSeed (3);  // Changes seed from default of 1 to 3
  RngSeedManager::SetRun (95);  //Best seed 56 //58 is difficult!
  
  /***********************
  * Set params           *
  ***********************/

  CommandLine cmd;
  cmd.AddValue ("NumTeams", "Number of teams", numTeams);
  cmd.AddValue("NumUAVs", "Number of UAVs", numUavs);
  cmd.AddValue("NumMembers", "Number of team members", numMembers);
  cmd.Parse (argc, argv);

  /************************
  * Config Defaults       *
  ************************/

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));

  /***********************
  * Schedule events      *
  ***********************/



  /************************
  *  Create the channels  *
  ************************/

  NS_LOG_INFO ("Creating the AtG channel...");

  Ptr<LogDistancePropagationLossModel> medLoss = CreateObject<LogDistancePropagationLossModel> ();
  medLoss -> SetReference(1, 31.23);
  medLoss -> SetPathLossExponent(5.2);

  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();

  Ptr<LoraChannel> channel = CreateObject<LoraChannel> (medLoss, delay);

  //Friis Loss Model
  Ptr<FriisPropagationLossModel> friisLoss = CreateObject<FriisPropagationLossModel> ();
  friisLoss -> SetFrequency (2.4e+09);

  /************************
  *  Create the helpers  *
  ************************/

  NS_LOG_INFO ("Setting up helpers...");

  MobilityHelper mobility;

  // Create the LoraPhyHelper
  LoraPhyHelper phyHelper = LoraPhyHelper ();
  phyHelper.SetChannel (channel);

  // Create the LoraMacHelper
  LoraMacHelper macHelper = LoraMacHelper ();

  // Create the LoraHelper
  LoraHelper helper = LoraHelper ();

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  wifiPhy.Set ("RxGain", DoubleValue (-10));
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue(2.4e+09));
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add an upper mac and disable rate control
  WifiMacHelper wifiMac;
  WifiHelper wifi;

  wifi.SetStandard (WIFI_PHY_STANDARD_80211g);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));

  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");

  /*******************
  * Create EDs       *
  *******************/

  NS_LOG_INFO ("Creating end devices...");

  // Create a set of nodes
  NodeContainer endDevices;
  NodeContainer team0;
  NodeContainer team1;
  NodeContainer team2;
  NodeContainer team3;
  NodeContainer team4;

  switch (numTeams)
  {
    case (1):
      team0.Create (numMembers);
      break;
    case (2):
      team0.Create (numMembers);
      team1.Create (numMembers);
      break;
    case (3):
      team0.Create (numMembers);
      team1.Create (numMembers);
      team2.Create (numMembers);
      break;
    case (4):
      team0.Create (numMembers);
      team1.Create (numMembers);
      team2.Create (numMembers);
      team3.Create (numMembers);
      break;
    case (5):
      team0.Create (numMembers);
      team1.Create (numMembers);
      team2.Create (numMembers);
      team3.Create (numMembers);
      team4.Create (numMembers);
      break;
  }

  endDevices.Add (team0);
  endDevices.Add (team1);
  endDevices.Add (team2);
  endDevices.Add (team3);
  endDevices.Add (team4);


  // Assign a mobility model to team0
  Rectangle area = Rectangle (0, 2000, 0, 2000);
  Ptr<UniformDiscPositionAllocator> edsAllocator = CreateObject<UniformDiscPositionAllocator> ();
  edsAllocator->SetRho(50);
  edsAllocator->SetX(area.xMax / 2 - 100);
  edsAllocator->SetY(100);

  mobility.SetPositionAllocator (edsAllocator);
  mobility.SetMobilityModel ("ns3::FiremenMobilityModel",
                             "PropagationArea", RectangleValue(area),
                             "SpreadY", DoubleValue(100),
                             "NumTeams", IntegerValue(numTeams),
                             "TeamId", IntegerValue (0));
  mobility.Install (team0);

  // Assign a mobility model to team1
  edsAllocator->SetRho(50);
  edsAllocator->SetX(area.xMax / 2 + 100);
  edsAllocator->SetY(100);

  mobility.SetPositionAllocator (edsAllocator);
  mobility.SetMobilityModel ("ns3::FiremenMobilityModel",
                             "PropagationArea", RectangleValue(area),
                             "SpreadY", DoubleValue(100),
                             "NumTeams", IntegerValue(numTeams),
                             "TeamId", IntegerValue (1));
  mobility.Install (team1);

  // Assign a mobility model to team2
  edsAllocator->SetRho(50);
  edsAllocator->SetX(area.xMax / 2 + 100);
  edsAllocator->SetY(250.0);

  mobility.SetPositionAllocator (edsAllocator);
  mobility.SetMobilityModel ("ns3::FiremenMobilityModel",
                             "PropagationArea", RectangleValue(area),
                             "SpreadY", DoubleValue(100),
                             "NumTeams", IntegerValue(numTeams),
                             "TeamId", IntegerValue (2));
  mobility.Install (team2);

  // Assign a mobility model to team3
  edsAllocator->SetRho(10);
  edsAllocator->SetX(150.0);
  edsAllocator->SetY(200.0);

  mobility.SetPositionAllocator (edsAllocator);
  mobility.SetMobilityModel ("ns3::FiremenMobilityModel",
                             "PropagationArea", RectangleValue(area),
                             "SpreadY", DoubleValue(100),
                             "NumTeams", IntegerValue(numTeams),
                             "TeamId", IntegerValue (3));
  mobility.Install (team3);

  // Assign a mobility model to team4
  edsAllocator->SetRho(10);
  edsAllocator->SetX(250.0);
  edsAllocator->SetY(200.0);

  mobility.SetPositionAllocator (edsAllocator);
  mobility.SetMobilityModel ("ns3::FiremenMobilityModel",
                             "PropagationArea", RectangleValue(area),
                             "SpreadY", DoubleValue(100),
                             "NumTeams", IntegerValue(numTeams),
                             "TeamId", IntegerValue (4));
  mobility.Install (team4);

  phyHelper.SetDeviceType (LoraPhyHelper::ED);
  macHelper.SetDeviceType (LoraMacHelper::ED);
  helper.Install (phyHelper, macHelper, endDevices);

  /************************
  *  Create UAVs          *
  ************************/

  NS_LOG_INFO ("Creating UAV devices...");

  // Create a set of nodes
  NodeContainer ataNodes;
  ataNodes.Create (numUavs);

  Vector bsPos = Vector(area.xMax / 2, 100 ,0);
  
  //Assign mobility model to nodes

  Ptr<UniformDiscPositionAllocator> nodesAllocator = CreateObject<UniformDiscPositionAllocator> ();
  nodesAllocator -> SetRho (200);  //50
  nodesAllocator -> SetX (bsPos.x);
  nodesAllocator -> SetY (bsPos.y);

  mobility.SetPositionAllocator (nodesAllocator);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ataNodes);
  
  phyHelper.SetDeviceType (LoraPhyHelper::GW);
  macHelper.SetDeviceType (LoraMacHelper::GW);
  helper.Install (phyHelper, macHelper, ataNodes);

  /*******************
  * Create BS        *
  *******************/

  NodeContainer bsNodes;
  bsNodes.Create(1);

  Ptr<ListPositionAllocator> bsAllocator = CreateObject<ListPositionAllocator>();
  bsAllocator -> Add(bsPos);

  mobility.SetPositionAllocator(bsAllocator);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(bsNodes);


  /*******************
  * Install devices **
  *******************/
  NodeContainer wifiNodes;
  wifiNodes.Add(bsNodes);
  wifiNodes.Add(ataNodes);

  NetDeviceContainer wifiDevices = wifi.Install (wifiPhy, wifiMac, wifiNodes);

  /************************
  * Set Routing           *
  ************************/
  
  OlsrHelper olsr;
  InternetStackHelper stack;
  stack.SetRoutingHelper (olsr);
  stack.Install (wifiNodes);


  /*************************
  * Install Internet Stack *
  *************************/
  
  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses to WiFi nodes...");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (wifiDevices);

  /************************
  * Set PSO manager       *
  ************************/

  Ptr<PsoManager> psoManager = CreateObject<PsoManager> ();
    
  //Add AtA nodes
  for (NodeContainer::Iterator it = ataNodes.Begin (); it != ataNodes.End (); ++it)
  {
    Ptr<Node> node = (*it);
    psoManager -> AddAtaNode (node -> GetId ());
  }

  //Add AtG nodes
  for (NodeContainer::Iterator it = endDevices.Begin (); it != endDevices.End (); ++it)
  {
    Ptr<Node> node = (*it);
    psoManager -> AddAtgNode (node -> GetId ());
  }

  psoManager -> DoInitialize ();



  /************************
  * Install applications  *
  ************************/

  Address serverAddress = Address(i.GetAddress(0));
  NS_LOG_INFO("Server Address:");
  NS_LOG_INFO(i.GetAddress(0));
  

  uint16_t port = 4000;
  PacketSinkHelper server ("ns3::UdpSocketFactory", InetSocketAddress(i.GetAddress(0), port));
  ApplicationContainer apps = server.Install (bsNodes.Get (0));
  Ptr<PacketSink> sink = DynamicCast<PacketSink> (apps.Get (0));
  sink -> TraceConnectWithoutContext ("Rx", MakeCallback(&ReceiveFromBs));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (runtime));


  uint32_t MaxPacketSize = 1024;
  uint32_t maxPacketCount = 320;
  UdpRelayHelper client (serverAddress, port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
  apps = client.Install (ataNodes);
  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (runtime));

  PeriodicSenderHelper appHelper = PeriodicSenderHelper ();
  appHelper.SetPeriod (Seconds (30.0));
  apps = appHelper.Install (endDevices);

  apps.Start (Seconds (2));
  apps.Stop (Seconds (runtime));


  /*******************************
  * Set relay callback           *
  *******************************/
  for (NodeContainer::Iterator j = ataNodes.Begin (); j != ataNodes.End (); ++j)
  {
    Ptr<Node> source = *j;
    Ptr<NetDevice> sourceNetDevice = source->GetDevice (0);
    Ptr<LoraNetDevice> sourceLoraNetDevice;
    if (sourceNetDevice->GetObject<LoraNetDevice> () != 0)
    {
      sourceLoraNetDevice = sourceNetDevice->GetObject<LoraNetDevice> ();
      sourceLoraNetDevice->SetReceiveCallback (MakeCallback(&ReceiveFromLora));
    } 
  }


  macHelper.SetSpreadingFactorsUp (endDevices, ataNodes, channel);

  /********************
  * RUN SIMULATION    *
  ********************/ 

  Simulator::Stop (Seconds (runtime));
  
  /*******************
  * Netanim config   *
  *******************/

  AnimationInterface anim ("uav-scenario.xml");
  anim.SetMobilityPollInterval (Seconds (5));
  anim.SetMaxPktsPerTraceFile (1000000000);

  uint32_t edIcon = anim.AddResource("/home/letal32/workspace/ns-allinone-3.27/ns-3.27/images/fireman.png");
  uint32_t uavIcon = anim.AddResource("/home/letal32/workspace/ns-allinone-3.27/ns-3.27/images/drone.png");
  uint32_t bsIcon = anim.AddResource("/home/letal32/workspace/ns-allinone-3.27/ns-3.27/images/bs.png");

  for (uint32_t i = 0; i < endDevices.GetN (); ++i)
  {
      anim.UpdateNodeDescription (endDevices.Get (i), "ED"); // Optional
      anim.UpdateNodeColor (endDevices.Get (i), 255, 0, 0); // Optional
      anim.UpdateNodeSize (endDevices.Get (i)-> GetId(), 50, 50);
      anim.UpdateNodeImage (endDevices.Get (i)-> GetId(), edIcon);
  }
  for (uint32_t i = 0; i < bsNodes.GetN (); ++i)
  {
      anim.UpdateNodeDescription (bsNodes.Get (i), "BS"); // Optional
      anim.UpdateNodeColor (bsNodes.Get (i), 0, 255, 0); // Optional
      anim.UpdateNodeSize (bsNodes.Get (i)-> GetId(), 50, 50);
      anim.UpdateNodeImage (bsNodes.Get (i)-> GetId(), bsIcon);
  }
  for (uint32_t i = 0; i < ataNodes.GetN (); ++i)
  {
      anim.UpdateNodeDescription (ataNodes.Get (i), "UAV"); // Optional
      anim.UpdateNodeColor (ataNodes.Get (i), 0, 0, 255); // Optional
      anim.UpdateNodeSize (ataNodes.Get (i) -> GetId(), 50, 50);
      anim.UpdateNodeImage (ataNodes.Get (i)-> GetId(), uavIcon); 
  }

  anim.SetBackgroundImage("background.jpg", 0, 0, 2, 2, 0.8);

  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}
