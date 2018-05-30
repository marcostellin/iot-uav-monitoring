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
 * Author: Marco Stellin
 */

#ifndef LORA_EDS_MONITOR_H
#define LORA_EDS_MONITOR_H

#include "ns3/object.h"
#include "ns3/gateway-lora-phy.h"

namespace ns3 {

struct EdsEntry 
{
	double x;
	double y;
	Time time;
};

/**
 * This class keeps track of the connected EDs to the gateway
 */
class LoraEdsMonitor : public SimpleRefCount<LoraEdsMonitor>
{
public:

  LoraEdsMonitor();
  virtual ~LoraEdsMonitor();

  LoraEdsMonitor(Ptr<GatewayLoraPhy> gatewayPhy);

  std::map<uint32_t, EdsEntry> GetEdsList (void);
  std::map<uint32_t, EdsEntry> GetEdsList (Time tolerance);


private:

  void UpdateList (Ptr<Packet const> packet, uint32_t id);

  Ptr<GatewayLoraPhy> m_phy;   //!< Pointer to the phy

  std::map<uint32_t, EdsEntry> m_eds;   

};

}

#endif /* DEVICE_STATUS_H */
