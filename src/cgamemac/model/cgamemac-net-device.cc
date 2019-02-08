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
#include "ns3/cgame-class.h"
#include "cgamemac-net-device.h"
#include "cgamemac-mac.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-channel.h"
#include "ns3/wifi-remote-station-manager.h"
#include "ns3/llc-snap-header.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/node.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/log.h"
#include "ns3/core-module.h"
namespace cgamemac {

NS_LOG_COMPONENT_DEFINE("CgamemacNetDevice");
NS_OBJECT_ENSURE_REGISTERED(CgamemacNetDevice);

ns3::TypeId CgamemacNetDevice::GetTypeId(void) {
	static ns3::TypeId tid =
			ns3::TypeId("cgamemac::CgamemacNetDevice").SetParent<ns3::NetDevice>().AddConstructor<
					CgamemacNetDevice>().AddAttribute("Mtu",
					"The MAC-level Maximum Transmission Unit",
					ns3::UintegerValue(
							MAX_MSDU_SIZE - ns3::LLC_SNAP_HEADER_LENGTH),
					ns3::MakeUintegerAccessor(&CgamemacNetDevice::SetMtu,
							&CgamemacNetDevice::GetMtu),
					ns3::MakeUintegerChecker<uint16_t>(1,
							MAX_MSDU_SIZE - ns3::LLC_SNAP_HEADER_LENGTH)).AddAttribute(
					"Channel", "The channel attached to this device",
					ns3::PointerValue(),
					ns3::MakePointerAccessor(&CgamemacNetDevice::DoGetChannel),
					ns3::MakePointerChecker<ns3::WifiChannel>()).AddAttribute(
					"Phy", "The PHY layer attached to this device.",
					ns3::PointerValue(),
					ns3::MakePointerAccessor(&CgamemacNetDevice::GetPhy,
							&CgamemacNetDevice::SetPhy),
					ns3::MakePointerChecker<ns3::WifiPhy>()).AddAttribute("Mac",
					"The MAC layer attached to this device.",
					ns3::PointerValue(),
					ns3::MakePointerAccessor(&CgamemacNetDevice::GetMac,
							&CgamemacNetDevice::SetMac),
					ns3::MakePointerChecker<CgamemacMac>());

	return tid;
}

CgamemacNetDevice::CgamemacNetDevice() :
		m_ifIndex(0), m_linkUp(false), m_mtu(
				MAX_MSDU_SIZE - ns3::LLC_SNAP_HEADER_LENGTH), m_configComplete(
				false) {
	NS_LOG_FUNCTION_NOARGS ();
}
CgamemacNetDevice::~CgamemacNetDevice() {
	NS_LOG_FUNCTION_NOARGS ();
}

void CgamemacNetDevice::DoDispose(void) {
	NS_LOG_FUNCTION_NOARGS ();
	m_node = 0;
	m_mac->Dispose();
	m_phy->Dispose();
	m_mac = 0;
	m_phy = 0;
	// chain up.
	NetDevice::DoDispose();
}

void CgamemacNetDevice::DoInitialize(void) {
	m_phy->Initialize();
	m_mac->Initialize();
	NetDevice::DoInitialize();
}

void CgamemacNetDevice::CompleteConfig(void) {
	if (m_mac == 0 || m_phy == 0 || m_node == 0 || m_configComplete) {
		NS_LOG_WARN("Complete config called with some of the entities not set");
		return;
	}
	m_mac->SetWifiPhy(m_phy);
	m_mac->SetForwardUpCallback(
			MakeCallback(&CgamemacNetDevice::ForwardUp, this));
	m_mac->SetLinkUpCallback(MakeCallback(&CgamemacNetDevice::LinkUp, this));
	m_mac->SetLinkDownCallback(
			MakeCallback(&CgamemacNetDevice::LinkDown, this));
	m_configComplete = true;
	NS_LOG_DEBUG("Complete config called and all entities were set");
}

void CgamemacNetDevice::SetMac(ns3::Ptr<CgamemacMac> mac) {
	m_mac = mac;
	CompleteConfig();
}
void CgamemacNetDevice::SetPhy(ns3::Ptr<ns3::WifiPhy> phy) {
	m_phy = phy;
	CompleteConfig();
}
ns3::Ptr<CgamemacMac> CgamemacNetDevice::GetMac(void) const {
	return m_mac;
}
ns3::Ptr<ns3::WifiPhy> CgamemacNetDevice::GetPhy(void) const {
	return m_phy;
}
void CgamemacNetDevice::SetIfIndex(const uint32_t index) {
	m_ifIndex = index;
}
uint32_t CgamemacNetDevice::GetIfIndex(void) const {
	return m_ifIndex;
}
ns3::Ptr<ns3::Channel> CgamemacNetDevice::GetChannel(void) const {
	return m_phy->GetChannel();
}
ns3::Ptr<ns3::WifiChannel> CgamemacNetDevice::DoGetChannel(void) const {
	return m_phy->GetChannel();
}
void CgamemacNetDevice::SetAddress(ns3::Address address) {
	m_mac->SetAddress(ns3::Mac48Address::ConvertFrom(address));
}
ns3::Address CgamemacNetDevice::GetAddress(void) const {
	return m_mac->GetAddress();
}
bool CgamemacNetDevice::SetMtu(const uint16_t mtu) {
	if (mtu > MAX_MSDU_SIZE - ns3::LLC_SNAP_HEADER_LENGTH) {
		return false;
	}
	m_mtu = mtu;
	return true;
}
uint16_t CgamemacNetDevice::GetMtu(void) const {
	return m_mtu;
}
bool CgamemacNetDevice::IsLinkUp(void) const {
	return m_phy != 0 && m_linkUp;
}
void CgamemacNetDevice::AddLinkChangeCallback(ns3::Callback<void> callback) {
	m_linkChanges.ConnectWithoutContext(callback);
}
bool CgamemacNetDevice::IsBroadcast(void) const {
	return true;
}
ns3::Address CgamemacNetDevice::GetBroadcast(void) const {
	return ns3::Mac48Address::GetBroadcast();
}
bool CgamemacNetDevice::IsMulticast(void) const {
	return true;
}
ns3::Address CgamemacNetDevice::GetMulticast(
		ns3::Ipv4Address multicastGroup) const {
	return ns3::Mac48Address::GetMulticast(multicastGroup);
}
ns3::Address CgamemacNetDevice::GetMulticast(ns3::Ipv6Address addr) const {
	return ns3::Mac48Address::GetMulticast(addr);
}
bool CgamemacNetDevice::IsPointToPoint(void) const {
	return false;
}
bool CgamemacNetDevice::IsBridge(void) const {
	return false;
}
bool CgamemacNetDevice::Send(ns3::Ptr<ns3::Packet> packet,
		const ns3::Address& dest, uint16_t protocolNumber) {
	NS_ASSERT(ns3::Mac48Address::IsMatchingType(dest));

	ns3::Mac48Address realTo = ns3::Mac48Address::ConvertFrom(dest);

	ns3::LlcSnapHeader llc;
	llc.SetType(protocolNumber);
	packet->AddHeader(llc);

	m_mac->Enqueue(packet, realTo);
	return true;
}
ns3::Ptr<ns3::Node> CgamemacNetDevice::GetNode(void) const {
	return m_node;
}
void CgamemacNetDevice::SetNode(ns3::Ptr<ns3::Node> node) {
	m_node = node;
	CompleteConfig();
}
bool CgamemacNetDevice::NeedsArp(void) const {
	return true;
}
void CgamemacNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb) {
	m_forwardUp = cb;
}
void CgamemacNetDevice::ForwardUp(ns3::Ptr<ns3::Packet> packet,
		ns3::Mac48Address from, ns3::Mac48Address to) {
	ns3::LlcSnapHeader llc;
	packet->RemoveHeader(llc);
	enum NetDevice::PacketType type;
	if (to.IsBroadcast()) {
		type = NetDevice::PACKET_BROADCAST;
	} else if (to.IsGroup()) {
		type = NetDevice::PACKET_MULTICAST;
	} else if (to == m_mac->GetAddress()) {
		type = NetDevice::PACKET_HOST;
	} else {
		type = NetDevice::PACKET_OTHERHOST;
	}

	if (type != NetDevice::PACKET_OTHERHOST) {
		m_forwardUp(this, packet, llc.GetType(), from);
	}

	if (!m_promiscRx.IsNull()) {
		m_promiscRx(this, packet, llc.GetType(), from, to, type);
	}
}
void CgamemacNetDevice::LinkUp(void) {
	m_linkUp = true;
	m_linkChanges();
}
void CgamemacNetDevice::LinkDown(void) {
	m_linkUp = false;
	m_linkChanges();
}
bool CgamemacNetDevice::SendFrom(ns3::Ptr<ns3::Packet> packet,
		const ns3::Address& source, const ns3::Address& dest,
		uint16_t protocolNumber) {
	NS_ASSERT(ns3::Mac48Address::IsMatchingType(dest));
	NS_ASSERT(ns3::Mac48Address::IsMatchingType(source));

	ns3::Mac48Address realTo = ns3::Mac48Address::ConvertFrom(dest);
	ns3::Mac48Address realFrom = ns3::Mac48Address::ConvertFrom(source);

	ns3::LlcSnapHeader llc;
	llc.SetType(protocolNumber);
	packet->AddHeader(llc);

	m_mac->Enqueue(packet, realTo, realFrom);

	return true;
}
void CgamemacNetDevice::SetPromiscReceiveCallback(PromiscReceiveCallback cb) {
	m_promiscRx = cb;
}
bool CgamemacNetDevice::SupportsSendFrom(void) const {
	return m_mac->SupportsSendFrom();
}
void CgamemacNetDevice::SetGraphUpLoadCallback(
		ns3::Callback<void, ns3::Ptr<cgame::GraphUpLoad> > upcallack) {
	m_mac->SetGraphUpLoadCallback(upcallack);
}
void CgamemacNetDevice::SendtoMacLayer(
		ns3::Ptr<cgame::cgameDownload> download) {
	m_mac->SendtoMacLayer(download);
}

} // namespace cgamemac
