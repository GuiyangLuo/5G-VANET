/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Jens Mittag, Tristan Gaugel
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
 * Author: Jens Mittag <jens.mittag@gmail.com>
 *         Tristan Gaugel <tristan.gaugel@kit.edu>
 */

#include "cgamemac-helper.h"
#include "cgamemac-mac-helper.h"
#include "ns3/cgamemac-mac.h"
#include "ns3/cgamemac-net-device.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-channel.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/simulator.h"
#include "ns3/names.h"

NS_LOG_COMPONENT_DEFINE("cgamemacHelper");

namespace cgamemac {

CgamemacHelper::CgamemacHelper() :
		m_standard(ns3::WIFI_PHY_STANDARD_80211p_CCH) {
}

CgamemacHelper CgamemacHelper::Default(void) {
	CgamemacHelper helper;
	return helper;
}

void CgamemacHelper::SetStandard(enum ns3::WifiPhyStandard standard) {
	m_standard = standard;
}

ns3::NetDeviceContainer CgamemacHelper::Install(
		const ns3::WifiPhyHelper &phyHelper, const CgamemacMacHelper &macHelper,
		ns3::NodeContainer c, std::vector<ns3::Time> startups) const {
	NS_ASSERT(c.GetN() <= startups.size());
	ns3::NetDeviceContainer devices;
	uint32_t index = 0;
	for (ns3::NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
		ns3::Ptr<ns3::Node> node = *i;
		ns3::Ptr<CgamemacNetDevice> device =
				ns3::CreateObject<CgamemacNetDevice>();
		ns3::Ptr<CgamemacMac> mac = macHelper.Create();
		ns3::Ptr<ns3::WifiPhy> phy = phyHelper.Create(node, device);
		mac->SetAddress(ns3::Mac48Address::Allocate());
		mac->ConfigureStandard(m_standard);
		phy->ConfigureStandard(m_standard);
		device->SetMac(mac);
		device->SetPhy(phy);
		node->AddDevice(device);
		devices.Add(device);
		ns3::Simulator::ScheduleWithContext(node->GetId(), startups[index],
				&CgamemacMac::StartInitializationPhase, mac);
		index++;
	}
	return devices;
}


ns3::NetDeviceContainer CgamemacHelper::Install(
		const ns3::WifiPhyHelper &phyHelper, const CgamemacMacHelper &macHelper,
		ns3::NodeContainer c) const {
	ns3::NetDeviceContainer devices;
	for (ns3::NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
		ns3::Ptr<ns3::Node> node = *i;
		ns3::Ptr<CgamemacNetDevice> device =
				ns3::CreateObject<CgamemacNetDevice>();
		ns3::Ptr<CgamemacMac> mac = macHelper.Create();
		ns3::Ptr<ns3::WifiPhy> phy = phyHelper.Create(node, device);
		mac->SetAddress(ns3::Mac48Address::Allocate());
		mac->ConfigureStandard(m_standard);
		phy->ConfigureStandard(m_standard);
		device->SetMac(mac);
		device->SetPhy(phy);
		node->AddDevice(device);
		devices.Add(device);
		ns3::Simulator::ScheduleWithContext(node->GetId(), ns3::Seconds(0),
				&CgamemacMac::StartInitializationPhase, mac);
	}
	return devices;
}

ns3::NetDeviceContainer CgamemacHelper::InstallRsuAndObu(
		const ns3::WifiPhyHelper &phyHelper, const CgamemacMacHelper &macHelper,
		ns3::NodeContainer c, uint8_t type) const {
	ns3::NetDeviceContainer devices;
	for (ns3::NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
		ns3::Ptr<ns3::Node> node = *i;
		ns3::Ptr<CgamemacNetDevice> device =
				ns3::CreateObject<CgamemacNetDevice>();
		ns3::Ptr<CgamemacMac> mac = macHelper.Create();
		ns3::Ptr<ns3::WifiPhy> phy = phyHelper.Create(node, device);
		mac->SetAddress(ns3::Mac48Address::Allocate());
		mac->ConfigureStandard(m_standard);
		phy->ConfigureStandard(m_standard);
		device->SetMac(mac);
		device->SetPhy(phy);
		node->AddDevice(device);
		devices.Add(device);
		ns3::Simulator::ScheduleWithContext(node->GetId(), ns3::Seconds(0),
				&CgamemacMac::StartInitializationPhaseRsuAndObu, mac, type);
	}
	return devices;
}
ns3::NetDeviceContainer CgamemacHelper::Install(const ns3::WifiPhyHelper &phy,
		const CgamemacMacHelper &mac, ns3::Ptr<ns3::Node> node) const {
	return Install(phy, mac, ns3::NodeContainer(node));
}

ns3::NetDeviceContainer CgamemacHelper::Install(const ns3::WifiPhyHelper &phy,
		const CgamemacMacHelper &mac, std::string nodeName) const {
	ns3::Ptr<ns3::Node> node = ns3::Names::Find<ns3::Node>(nodeName);
	return Install(phy, mac, ns3::NodeContainer(node));
}

} // namespace cgamemac
