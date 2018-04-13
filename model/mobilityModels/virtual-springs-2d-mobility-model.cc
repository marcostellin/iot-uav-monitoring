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
#include <cmath>
#include <algorithm>

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
                   MakeDoubleChecker<double> ());          
                   
                   
  return tid;
}

void
VirtualSprings2dMobilityModel::DoInitialize (void)
{
  DoInitializePrivate ();
  MobilityModel::DoInitialize ();
}

void
VirtualSprings2dMobilityModel::DoInitializePrivate (void)
{
  m_helper.Update();
  double speed = m_speed;
  
  //Compute the direction of the movement
  Vector force = VirtualSprings2dMobilityModel::ComputeAtaForce();
  //NS_LOG_DEBUG("Force");
  //NS_LOG_DEBUG(force);
  double k = speed/std::sqrt(force.x*force.x + force.y*force.y);
  Vector vector (k*force.x,
  				 k*force.y,
  				 0.0);

  //NS_LOG_DEBUG("Velocity");
  //NS_LOG_DEBUG(vector);

  m_helper.SetVelocity(vector);
  m_helper.Unpause();

  Time delayLeft = m_modeTime;

  DoWalk (delayLeft);
  
  /*
  m_helper.Update ();
  double speed = m_speed->GetValue ();
  double direction = m_direction->GetValue ();
  Vector vector (std::cos (direction) * speed,
                 std::sin (direction) * speed,
                 0.0);
  m_helper.SetVelocity (vector);
  m_helper.Unpause ();

  Time delayLeft;
  if (m_mode == VirtualSprings2dMobilityModel::MODE_TIME)
    {
      delayLeft = m_modeTime;
    }
  else
    {
      delayLeft = Seconds (m_modeDistance / speed); 
    }
  DoWalk (delayLeft);
  */
}


Vector
VirtualSprings2dMobilityModel::ComputeAtaForce()
{
  Vector myPos = m_helper.GetCurrentPosition ();
  NS_LOG_DEBUG("myPos=");
  NS_LOG_DEBUG(myPos);

  double fx = 0;
  double fy = 0;
  
  for (uint16_t j = 0; j < m_ataNodes.size(); j++)
  {
    Ptr<Node> node = NodeList::GetNode(m_ataNodes[j]);
    Ptr<MobilityModel> otherMob = node -> GetObject<MobilityModel>();
    Vector otherPos = otherMob -> GetPosition();
    double dist = CalculateDistance(myPos, otherPos);
    NS_LOG_DEBUG("otherPos=");
    NS_LOG_DEBUG(otherPos);
    NS_LOG_DEBUG("Distance");
    NS_LOG_DEBUG(dist);

    if (dist > 0  && dist < m_rangeAta)
    {   

        double disp = dist - m_l0Ata;
        Vector diff = otherPos - myPos;
        double force = m_kAta*disp;
        double k = force/std::sqrt(diff.x*diff.x + diff.y*diff.y);

        fx += k*diff.x;
        fy += k*diff.y;

        // double theta = difference.y / difference.x;
        // fx += -m_kAta*disp*cos(theta);
    }
   }
  
  // NS_LOG_DEBUG("ForceX:");
  // NS_LOG_DEBUG(fx);
  // NS_LOG_DEBUG("ForceY:");
  // NS_LOG_DEBUG(fy);

  return Vector(fx,fy,0);
}

void
VirtualSprings2dMobilityModel::AddAtaNode(uint32_t id)
{
  m_ataNodes.push_back(id); 
}

// void
// VirtualSprings2dMobilityModel::SetAtgNodes(std::vector<Ptr<Node>> nodes)
// {
//   m_atgNodes = nodes;
// }


void
VirtualSprings2dMobilityModel::DoWalk (Time delayLeft)
{
  m_event.Cancel ();

  m_event = Simulator::Schedule (delayLeft, &VirtualSprings2dMobilityModel::DoInitializePrivate, this);

  NotifyCourseChange ();

  /*
  Vector position = m_helper.GetCurrentPosition ();
  Vector speed = m_helper.GetVelocity ();
  Vector nextPosition = position;
  nextPosition.x += speed.x * delayLeft.GetSeconds ();
  nextPosition.y += speed.y * delayLeft.GetSeconds ();
  m_event.Cancel ();
  if (m_bounds.IsInside (nextPosition))
    {
      m_event = Simulator::Schedule (delayLeft, &VirtualSprings2dMobilityModel::DoInitializePrivate, this);
    }
  else
    {
      nextPosition = m_bounds.CalculateIntersection (position, speed);
      Time delay = Seconds ((nextPosition.x - position.x) / speed.x);
      m_event = Simulator::Schedule (delay, &VirtualSprings2dMobilityModel::Rebound, this,
                                     delayLeft - delay);
    }
  NotifyCourseChange ();
  */
}

/*
void
VirtualSprings2dMobilityModel::Rebound (Time delayLeft)
{
  m_helper.UpdateWithBounds (m_bounds);
  Vector position = m_helper.GetCurrentPosition ();
  Vector speed = m_helper.GetVelocity ();
  switch (m_bounds.GetClosestSide (position))
    {
    case Rectangle::RIGHT:
    case Rectangle::LEFT:
      speed.x = -speed.x;
      break;
    case Rectangle::TOP:
    case Rectangle::BOTTOM:
      speed.y = -speed.y;
      break;
    }
  m_helper.SetVelocity (speed);
  m_helper.Unpause ();
  DoWalk (delayLeft);
  
}

*/

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
  /*
  m_helper.UpdateWithBounds (m_bounds);
  return m_helper.GetCurrentPosition ();
  */
}
void
VirtualSprings2dMobilityModel::DoSetPosition (const Vector &position)
{ 
  m_helper.SetPosition (position);
  Simulator::Remove (m_event);
  m_event = Simulator::ScheduleNow (&VirtualSprings2dMobilityModel::DoInitializePrivate, this);

  /*
  NS_ASSERT (m_bounds.IsInside (position));
  m_helper.SetPosition (position);
  Simulator::Remove (m_event);
  m_event = Simulator::ScheduleNow (&VirtualSprings2dMobilityModel::DoInitializePrivate, this);
  */
}
Vector
VirtualSprings2dMobilityModel::DoGetVelocity (void) const
{
  
  return m_helper.GetVelocity ();
  
}



} // namespace ns3
