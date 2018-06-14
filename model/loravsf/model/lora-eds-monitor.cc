/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#include "ns3/lora-eds-monitor.h"
#include "ns3/log.h"
#include "ns3/lora-tag.h"
#include "ns3/node-list.h"
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoraEdsMonitor");

LoraEdsMonitor::LoraEdsMonitor ()
{
  NS_LOG_FUNCTION (this);
}

LoraEdsMonitor::~LoraEdsMonitor ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

LoraEdsMonitor::LoraEdsMonitor (Ptr<GatewayLoraPhy> gatewayPhy)
{
  NS_LOG_FUNCTION (this);
  m_phy = gatewayPhy;
  m_phy -> TraceConnectWithoutContext ("ReceivedPacket",
                                        MakeCallback (&LoraEdsMonitor::UpdateList, this));
  //NS_LOG_UNCOND (this);

}

void
LoraEdsMonitor::UpdateList (Ptr<Packet const> packet, uint32_t id)
{
  NS_LOG_FUNCTION (this);

  //NS_LOG_UNCOND (m_eds.size());
  LoraTag tag;
  bool found = packet -> PeekPacketTag (tag);

  if (found)
  {    
    std::map<uint32_t, EdsEntry>::iterator it;
    uint32_t edId = tag.GetId ();
    Ptr<Node> node = NodeList::GetNode (edId);
    Ptr<MobilityModel> mob = node -> GetObject<MobilityModel> ();
    Vector pos = mob -> GetPosition ();

    it = m_eds.find (tag.GetId ());

    if (it != m_eds.end ())
    {
       it -> second.time = Simulator::Now ();
       it -> second.x = pos.x;
       it -> second.y = pos.y;
    }
    else
    {
       EdsEntry entry;
       entry.time = Simulator::Now ();
       entry.x = pos.x;
       entry.y = pos.y;

       m_eds[edId] = entry;
    }
  }

}

void
LoraEdsMonitor::UpdateHistory (Time tolerance)
{
  std::map<uint32_t, EdsEntry> eds = LoraEdsMonitor::GetEdsList (tolerance);

  if (eds.size () > 0)
  {
    EdsEntry e;
    //Compute center of mass of eds
    for (std::map<uint32_t, EdsEntry>::iterator it = eds.begin (); it != eds.end (); ++it )
    {
      //NS_LOG_INFO ("ED : " << (*it).second.x << ":" << (*it).second.y);
      e.x = e.x + (*it).second.x;
      e.y = e.y + (*it).second.y;
      e.time = (*it).second.time;
    }

    e.x = e.x / eds.size ();
    e.y = e.y / eds.size ();

    //Save entry in queue
    m_eds_hist.push (e);
    NS_LOG_INFO ("Center mass: " << e.x << ":" << e.y);
    if (m_eds_hist.size () > 3)
      m_eds_hist.pop ();
  }

}

Vector
LoraEdsMonitor::GetDirection ()
{
  std::queue<EdsEntry> eds = LoraEdsMonitor::GetEdsHistory ();

  double size = eds.size ();
  Vector res;

  while (!eds.empty ())
  {
    Vector v1 ( eds.front ().x, eds.front ().y, 0.0 );
    eds.pop ();

    if (!eds.empty ())
    {
      Vector v2 ( eds.front ().x, eds.front ().y, 0.0 );
      res.x += (v2 - v1).x;
      res.y += (v2 - v1).y;
    }
  }

  if (size > 1)
  {
    res.x = res.x / (size - 1);
    res.y = res.y / (size - 1);
  }

  return res;

}

double
LoraEdsMonitor::GetSpeed ()
{
  std::queue<EdsEntry> eds = LoraEdsMonitor::GetEdsHistory ();

  double interval =  10.0;
  double size = eds.size ();
  double speed = 0;

  while (!eds.empty ())
  {
    Vector v1 ( eds.front ().x, eds.front ().y, 0.0 );
    eds.pop ();

    if (!eds.empty ())
    {
      Vector v2 ( eds.front ().x, eds.front ().y, 0.0 );
      speed += CalculateDistance (v1, v2) / interval;
    }
  }

  if (size > 1)
  {
    speed = speed / (size - 1);
  }

  return speed;

}

Vector
LoraEdsMonitor::GetSpeedVector ()
{
  Vector dir = LoraEdsMonitor::GetDirection ();
  double speed = LoraEdsMonitor::GetSpeed ();

  Vector vector;

  if (dir.GetLength () > 0)
  {
    double k = speed/std::sqrt(dir.x*dir.x + dir.y*dir.y);
    vector.x = k*dir.x;
    vector.y = k*dir.y;
  }

  NS_LOG_INFO ("Direction: " << dir << " Speed: " << speed << " Speed Vector: " << vector);

  return vector;

}

Time
LoraEdsMonitor::GetLastTime ()
{
  return m_eds_hist.back ().time;
}

Vector
LoraEdsMonitor::GetLastPosition ()
{
  return Vector ( m_eds_hist.back().x, m_eds_hist.back ().y, 0.0 );
}

std::map<uint32_t, EdsEntry>
LoraEdsMonitor::GetEdsList (void)
{
  return m_eds;
}

std::queue<EdsEntry>
LoraEdsMonitor::GetEdsHistory (void)
{
  return m_eds_hist;
}

std::map<uint32_t, EdsEntry>
LoraEdsMonitor::GetEdsList (Time tolerance)
{
  Time cur = Simulator::Now ();

  std::map<uint32_t, EdsEntry> filtered;

  for (std::map<uint32_t, EdsEntry>::iterator it = m_eds.begin (); it != m_eds.end (); ++it)
  {
    Time last = (*it).second.time;

    if (cur < last + tolerance)
      filtered.insert (std::pair<uint32_t, EdsEntry> ( (*it).first, (*it).second ));
  }

  return filtered;
}

}
