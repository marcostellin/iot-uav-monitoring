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
#include "ns3/virtual-springs-2d-mobility-model.h"
//#include "ns3/random-walk-2d-mobility-model.h"
//#include "ns3/hierarchical-mobility-model.h"
//#include "ns3/waypoint-mobility-model.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/firemen-mobility-model.h"
//#include "ns3/alfa-friis-loss-model.h"

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

#include "ns3/link-budget-estimator.h"

#include <algorithm>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include <utility>
#include <unordered_set>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("VsfExp3");

std::string filename;

std::string phyMode ("ErpOfdmRate12Mbps");
uint32_t numTeams = 3;
uint32_t numUavs = 10;
uint32_t numMembers = 20;
double runtime = 2000;

//NodeContainer endDevices;


/************************************
* Callbacks                         *
************************************/

// enum PacketOutcome {
//   RECEIVED,
//   INTERFERED,
//   NO_MORE_RECEIVERS,
//   UNDER_SENSITIVITY,
//   UNSET
// };

struct PacketStatus {
  Ptr<Packet const> packet;
  uint32_t senderId;
  bool recv;
  //std::vector<enum PacketOutcome> outcomes;
};

struct EdStatus
{
	uint32_t nSent = 0;
	uint32_t nRecv = 0;
	uint32_t nInterf = 0;
	uint32_t nNoRecvs = 0;
	uint32_t nUnderSens = 0;
};

std::map<Ptr<Packet const>, PacketStatus> packetTracker;
std::map<uint64_t, PacketStatus> packetRelayedTracker;
std::map<Ptr<Node>, EdStatus> edTracker;


std::vector<std::pair<int64_t,double>> connTracker;
Time lastInsert = Seconds (-1);
//uint32_t curUniqueCoveredNodes = 0;


std::unordered_set<int> curConn;

void
TransmissionCallback (Ptr<Packet const> packet, uint32_t systemId)
{
  // NS_LOG_INFO ("Transmitted a packet from device " << systemId);
  // Create a packetStatus
  // PacketStatus status;
  // status.packet = packet;
  // status.senderId = systemId;
  // status.outcomeNumber = 0;
  // status.outcomes = std::vector<enum PacketOutcome> (numUavs, UNSET);

  // packetTracker.insert (std::pair<Ptr<Packet const>, PacketStatus> (packet, status));
	PacketStatus pStatus;
	pStatus.packet = packet;
	pStatus.senderId = systemId;
	pStatus.recv = false;

	packetTracker.insert (std::pair<Ptr<Packet const>, PacketStatus> (packet, pStatus));

	std::map<Ptr<Node>, EdStatus>::iterator it = edTracker.find(NodeList::GetNode (systemId));

	if ( it == edTracker.end () )
	{
		EdStatus status;
		status.nSent += 1;

  	edTracker.insert (std::pair<Ptr<Node>, EdStatus> (NodeList::GetNode (systemId), status ));

  	return;
	}

  it -> second.nSent += 1;	

}

void
PacketReceptionCallback (Ptr<Packet const> packet, uint32_t systemId)
{
  // Remove the successfully received packet from the list of sent ones
  // NS_LOG_INFO ("A packet was successfully received at gateway " << systemId);

  // std::map<Ptr<Packet const>, PacketStatus>::iterator it = packetTracker.find (packet);
  // (*it).second.outcomes.at (systemId - numEds) = RECEIVED;
  // (*it).second.outcomeNumber += 1;
  std::map<Ptr<Packet const>, PacketStatus>::iterator itPacket = packetTracker.find (packet);

  if (!(*itPacket).second.recv)
  {
  	std::map<Ptr<Node>, EdStatus>::iterator it = edTracker.find(NodeList::GetNode ( (*itPacket).second.senderId) );
  	(*it).second.nRecv += 1;
  	(*itPacket).second.recv = true;
  }


}

void
InterferenceCallback (Ptr<Packet const> packet, uint32_t systemId)
{
  // NS_LOG_INFO ("A packet was lost because of interference at gateway " << systemId);

  // std::map<Ptr<Packet const>, PacketStatus>::iterator it = packetTracker.find (packet);
  // (*it).second.outcomes.at (systemId - numEds) = INTERFERED;
  // (*it).second.outcomeNumber += 1;
  
  std::map<Ptr<Packet const>, PacketStatus>::iterator itPacket = packetTracker.find (packet);
  
  std::map<Ptr<Node>, EdStatus>::iterator it = edTracker.find(NodeList::GetNode ( (*itPacket).second.senderId) );
  (*it).second.nInterf += 1;

}

void
NoMoreReceiversCallback (Ptr<Packet const> packet, uint32_t systemId)
{
  // NS_LOG_INFO ("A packet was lost because there were no more receivers at gateway " << systemId);

  // std::map<Ptr<Packet const>, PacketStatus>::iterator it = packetTracker.find (packet);
  // (*it).second.outcomes.at (systemId - numEds) = NO_MORE_RECEIVERS;
  // (*it).second.outcomeNumber += 1;

  std::map<Ptr<Packet const>, PacketStatus>::iterator itPacket = packetTracker.find (packet);

  std::map<Ptr<Node>, EdStatus>::iterator it = edTracker.find(NodeList::GetNode ( (*itPacket).second.senderId) );
  (*it).second.nNoRecvs += 1;

}

void
UnderSensitivityCallback (Ptr<Packet const> packet, uint32_t systemId)
{
  // NS_LOG_INFO ("A packet arrived at the gateway under sensitivity at gateway " << systemId);

  // std::map<Ptr<Packet const>, PacketStatus>::iterator it = packetTracker.find (packet);
  // (*it).second.outcomes.at (systemId - numEds) = UNDER_SENSITIVITY;
  // (*it).second.outcomeNumber += 1;

  std::map<Ptr<Packet const>, PacketStatus>::iterator itPacket = packetTracker.find (packet);

  std::map<Ptr<Node>, EdStatus>::iterator it = edTracker.find(NodeList::GetNode ( (*itPacket).second.senderId) );
  (*it).second.nUnderSens += 1;
 
}

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

  relay -> Send(relayedPkt);

  return true;
}

void
ReceiveFromBs(Ptr<const Packet> packet, const Address &addr)
{
  //NS_LOG_UNCOND("TestRx");
  std::map<uint64_t, PacketStatus>::iterator it = packetRelayedTracker.find (packet -> GetUid ());
  (*it).second.recv = true;
}

void
ChangedConnectivity (const std::vector<int> &coveredList)
{
	Time curTime = Simulator::Now ();

	if (curTime > lastInsert)
	{
		std::pair <int64_t,double> measurement = std::make_pair (lastInsert.GetMilliSeconds (), curConn.size ());
		connTracker.push_back (measurement);

		curConn.clear ();
		lastInsert = curTime;
	}

	for (uint16_t i = 0; i < coveredList.size (); i++)
	{
		curConn.insert (coveredList[i]);
	}

}

/********************
/ Utility functions *
********************/

void
WriteNodeStats () 
{
	std::ofstream file;
	file.open ("node-stats.out.txt");
	file << "/* Unique packets received by the BS */" << std::endl;
	file << "id,nSent,nRecv,nInterf,nNoRecvs,nUnderSens,PDR, avgDelayToGw, avgDelayToBs" << std::endl;

  for (std::map<Ptr<Node>, EdStatus>::iterator it=edTracker.begin(); it!=edTracker.end(); ++it)
  {
    file << it -> first -> GetId () << "," << 
    						 it -> second.nSent  << "," <<  
    						 it -> second.nRecv << "," <<
    						 it -> second.nInterf << "," <<
    						 it -> second.nNoRecvs << "," <<
    						 it -> second.nUnderSens << std::endl;
  }

  file.close ();
}

void
WriteMeshPdr () 
{
	std::ofstream file;
	file.open ("mesh-pdr-stats.out.txt");
	file << "sent,received,notReceived" << std::endl;

	uint16_t nBsRecv = 0;
  uint16_t nBsNotRecv = 0;
  for (std::map<uint64_t, PacketStatus>::iterator it=packetRelayedTracker.begin(); it!=packetRelayedTracker.end(); ++it)
  {
    if (it -> second.recv)
      nBsRecv++;
    else
      nBsNotRecv++;
  }

  file << nBsRecv + nBsNotRecv << "," << nBsRecv << "," << nBsNotRecv << std::endl;

  file.close();
}

void
WriteConnectivityStats ()
{
	std::ofstream file;
	file.open ("connectivity-stats.out.txt");

	for (uint16_t i = 0; i < connTracker.size (); i++)
	{
		std::pair<int64_t, double> pair = connTracker[i];
		file << pair.first << "," << pair.second / (numMembers*numTeams) << std::endl;
	}

	file.close ();
}


int
main (int argc, char* argv[]){

  /*****************************
  * LOGGING                    *
  *****************************/
	//LogComponentEnable("UdpRelay", LOG_LEVEL_INFO);
	//LogComponentEnable("UdpServer", LOG_LEVEL_INFO);
  //LogComponentEnable ("FiremenMobilityModel", LOG_LEVEL_DEBUG);
  //LogComponentEnable ("MobilityHelper", LOG_LEVEL_DEBUG);
  //LogComponentEnable ("EndDeviceLoraPhy", LOG_LEVEL_INFO);
  //LogComponentEnable ("GatewayLoraPhy", LOG_LEVEL_INFO);
  //LogComponentEnable ("GatewayLoraMac", LOG_LEVEL_INFO);
  //LogComponentEnable ("VirtualSprings2d", LOG_LEVEL_ALL);
  LogComponentEnable ("VirtualSprings2d", LOG_LOGIC);
  //LogComponentEnable ("VirtualSprings2d", LOG_INFO);
  //LogComponentEnable ("VirtualSprings2d", LOG_DEBUG);
  //LogComponentEnable ("LinkBudgetEstimator", LOG_DEBUG);
  //LogComponentEnable ("VsfExp3", LOG_LEVEL_ALL);

	LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnableAll (LOG_PREFIX_TIME);

  RngSeedManager::SetSeed (3);  // Changes seed from default of 1 to 3
  RngSeedManager::SetRun (58);  //Best seed 56 //58 is difficult!
  
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
  Ptr<UniformDiscPositionAllocator> edsAllocator = CreateObject<UniformDiscPositionAllocator> ();
  edsAllocator->SetRho(10);
  edsAllocator->SetX(250.0);
  edsAllocator->SetY(250.0);

  Rectangle area = Rectangle (0, 2000, 0, 2000);

  mobility.SetPositionAllocator (edsAllocator);
  mobility.SetMobilityModel ("ns3::FiremenMobilityModel",
                             "PropagationArea", RectangleValue(area),
                             "SpreadY", DoubleValue(100),
                             "NumTeams", IntegerValue(numTeams),
                             "TeamId", IntegerValue (0));
  mobility.Install (team0);

  // Assign a mobility model to team1
  edsAllocator->SetRho(10);
  edsAllocator->SetX(200.0);
  edsAllocator->SetY(250.0);

  mobility.SetPositionAllocator (edsAllocator);
  mobility.SetMobilityModel ("ns3::FiremenMobilityModel",
                             "PropagationArea", RectangleValue(area),
                             "SpreadY", DoubleValue(100),
                             "NumTeams", IntegerValue(numTeams),
                             "TeamId", IntegerValue (1));
  mobility.Install (team1);

  // Assign a mobility model to team2
  edsAllocator->SetRho(10);
  edsAllocator->SetX(150.0);
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

  Vector bsPos = Vector(200,200,0);
  
  //Assign mobility model to nodes

  Ptr<UniformDiscPositionAllocator> nodesAllocator = CreateObject<UniformDiscPositionAllocator> ();
  nodesAllocator -> SetRho (50);
  nodesAllocator -> SetX (bsPos.x);
  nodesAllocator -> SetY (bsPos.y);

  mobility.SetPositionAllocator (nodesAllocator);
  mobility.SetMobilityModel ("ns3::VirtualSprings2dMobilityModel", 
  																	"Time", TimeValue(Seconds(10)),
  																	"Tolerance", DoubleValue(5),
                                    "Speed", DoubleValue (1.5),
                                    "BsPosition", VectorValue(bsPos),
                                    //"TxRangeAta", DoubleValue(500),
                                    //"TxRangeAtg", DoubleValue (150),
                                    //"l0Atg", DoubleValue (10),
                                    //"l0Ata", DoubleValue(350),
                                    "LinkBudgetAta", DoubleValue (20),  //20
                                    "LinkBudgetAtg", DoubleValue (15),
                                    "kAta", DoubleValue (3)  //5
                                    //"KatgPlusMode", BooleanValue (false)
                                    );
  mobility.Install (ataNodes);
  
  // Setup Nodes in VSF mobility model
  for (NodeContainer::Iterator j = ataNodes.Begin (); j != ataNodes.End (); ++j)
  {
    Ptr<Node> node = *j;
    Ptr<VirtualSprings2dMobilityModel> model = node -> GetObject<VirtualSprings2dMobilityModel>();

    for (NodeContainer::Iterator i = ataNodes.Begin (); i != ataNodes.End (); ++i)
    {
      Ptr<Node> nodeToAdd = *i;
      if (nodeToAdd -> GetId() != node->GetId()){
        model -> AddAtaNode(nodeToAdd->GetId());
      }
    }

    for (NodeContainer::Iterator i = endDevices.Begin (); i != endDevices.End (); ++i)
    {
      Ptr<Node> nodeToAdd = *i;
      model -> AddAtgNode(nodeToAdd -> GetId());
    }
  }

  //Setup Callbacks of VSF
  for (NodeContainer::Iterator j = ataNodes.Begin (); j != ataNodes.End (); ++j)
  {
  	Ptr<Node> node = *j;
    Ptr<VirtualSprings2dMobilityModel> model = node -> GetObject<VirtualSprings2dMobilityModel>();

    model -> TraceConnectWithoutContext ("NodesInRange", MakeCallback (&ChangedConnectivity) );
  }

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

  // Setup Routing in VSF mobility model
  for (NodeContainer::Iterator j = ataNodes.Begin (); j != ataNodes.End (); ++j)
  {
    Ptr<Node> node = *j;
    Ptr<VirtualSprings2dMobilityModel> model = node -> GetObject<VirtualSprings2dMobilityModel>();
    Ptr<olsr::RoutingProtocol> routing = node -> GetObject<olsr::RoutingProtocol> ();

    model -> SetOlsrRouting (routing);
    
  }

  /*************************
  * Install Internet Stack *
  *************************/
  
  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses to WiFi nodes...");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (wifiDevices);


  /******************************
  * Set Link Budget Estimator   *
  ******************************/

  Ptr<LinkBudgetEstimator> est = CreateObject<LinkBudgetEstimator> ();
  est -> SetAttribute ("TxPowerAtg", DoubleValue (14.0));
  est -> SetAttribute ("TxPowerAta", DoubleValue (16.0206));
  est -> SetAttribute ("SensitivityAta", DoubleValue (-96));
  est -> SetAttribute ("SensitivityAtg", DoubleValue (-130));
  est -> SetAtgModel (medLoss);
  est -> SetAtaModel (friisLoss);

  for (uint16_t i = 0; i < ataNodes.GetN (); i++)
  {
  	Ptr<Node> node = ataNodes.Get(i);
  	Ptr<VirtualSprings2dMobilityModel> mob = node -> GetObject<VirtualSprings2dMobilityModel> ();
  	mob -> SetLinkBudgetEstimator (est);
  }


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
  appHelper.SetPeriod (Seconds (10.0));
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

  /******************************
  * Set LoRa callbacks          *
  ******************************/
  
  for (NodeContainer::Iterator j = endDevices.Begin (); j != endDevices.End (); ++j)
  {
    Ptr<Node> node = *j;
    Ptr<LoraNetDevice> loraNetDevice = node->GetDevice (0)->GetObject<LoraNetDevice> ();
    Ptr<LoraPhy> phy = loraNetDevice->GetPhy ();
    phy->TraceConnectWithoutContext ("StartSending",
                                       MakeCallback (&TransmissionCallback));
  }

  for (NodeContainer::Iterator j = ataNodes.Begin (); j != ataNodes.End (); j++)
  {

    Ptr<Node> object = *j;

    Ptr<NetDevice> netDevice = object->GetDevice (0);
    Ptr<LoraNetDevice> loraNetDevice = netDevice->GetObject<LoraNetDevice> ();
    NS_ASSERT (loraNetDevice != 0);
    Ptr<GatewayLoraPhy> gwPhy = loraNetDevice->GetPhy ()->GetObject<GatewayLoraPhy> ();

    // Global callbacks (every gateway)
    gwPhy->TraceConnectWithoutContext ("ReceivedPacket",
                                         MakeCallback (&PacketReceptionCallback));
    gwPhy->TraceConnectWithoutContext ("LostPacketBecauseInterference",
                                         MakeCallback (&InterferenceCallback));
    gwPhy->TraceConnectWithoutContext ("LostPacketBecauseNoMoreReceivers",
                                         MakeCallback (&NoMoreReceiversCallback));
    gwPhy->TraceConnectWithoutContext ("LostPacketBecauseUnderSensitivity",
                                         MakeCallback (&UnderSensitivityCallback));
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

  /**********************
  * Print Stats         *
  **********************/

  WriteNodeStats ();

  WriteMeshPdr ();

  WriteConnectivityStats ();

  return 0;
}
