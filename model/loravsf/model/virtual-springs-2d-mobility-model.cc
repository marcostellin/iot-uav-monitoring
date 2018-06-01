/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
 *
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
 * Author: Marco Stellin
 */
#include "virtual-springs-2d-mobility-model.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/node-list.h"
#include "ns3/boolean.h"
#include "ns3/olsr-routing-protocol.h"
#include "ns3/lora-net-device.h"
#include "ns3/gateway-lora-phy.h"
#include <cmath>
#include <algorithm>
#include <queue>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("VirtualSprings2d");

NS_OBJECT_ENSURE_REGISTERED (VirtualSprings2dMobilityModel);

TypeId
VirtualSprings2dMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::VirtualSprings2dMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<VirtualSprings2dMobilityModel> ()

    .AddAttribute ("Time",
                   "Change current direction and speed after moving for this delay.",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&VirtualSprings2dMobilityModel::m_modeTime),
                   MakeTimeChecker ())

    .AddAttribute ("Speed",
                   "The speed of the aerial nodes",
                   DoubleValue (17.0),
                   MakeDoubleAccessor (&VirtualSprings2dMobilityModel::m_speed),
                   MakeDoubleChecker<double> ())

    .AddAttribute ("Tolerance",
                   "Minimum force to cause displacement",
                   DoubleValue (30.0),
                   MakeDoubleAccessor (&VirtualSprings2dMobilityModel::m_tol),
                   MakeDoubleChecker<double> ())
                                 
    .AddAttribute ("kAta",
                   "Stifness of AtA springs",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&VirtualSprings2dMobilityModel::m_kAta),
                   MakeDoubleChecker<double> ())

    // .AddAttribute ("l0Ata",
    //                "Natural length of AtA springs",
    //                DoubleValue (150),
    //                MakeDoubleAccessor (&VirtualSprings2dMobilityModel::m_l0Ata),
    //                MakeDoubleChecker<double> ())

    // .AddAttribute ("l0Atg",
    //                "Natural length of AtG springs",
    //                DoubleValue (20),
    //                MakeDoubleAccessor (&VirtualSprings2dMobilityModel::m_l0Atg),
    //                MakeDoubleChecker<double> ())

    .AddAttribute ("LinkBudgetAta",
                   "Required link budget for AtA links",
                   DoubleValue (10.0),
                   MakeDoubleAccessor (&VirtualSprings2dMobilityModel::m_lbReqAta),
                   MakeDoubleChecker<double> ())

    .AddAttribute ("LinkBudgetAtg",
                   "Required link budget for AtG links",
                   DoubleValue (10.0),
                   MakeDoubleAccessor (&VirtualSprings2dMobilityModel::m_lbReqAtg),
                   MakeDoubleChecker<double> ())

    .AddAttribute ("BsPosition", "Position of BS",
                    VectorValue (Vector (200.0, 200.0, 0.0)), 
                    MakeVectorAccessor (&VirtualSprings2dMobilityModel::m_bsPos),
                    MakeVectorChecker ())

    .AddAttribute ("BsAddress", "IP Address of the base station",
                    Ipv4AddressValue("10.1.1.1"),
                    MakeIpv4AddressAccessor (&VirtualSprings2dMobilityModel::m_bsAddr),
                    MakeIpv4AddressChecker ())

    .AddAttribute ("KatgPlusMode", "K_ATG boost option",
                    BooleanValue (true), // ignored initial value.
                    MakeBooleanAccessor (&VirtualSprings2dMobilityModel::m_kAtgPlusMode),
                    MakeBooleanChecker ())

    .AddTraceSource ("NodesInRange",
                     "Nodes in range have changed",
                      MakeTraceSourceAccessor (&VirtualSprings2dMobilityModel::m_nodesInRangeTrace),
                     "ns3::VirtualSprings2dMobilityModel::TracedCallback");         
                   
                   
  return tid;
}

void
VirtualSprings2dMobilityModel::DoInitialize (void)
{
  //Initialize private variables
  m_pause = 0;
  m_hops = 0;
  m_persist = 0;

  //Initialize LoraMonitor
  Ptr<VirtualSprings2dMobilityModel> mob = Ptr<VirtualSprings2dMobilityModel> (this);
  Ptr<Node> node = mob -> GetObject<Node> ();

  Ptr<NetDevice> netDevice = node -> GetDevice (0);
  Ptr<LoraNetDevice> loraNetDevice = netDevice -> GetObject<LoraNetDevice> ();
  NS_ASSERT (loraNetDevice != 0);
  Ptr<GatewayLoraPhy> gwPhy = loraNetDevice->GetPhy ()->GetObject<GatewayLoraPhy> ();

  m_monitor = Ptr<LoraEdsMonitor>( new LoraEdsMonitor (gwPhy));

  //Set Node Id
  m_id = node -> GetId ();


  DoInitializePrivate ();
  MobilityModel::DoInitialize ();
}

void
VirtualSprings2dMobilityModel::DoInitializePrivate (void)
{
  m_helper.Update();
  double speed = m_speed;

  //Update neighbours list
  VirtualSprings2dMobilityModel::SetNeighboursList ();

  //Update Eds List
  VirtualSprings2dMobilityModel::SetEdsList ();

  // for (std::map<uint32_t, EdsEntry>::iterator it = m_eds.begin (); it != m_eds.end (); ++it)
  // {
  //   NS_LOG_LOGIC ( it-> first << " " << it -> second.time );
  // }

  //Compute the direction of the movement
  Vector forceAta = VirtualSprings2dMobilityModel::ComputeAtaForce ();
  Vector forceAtg = VirtualSprings2dMobilityModel::ComputeAtgForce ();
  
  Vector force = forceAta + forceAtg;

  NS_LOG_DEBUG ("AtG Force = " << forceAtg << ", AtA force = " << forceAta);

  double k = speed/std::sqrt(force.x*force.x + force.y*force.y);
  Vector vector (k*force.x,
  				       k*force.y,
  				       0.0);

  //Move only if force is bigger than a threshold to avoid oscillations
  if (force.GetLength () > m_tol)
  {
    //Compute the future position based on current velocity and direction
    Vector myPos = m_helper.GetCurrentPosition();
    Vector nextPos;
    nextPos.x = myPos.x + vector.x * m_modeTime.GetSeconds ();
    nextPos.y = myPos.y + vector.y * m_modeTime.GetSeconds ();

    olsr::RoutingTableEntry entry = VirtualSprings2dMobilityModel::HasPathToBs();
    
    //If a path to the BS exists
    if (entry.distance != 0) 
    {
      m_hops = entry.distance;
      double rangeApprox = GetDistanceFromBs () / GetDistanceFromFurthestNeighbour ();
      m_persist = 3*(entry.distance - 1) * (uint32_t)rangeApprox ;

      //Move according to forces if m_pause intervals have passed...
      if (m_pause == 0) 
      {
        m_helper.SetVelocity(vector);
        m_helper.Unpause ();
      }

      //Or don't do anything if m_pause intervals still have to pass.
      if (m_pause > 0)
      {
        m_pause --;
        m_helper.Pause ();
      }

    }

    if (entry.distance  == 0)
    {
      //Go back and try to recover connectivity...
      if (m_persist == 0)
      {
        NS_LOG_INFO ("Not in range of BS. Going Back!");
        Vector curPos = m_helper.GetCurrentPosition ();
        Vector diff = m_bsPos - curPos;
        double h = speed*2/std::sqrt(diff.x*diff.x + diff.y*diff.y);
        Vector vec (h*diff.x,
                    h*diff.y,
                    0.0);

        NS_LOG_INFO ("m_hops = " << m_hops);
        m_pause = VirtualSprings2dMobilityModel::SetPause(m_hops);

        m_helper.SetVelocity(vec);
        m_helper.Unpause ();
      }

      //Try to go on and see if connectivity is recovered after m_persist intervals...
      if (m_persist > 0)
      {
        NS_LOG_INFO ("Persit on route for " << m_persist << " intervals");

        if (!m_eds.empty ())
        {
          uint32_t simNode = FindMostSimilarNode ();

          if (m_id > simNode)
            m_persist = m_persist / 2;
        }
        else
        {
          m_persist --;
        }
        
        m_helper.SetVelocity(vector);
        m_helper.Unpause ();
      }  

    }
 
  }
  else //Stop if force is too small
  {
    m_helper.Pause();
  }

  Time delayLeft = m_modeTime;

  DoWalk (delayLeft);
  
}

olsr::RoutingTableEntry
VirtualSprings2dMobilityModel::HasPathToBs ()
{
  std::vector<ns3::olsr::RoutingTableEntry> entries = m_routing -> ns3::olsr::RoutingProtocol::GetRoutingTableEntries ();

  for (uint16_t i = 0; i < entries.size(); i++)
  {
    if (entries[i].destAddr.IsEqual (m_bsAddr))
    {
      return entries[i];
    }
  }

  return olsr::RoutingTableEntry ();

}

uint32_t
VirtualSprings2dMobilityModel::SetPause (uint32_t hops)
{
  if (hops > 0)
  {
    double pause = 100.0 * std::exp (-m_modeTime.GetSeconds () / 10.0) /(std::pow(hops, 3)) + 1;
    NS_LOG_INFO ("Stop for " << (uint32_t)pause << " intervals");
    return (uint32_t)pause;
  }

  return 0;
}

void
VirtualSprings2dMobilityModel::SetNeighboursList ()
{
  std::vector<ns3::olsr::RoutingTableEntry> entries = m_routing -> ns3::olsr::RoutingProtocol::GetRoutingTableEntries ();

  m_neighbours.clear ();

  for (uint16_t i = 0; i < entries.size(); i++)
  {
    if (entries[i].distance == 1)
    {
      for (uint16_t j = 0; j < m_ataNodes.size (); j++)
      {
        Ptr<Node> node = NodeList::GetNode(m_ataNodes[j]);
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); 
        Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal (); 

        if (addr.IsEqual (entries[i].destAddr))
        {
          m_neighbours.push_back (node -> GetId());
        }

      }
    }
  }
}

void
VirtualSprings2dMobilityModel::SetEdsList ()
{
  std::map<uint32_t, EdsEntry> curNodes = m_monitor -> GetEdsList (Seconds (20));

  m_eds = curNodes;
}



Vector
VirtualSprings2dMobilityModel::ComputeAtaForce()
{
  Vector myPos = m_helper.GetCurrentPosition ();

  double fx = 0;
  double fy = 0;
  
  for (uint16_t j = 0; j < m_neighbours.size(); j++)
  {
    Ptr<Node> node = NodeList::GetNode(m_neighbours[j]);
    Ptr<MobilityModel> otherMob = node -> GetObject<MobilityModel>();
    Vector otherPos = otherMob -> GetPosition();
    //double dist = CalculateDistance(myPos, otherPos);

    double lb = m_estimator -> GetAtaLinkBudget ( Ptr<MobilityModel> (this), otherMob);

    double k_ata = m_kAta;

    //double disp = dist - m_l0Ata;
    double disp = m_lbReqAta - lb;
    Vector diff = otherPos - myPos;
    //double force = m_kAta*disp;
    double force = k_ata * disp;
    double k = force/std::sqrt(diff.x*diff.x + diff.y*diff.y);

    fx += k*diff.x;
    fy += k*diff.y;
  }

  return Vector(fx,fy,0);
}

Vector
VirtualSprings2dMobilityModel::ComputeAtgForce()
{
  Vector myPos = m_helper.GetCurrentPosition ();
  myPos.z = 0;

  double kAtg = VirtualSprings2dMobilityModel::ComputeKatg();

  double fx = 0;
  double fy = 0;

  for (std::map<uint32_t, EdsEntry>::iterator it = m_eds.begin (); it != m_eds.end (); ++it)
  {

    Vector pos ( (*it).second.x,
                 (*it).second.y,
                 0
               );

    Ptr<ConstantPositionMobilityModel> mob = CreateObject<ConstantPositionMobilityModel> ();
    mob -> SetPosition (pos);
    double lb = m_estimator -> GetAtgLinkBudget ( Ptr<MobilityModel> (this), mob);

    //double dist = CalculateDistance(myPos, pos);

    double kAtgPlus = 1;
    
    if (m_kAtgPlusMode)
    {
      uint16_t cnt = 1;

      for (uint16_t i = 0; i < m_neighbours.size (); i++)
      {
        Ptr<Node> ataNode = NodeList::GetNode(m_neighbours[i]);
        Ptr<VirtualSprings2dMobilityModel> ataMob = ataNode -> GetObject<VirtualSprings2dMobilityModel> ();
        std::map<uint32_t, EdsEntry> otherList = ataMob -> GetEdsList ();

        //otherList.insert (m_eds.begin (), m_eds.end ());

        std::map<uint32_t, EdsEntry>::iterator oIt;
        oIt = otherList.find ( it -> first );

        if ( oIt != otherList.end () )
        {
          cnt ++;
        }
      }

      cnt > 1 ? kAtgPlus = 1 : kAtgPlus = 20;
    }
    
    double disp = m_lbReqAtg - lb;
    Vector diff = pos - myPos;
    double force = (kAtg * kAtgPlus) * disp;
    double k = force/std::sqrt(diff.x*diff.x + diff.y*diff.y);

    fx += k*diff.x;
    fy += k*diff.y;
  }

   return Vector(fx,fy,0);

}

int
VirtualSprings2dMobilityModel::GetMaxNodesNeighbours()
{
  uint32_t curMax = 0;

  for (uint16_t j = 0; j < m_neighbours.size(); j++)
  {
    Ptr<Node> ataNode = NodeList::GetNode(m_neighbours[j]);
    Ptr<VirtualSprings2dMobilityModel> mob = ataNode -> GetObject<VirtualSprings2dMobilityModel> ();

    uint32_t numAtgNodes = mob -> GetCoveredEds ();

    if (numAtgNodes > curMax){
      curMax = numAtgNodes;
    }
  }

  return curMax;
}

double
VirtualSprings2dMobilityModel::ComputeKatg()
{
  double coveredNodes = m_eds.size ();
  double maxNodesNeighbours = VirtualSprings2dMobilityModel::GetMaxNodesNeighbours();

  if (!maxNodesNeighbours)
  {
    return 1;
  }

  //NS_LOG_DEBUG("kAtg = " << coveredNodes/maxNodesNeighbours);

  return coveredNodes/maxNodesNeighbours;
}

double
VirtualSprings2dMobilityModel::GetDistanceFromFurthestNeighbour ()
{ 
  double maxDist = 0.0;
  Vector myPos = m_helper.GetCurrentPosition ();

  for (uint16_t i = 0; i < m_neighbours.size (); i++)
  {
    Ptr<Node> node = NodeList::GetNode (m_neighbours[i]);
    Ptr<MobilityModel> mob = node -> GetObject<MobilityModel> ();
    Vector pos = mob -> GetPosition ();
    pos.z = 0;

    double dist = CalculateDistance (myPos, pos);

    if (dist > maxDist)
      maxDist = dist;

  }

  return maxDist;
}

double
VirtualSprings2dMobilityModel::GetDistanceFromBs ()
{ 
  Vector myPos = m_helper.GetCurrentPosition ();
  
  double dist = CalculateDistance (myPos, m_bsPos);

  return dist;
}

// bool
// VirtualSprings2dMobilityModel::HasOnlyAtaForces (uint32_t id, double tolerance)
// {
//   double maxShared = 0;
//   uint32_t minId = 9999;

//   for (uint16_t i = 0; i < m_neighbours.size (); i++)
//   {
//     uint32_t numShared = NumSharedEds ( m_neighbours[i] );

//     if  (numShared == maxShared && m_neighbours[i] < minId)
//       minId = m_neighbours[i];

//     if (numShared > maxShared)
//     {
//       maxShared = numShared;
//       minId = m_neighbours[i];
//     }

//     // if (numShared == maxShared && m_neighbours[i] < minId)
//     //   minId = m_neighbours[i];
//   }

//   //NS_LOG_LOGIC ("Node " << id << " share " << maxShared << " EDs with node " << minId);

//   if (maxShared / (double)m_eds.size () > tolerance && id < minId )
//     return true;

//   return false;

// }


uint32_t
VirtualSprings2dMobilityModel::FindMostSimilarNode ()
{
  uint32_t maxShared = 0;
  uint32_t maxId = 0;

  for (uint16_t i = 0; i < m_neighbours.size (); i++)
  {
    uint32_t numShared = NumSharedEds ( m_neighbours[i] );

    if  (numShared == maxShared && m_neighbours[i] > maxId)
      maxId = m_neighbours[i];

    if (numShared > maxShared)
    {
      maxShared = numShared;
      maxId = m_neighbours[i];
    }
  }

  return maxId;
}

uint32_t
VirtualSprings2dMobilityModel::NumSharedEds (uint32_t ataId)
{
  Ptr<Node> node = NodeList::GetNode (ataId);
  Ptr<VirtualSprings2dMobilityModel> mob = node -> GetObject<VirtualSprings2dMobilityModel> ();
  std::map<uint32_t, EdsEntry> neighEdsList = mob -> GetEdsList ();

  uint32_t counter = 0;
  //NS_LOG_LOGIC ("Node " << ataId << " EDs: ");
  for (std::map<uint32_t, EdsEntry>::iterator it = neighEdsList.begin (); it != neighEdsList.end (); ++it)
  {
    //NS_LOG_LOGIC (it -> first);
    if ( m_eds.find (it -> first) != m_eds.end () )
      counter ++;
  }

  return counter;
}

void
VirtualSprings2dMobilityModel::SetLinkBudgetEstimator (Ptr<LinkBudgetEstimator> estimator)
{
  m_estimator = estimator;
}

void
VirtualSprings2dMobilityModel::SetOlsrRouting (Ptr<olsr::RoutingProtocol> routing)
{
  m_routing = routing;
}

uint32_t
VirtualSprings2dMobilityModel::GetCoveredEds ()
{
  return m_eds.size ();
}

std::map<uint32_t, EdsEntry>
VirtualSprings2dMobilityModel::GetEdsList ()
{
  return m_monitor -> GetEdsList (Seconds (20));
}

void
VirtualSprings2dMobilityModel::AddAtaNode(uint32_t id)
{
  m_ataNodes.push_back(id); 
}

void
VirtualSprings2dMobilityModel::AddAtgNode(uint32_t id)
{
  m_atgNodes.push_back(id); 
}

void
VirtualSprings2dMobilityModel::DoWalk (Time delayLeft)
{
  m_event.Cancel ();

  m_event = Simulator::Schedule (delayLeft, &VirtualSprings2dMobilityModel::DoInitializePrivate, this);

  NotifyCourseChange ();
}


void
VirtualSprings2dMobilityModel::DoDispose (void)
{
  // chain up
  MobilityModel::DoDispose ();
}

Vector
VirtualSprings2dMobilityModel::DoGetPosition (void) const
{
  m_helper.Update();
  return m_helper.GetCurrentPosition ();	
}

void
VirtualSprings2dMobilityModel::DoSetPosition (const Vector &position)
{ 
  m_helper.SetPosition (position);
  Simulator::Remove (m_event);
  m_event = Simulator::ScheduleNow (&VirtualSprings2dMobilityModel::DoInitializePrivate, this);
}

Vector
VirtualSprings2dMobilityModel::DoGetVelocity (void) const
{
  
  return m_helper.GetVelocity ();
  
}



} // namespace ns3


//KATG PLUS
/*
    //double kAtgPlus = 1;
    // if (m_kAtgPlusMode)
    // {
    //   uint16_t cnt = 1;

    //   for (uint16_t i = 0; i < m_neighbours.size (); i++)
    //   {
    //         Ptr<Node> ataNode = NodeList::GetNode(m_neighbours[i]);
    //         Ptr<MobilityModel> ataMob = ataNode -> GetObject<MobilityModel> ();
    //         Vector ataPos = ataMob -> GetPosition ();
    //         ataPos.z = 0;

    //         double ataDist = CalculateDistance (pos, ataPos);

    //         if (ataDist < m_rangeAtg)
    //         {
    //           cnt++;
    //         }

    //       }

    //       cnt > 1 ? kAtgPlus = 1 : kAtgPlus = 10;

    //       //NS_LOG_DEBUG ("K_ATG_PLUS = " << kAtgPlus);
    // }
    //double force = ( kAtg * kAtgPlus ) * disp;

    //double disp = dist - m_l0Atg;

*/

// void
// VirtualSprings2dMobilityModel::SetEdsList ()
// {
//   std::map<uint32_t, Time> curNodes = m_monitor -> GetEdsList ();

//   m_eds.clear ();

//   for (std::map<uint32_t, Time>::iterator it = curNodes.begin(); it != curNodes.end (); ++it)
//   {
//     Time time = (*it).second;

//     if (time + Seconds(20) > Simulator::Now () )
//       m_eds.push_back ((*it).first);
//   }
// }