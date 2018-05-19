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
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/node-list.h"
#include "ns3/boolean.h"
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

    .AddAttribute ("l0Ata",
                   "Natural length of AtA springs",
                   DoubleValue (150),
                   MakeDoubleAccessor (&VirtualSprings2dMobilityModel::m_l0Ata),
                   MakeDoubleChecker<double> ())

    .AddAttribute ("l0Atg",
                   "Natural length of AtG springs",
                   DoubleValue (20),
                   MakeDoubleAccessor (&VirtualSprings2dMobilityModel::m_l0Atg),
                   MakeDoubleChecker<double> ())
    
                   
    .AddAttribute ("TxRangeAta",
                   "Transmission Range of AtA links",
                   DoubleValue (300),
                   MakeDoubleAccessor (&VirtualSprings2dMobilityModel::m_rangeAta),
                   MakeDoubleChecker<double> ())

    .AddAttribute ("TxRangeAtg",
                   "Transmission Range of AtG links",
                   DoubleValue (100),
                   MakeDoubleAccessor (&VirtualSprings2dMobilityModel::m_rangeAtg),
                   MakeDoubleChecker<double> ())

    .AddAttribute ("BsPosition", "Position of BS",
                       VectorValue (Vector (200.0, 200.0, 0.0)), // ignored initial value.
                       MakeVectorAccessor (&VirtualSprings2dMobilityModel::m_bsPos),
                       MakeVectorChecker ())

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
  // m_nodesCovered.resize (m_atgNodes.size());

  // for (uint32_t j = 0; j < m_nodesCovered.capacity(); j++)
  // {
  //   m_nodesCovered[j] = -1;
  // }

  DoInitializePrivate ();
  MobilityModel::DoInitialize ();
}

void
VirtualSprings2dMobilityModel::DoInitializePrivate (void)
{
  m_helper.Update();
  double speed = m_speed;
  
  //Compute the direction of the movement
  Vector forceAta = VirtualSprings2dMobilityModel::ComputeAtaForce();
  Vector forceAtg = VirtualSprings2dMobilityModel::ComputeAtgForce();
  Vector force = forceAta + forceAtg;

  NS_LOG_DEBUG ("AtG Force = " << forceAtg << ", AtA force = " << forceAta);

  double k = speed/std::sqrt(force.x*force.x + force.y*force.y);
  Vector vector (k*force.x,
  				 k*force.y,
  				 0.0);

  if (force.GetLength () > m_tol)
  {
    //Compute the future position based on current velocity and direction
    Vector myPos = m_helper.GetCurrentPosition();
    Vector nextPos;
    nextPos.x = myPos.x + vector.x * m_modeTime.GetSeconds ();
    nextPos.y = myPos.y + vector.y * m_modeTime.GetSeconds ();

    m_helper.SetVelocity(vector);
    m_helper.Unpause();

    if (VirtualSprings2dMobilityModel::HasPathToBs (nextPos))
    {
      m_helper.SetVelocity(vector);
      m_helper.Unpause();
    } 
    else 
    {
      NS_LOG_DEBUG ("Not in range of BS");
      m_helper.Pause ();
    }
 
  }
  else 
  {
    m_helper.Pause();
  }

  Time delayLeft = m_modeTime;

  DoWalk (delayLeft);
  
}

bool
VirtualSprings2dMobilityModel::HasPathToBs (Vector myPos)
{
  //Apply DFS to find any path to the BS
  int visited [m_ataNodes.size()] = {};
  int inQueue [m_ataNodes.size()] = {};
  std::queue<int> toVisit;

  if (CalculateDistance(myPos, m_bsPos) < m_rangeAta)
    return true;

  for (uint16_t j = 0; j < m_ataNodes.size(); j++)
  {
    Ptr<Node> node = NodeList::GetNode(m_ataNodes[j]);
    Ptr<MobilityModel> otherMob = node -> GetObject<MobilityModel>();
    Vector otherPos = otherMob -> GetPosition();
    otherPos.z = 0;
    double dist = CalculateDistance(myPos, otherPos);

    if (dist > 0  && dist < m_rangeAta && !visited[j] && !inQueue[j])
    {   
      toVisit.push(j);
      inQueue[j] = 1;
      //NS_LOG_DEBUG("Insert in queue " << j);
    }
   }

   int cur;
   while (!toVisit.empty()) 
   { 
     cur = toVisit.front();
     toVisit.pop();
     //NS_LOG_DEBUG ("Currently visiting " << cur);

     Ptr<Node> node = NodeList::GetNode(m_ataNodes[cur]);
     Ptr<MobilityModel> mob = node -> GetObject<MobilityModel>();
     Vector pos = mob -> GetPosition();
     pos.z = 0;

    if (CalculateDistance(pos, m_bsPos) < m_rangeAta)
      return true;

    for (uint16_t j = 0; j < m_ataNodes.size(); j++)
    {
      if (j != cur)
      {
        Ptr<Node> node2 = NodeList::GetNode(m_ataNodes[j]);
        Ptr<MobilityModel> otherMob = node2 -> GetObject<MobilityModel>();
        Vector otherPos = otherMob -> GetPosition();
        otherPos.z = 0;
        double dist = CalculateDistance(pos, otherPos);

        if (dist > 0  && dist < m_rangeAta && !visited[j] &&!inQueue[j])
        {   
          toVisit.push(j);
          inQueue[j] = 1;
          //NS_LOG_DEBUG("Insert in queue " << j);
        }
      }
    }

     visited[cur] = 1;
   }

   return false;

}


Vector
VirtualSprings2dMobilityModel::ComputeAtaForce()
{
  Vector myPos = m_helper.GetCurrentPosition ();

  double fx = 0;
  double fy = 0;
  
  for (uint16_t j = 0; j < m_ataNodes.size(); j++)
  {
    Ptr<Node> node = NodeList::GetNode(m_ataNodes[j]);
    Ptr<MobilityModel> otherMob = node -> GetObject<MobilityModel>();
    Vector otherPos = otherMob -> GetPosition();
    double dist = CalculateDistance(myPos, otherPos);
    //NS_LOG_DEBUG("otherPos=" << otherPos << " distace= " << dist);

    if (dist > 0  && dist < m_rangeAta)
    {   

        double disp = dist - m_l0Ata;
        Vector diff = otherPos - myPos;
        double force = m_kAta*disp;
        double k = force/std::sqrt(diff.x*diff.x + diff.y*diff.y);

        fx += k*diff.x;
        fy += k*diff.y;

    }
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

  for (uint16_t j = 0; j < m_atgNodes.size(); j++)
  {
    Ptr<Node> node = NodeList::GetNode(m_atgNodes[j]);
    Ptr<MobilityModel> mob = node -> GetObject<MobilityModel>();
    Vector pos = mob -> GetPosition();
    pos.z = 0;

    double dist = CalculateDistance(myPos, pos);
    double kAtgPlus = 1;

    if (dist < m_rangeAtg)
    {   
        if (m_kAtgPlusMode)
        {
          uint16_t cnt = 1;
          for (uint16_t i = 0; i < m_ataNodes.size (); i++)
          {
            Ptr<Node> ataNode = NodeList::GetNode(m_ataNodes[i]);
            Ptr<MobilityModel> ataMob = ataNode -> GetObject<MobilityModel> ();
            Vector ataPos = ataMob -> GetPosition ();
            ataPos.z = 0;

            double ataDist = CalculateDistance (pos, ataPos);

            if (ataDist < m_rangeAtg)
            {
              cnt++;
            }

          }

          cnt > 1 ? kAtgPlus = 1 : kAtgPlus = 2;
          //double kAtgPlus = 1.0 / (double)cnt;

          NS_LOG_DEBUG ("K_ATG_PLUS = " << kAtgPlus);
        }

        double disp = dist - m_l0Atg;
        Vector diff = pos - myPos;
        double force = ( kAtg * kAtgPlus ) * disp;
        double k = force/std::sqrt(diff.x*diff.x + diff.y*diff.y);

        fx += k*diff.x;
        fy += k*diff.y;
    }
   }

   return Vector(fx,fy,0);

}

int
VirtualSprings2dMobilityModel::GetMaxNodesNeighbours()
{
  Vector myPos = m_helper.GetCurrentPosition ();
  myPos.z = 0;

  int curMax = 0;
  for (uint16_t j = 0; j < m_ataNodes.size(); j++)
  {
    Ptr<Node> ataNode = NodeList::GetNode(m_ataNodes[j]);
    Ptr<MobilityModel> ataNodeMob = ataNode -> GetObject<MobilityModel>();
    Vector ataNodePos = ataNodeMob -> GetPosition();
    ataNodePos.z = 0;

    double dist = CalculateDistance(myPos, ataNodePos);

    if (dist < m_rangeAta)
    {
      int numAtgNodes = VirtualSprings2dMobilityModel::ComputeNumGroudNodes(ataNode);

      if (numAtgNodes > curMax){
        curMax = numAtgNodes;
      }
    }
  }

  return curMax;
}

double
VirtualSprings2dMobilityModel::ComputeKatg()
{
  double coveredNodes = VirtualSprings2dMobilityModel::ComputeNumGroudNodes();
  double maxNodesNeighbours = VirtualSprings2dMobilityModel::GetMaxNodesNeighbours();

  if (!maxNodesNeighbours)
  {
    return 1;
  }

  NS_LOG_DEBUG("kAtg = " << coveredNodes/maxNodesNeighbours);

  return coveredNodes/maxNodesNeighbours;
}

int
VirtualSprings2dMobilityModel::ComputeNumGroudNodes(Ptr<Node> node )
{
  Ptr<MobilityModel> mob = node -> GetObject<MobilityModel>();
  Vector pos = mob -> GetPosition();
  pos.z = 0;

  //bool coverageChange = false;
  int counter = 0;
  std::vector<int> nodesCovered;

  for (uint16_t j = 0; j < m_atgNodes.size(); j++)
  {
    Ptr<Node> atgNode = NodeList::GetNode(m_atgNodes[j]);
    Ptr<MobilityModel> atgNodeMob = atgNode -> GetObject<MobilityModel>();
    Vector atgNodePos = atgNodeMob -> GetPosition();
    atgNodePos.z = 0;
    double dist = CalculateDistance(pos, atgNodePos);

    if (dist < m_rangeAtg)
    {
      nodesCovered.push_back (m_atgNodes[j]);

      counter++;

    } 
  }

  m_nodesInRangeTrace (nodesCovered);

  return counter;
}

int
VirtualSprings2dMobilityModel::ComputeNumGroudNodes()
{
  Vector myPos = m_helper.GetCurrentPosition ();
  myPos.z = 0;

  int counter = 0;

  for (uint16_t j = 0; j < m_atgNodes.size(); j++)
  {
    Ptr<Node> node = NodeList::GetNode(m_atgNodes[j]);
    Ptr<MobilityModel> mob = node -> GetObject<MobilityModel>();
    Vector pos = mob -> GetPosition();
    pos.z = 0;
    double dist = CalculateDistance(myPos, pos);

    if (dist < m_rangeAtg)
    {
      counter++;
    }
  }

  return counter;

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
