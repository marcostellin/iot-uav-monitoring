/*
This experiment will evaluate the coverage of a gateway in a forest scenario.
For each distance evaluate throughput, delay, PDR
*/

#include "ns3/end-device-lora-phy.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/end-device-lora-mac.h"
#include "ns3/gateway-lora-mac.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/node-list.h"
#include "ns3/core-module.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/position-allocator.h"
#include "ns3/one-shot-sender-helper.h"
#include "ns3/command-line.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/alfa-friis-loss-model.h"
#include "ns3/propagation-loss-model.h"
#include <algorithm>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Experiment3");

double l_frequency = 867e+06;
double interval;
double nNodes = 10;
double nGateways = 1;
int radius = 1000; // Max radius of a SF7
int sf = 5; // data rate goes from 0 to 5
double nRcv = 0;
double nTx = 0;
double nRcvNoDup = 0;
double nUnderSens = 0;
double nInterf = 0;
double nNoRecv = 0;
std::string filename;


/**************************************
* GLOBAL CALLBACKS                    *
**************************************/

enum PacketOutcome {
  RECEIVED,
  INTERFERED,
  NO_MORE_RECEIVERS,
  UNDER_SENSITIVITY,
  UNSET
};

struct PacketStatus {
  Ptr<Packet const> packet;
  uint32_t senderId;
  int outcomeNumber;
  std::vector<enum PacketOutcome> outcomes;
  int64_t txTime;
  Vector senderPos;
};

std::map<Ptr<Packet const>, PacketStatus> packetTracker;

void
TransmissionCallback (Ptr<Packet const> packet, uint32_t systemId)
{
  nTx += 1;
  PacketStatus status;
  status.packet = packet;
  status.senderId = systemId;
  status.outcomeNumber = 0;
  status.outcomes = std::vector<enum PacketOutcome> (nGateways, UNSET);

  packetTracker.insert (std::pair<Ptr<Packet const>, PacketStatus> (packet, status));
}

void
PacketReceptionCallback (Ptr<Packet const> packet, uint32_t systemId)
{
  nRcv += 1;
  std::map<Ptr<Packet const>, PacketStatus>::iterator it = packetTracker.find (packet);
  (*it).second.outcomes.at (systemId - nNodes) = RECEIVED;
  (*it).second.outcomeNumber += 1;
  if ((*it).second.outcomeNumber == 1){
    nRcvNoDup++;
  }
}

void
InterferenceCallback (Ptr<Packet const> packet, uint32_t systemId)
{
  nInterf += 1;
} 

void
NoMoreReceiversCallback (Ptr<Packet const> packet, uint32_t systemId)
{
  nNoRecv += 1;
} 

void
UnderSensitivityCallback (Ptr<Packet const> packet, uint32_t systemId)
{
  nUnderSens += 1;
}

    
bool fexists(const std::string& filename) {
  std::ifstream ifile(filename.c_str());
  return (bool)ifile;
}


int
main (int argc, char* argv[]){


	  CommandLine cmd;
    cmd.AddValue ("DataRate", "Rate of Lora devices (0 to 5)", sf);
    cmd.AddValue ("nNodes", "Number of LoRa nodes", nNodes);
    cmd.AddValue ("Interval", "Space between packets", interval );
    cmd.AddValue ("nGateways", "Number of gateways", nGateways);

    cmd.Parse (argc, argv);

	/*****************************
	* LOGGING                    *
	*****************************/
	// LogComponentEnable ("Experiment1", LOG_LEVEL_INFO);
 //    LogComponentEnable ("LoraChannel", LOG_LEVEL_DEBUG);
       //LogComponentEnable ("EndDeviceLoraPhy", LOG_LEVEL_INFO);
       //LogComponentEnable ("GatewayLoraPhy", LOG_LEVEL_INFO);
 //    LogComponentEnable ("GatewayLoraMac", LOG_LEVEL_INFO);
 //    LogComponentEnable ("WeissbergerPropagationLossModel", LOG_LEVEL_DEBUG);
 //    LogComponentEnable ("PropagationLossModel", LOG_LEVEL_DEBUG);

	  LogComponentEnableAll (LOG_PREFIX_FUNC);
  	LogComponentEnableAll (LOG_PREFIX_NODE);
  	LogComponentEnableAll (LOG_PREFIX_TIME);

  	filename = "exp3.rate" + std::to_string(sf) + ".out.txt";
    bool isFileExistent = fexists(filename);
  	std::ofstream out(filename, std::ios_base::app);
    std::streambuf *coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(out.rdbuf());

  	/************************
    *  Create the channel  *
    ************************/

    NS_LOG_INFO ("Creating the channel...");

    // Create the lora channel object. The Lora channel has an alfa Friis propagation and a constant delay (default to speed of light)

    Ptr<AlfaFriisPropagationLossModel> medLoss = CreateObject<AlfaFriisPropagationLossModel> ();

    medLoss->SetFrequency(l_frequency);
    medLoss->SetAlfa(3.29);

    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();

    Ptr<LoraChannel> channel = CreateObject<LoraChannel> (medLoss, delay);

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

    /************************
    *  Create End Devices  *
    ************************/

    NS_LOG_INFO ("Creating end devices...");

    // Create a set of nodes
    NodeContainer endDevices;
    endDevices.Create (nNodes);

    // Assign a mobility model to the node
    Ptr<UniformDiscPositionAllocator> nodesAllocator = CreateObject<UniformDiscPositionAllocator> ();
    nodesAllocator->SetRho(radius);

    mobility.SetPositionAllocator (nodesAllocator);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (endDevices);

    // Create the LoraNetDevices of the end devices
    phyHelper.SetDeviceType (LoraPhyHelper::ED);
    macHelper.SetDeviceType (LoraMacHelper::ED);
    helper.Install (phyHelper, macHelper, endDevices);

    for (NodeContainer::Iterator j = endDevices.Begin (); j != endDevices.End (); ++j)
    {
      Ptr<Node> node = *j;

      Ptr<LoraNetDevice> loraNetDevice = node->GetDevice (0)->GetObject<LoraNetDevice> ();
      Ptr<EndDeviceLoraMac> mac = loraNetDevice->GetMac ()->GetObject<EndDeviceLoraMac> ();
      mac -> SetDataRate(sf);
    }

    /*********************
    *  Create Gateways  *
    *********************/

    NS_LOG_INFO ("Creating the gateway...");
    NodeContainer gateways;
    gateways.Create (nGateways);

    Ptr<UniformDiscPositionAllocator> gwDiscAllocator = CreateObject<UniformDiscPositionAllocator> ();
    gwDiscAllocator->SetRho(radius);
  
    Ptr<ListPositionAllocator> gwListAllocator = CreateObject<ListPositionAllocator> ();
    gwListAllocator->Add(Vector(0,0,60));
  
    mobility.SetPositionAllocator (gwDiscAllocator);
    mobility.Install (gateways);

    mobility.SetPositionAllocator (gwListAllocator);
    mobility.Install (gateways.Get(0));

    for (NodeContainer::Iterator j = gateways.Begin (); j != gateways.End (); ++j)
    {
      Ptr<MobilityModel> mobility = (*j)->GetObject<MobilityModel> ();
      Vector position = mobility->GetPosition ();
      position.z = 60;
      mobility->SetPosition (position);
    }

    // Create a netdevice for each gateway
    phyHelper.SetDeviceType (LoraPhyHelper::GW);
    macHelper.SetDeviceType (LoraMacHelper::GW);
    helper.Install (phyHelper, macHelper, gateways);

    // Set the data rates for different spreading factors
    //macHelper.SetSpreadingFactorsUp (endDevices, gateways, channel);

    /*******************************
    * Setup Trace sinks            *
    *******************************/

    
    for (NodeContainer::Iterator j = endDevices.Begin (); j != endDevices.End (); ++j)
    {
      Ptr<Node> node = *j;
      Ptr<LoraNetDevice> loraNetDevice = node->GetDevice (0)->GetObject<LoraNetDevice> ();
      Ptr<LoraPhy> phy = loraNetDevice->GetPhy ();
      phy->TraceConnectWithoutContext ("StartSending",
                                       MakeCallback (&TransmissionCallback));
    }

    for (NodeContainer::Iterator j = gateways.Begin (); j != gateways.End (); j++)
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

    

    /********************
    * Install Application*
    ********************/

    PeriodicSenderHelper appHelper = PeriodicSenderHelper ();
    appHelper.SetPeriod (Seconds (interval));
    ApplicationContainer appContainer = appHelper.Install (endDevices);

    appContainer.Start (Seconds (2));
    appContainer.Stop (Minutes (10)); 

    Simulator::Stop (Hours (2));

    Simulator::Run ();

    Simulator::Destroy ();
  
    if (!isFileExistent){    
      //std::cout << "SF\tnNodes\nGateways\ttInterval\tPDR\tnUnderSens\tnInterf\tnNoRecv" << std::endl;
      std::cout << "SF,nNodes,Gateways,Interval,PDR,nUnderSens,nInterf,nNoRecv" << std::endl;
    }

    std::cout << sf << "," << nNodes << "," << nGateways << "," << interval << "," << nRcvNoDup/nTx << "," << nUnderSens << "," << nInterf << "," << nNoRecv << std::endl;

    //std::cout << sf << "\t" << nNodes << "\t" << nGateways << "\t" << interval << "\t" << nRcvNoDup/nTx << "\t" << nRcvNoDup << "\t" << nRcv << std::endl;
    std::cout.rdbuf(coutbuf);

    return 0;
}

