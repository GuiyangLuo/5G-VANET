/* -*-  Mode: C++; c-file-style: "gnu"); indent-tabs-mode:nil; -*- */
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

#include "cgamemac-mac.h"
#include "ns3/assert.h"
#include "ns3/callback.h"
#include "ns3/enum.h"
#include "ns3/fatal-error.h"
#include "ns3/fatal-impl.h"
#include "ns3/llc-snap-header.h"
#include "ns3/log.h"
#include "ns3/log-macros-disabled.h"
#include "ns3/mobility-model.h"
#include "ns3/node.h"
#include "ns3/node-list.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/object-base.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/type-id.h"
#include "ns3/wifi-mac-trailer.h"
#include "ns3/wifi-mode.h"
#include "ns3/wifi-phy-standard.h"
#include "ns3/wifi-preamble.h"
#include "ns3/wifi-tx-vector.h"
#include "ns3/cgame-class.h"
#include "cgamemac-header.h"
#include "cgamemac-net-device.h"
#include "ns3/boolean.h"
#include "ns3/cgame-class.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/yans-wifi-phy.h"

namespace cgamemac {

NS_LOG_COMPONENT_DEFINE("CgamemacMac");
NS_OBJECT_ENSURE_REGISTERED(CgamemacMac);

ns3::TypeId CgamemacMac::GetTypeId(void) {
	static ns3::TypeId tid =
			ns3::TypeId("cgamemac::CgamemacMac").SetParent<Object>().AddConstructor<
					CgamemacMac>().AddAttribute("WifiMode",
					"The WiFi mode to use for transmission",
					ns3::WifiModeValue(ns3::WifiMode("OfdmRate12MbpsBW10MHz")),
					ns3::MakeWifiModeAccessor(&CgamemacMac::m_wifiMode),
					ns3::MakeWifiModeChecker()).AddAttribute("GuardInterval",
					"The guard interval added at the end of the slot in us (no transmission time)",
					ns3::TimeValue(ns3::MicroSeconds(6)),
					ns3::MakeTimeAccessor(&CgamemacMac::m_guardInterval),
					ns3::MakeTimeChecker()).AddAttribute("m_schedulingPeriod",
					"The guard interval added at the end of the slot in us (no transmission time)",
					ns3::TimeValue(ns3::Seconds(2)),
					ns3::MakeTimeAccessor(&CgamemacMac::m_schedulingPeriod),
					ns3::MakeTimeChecker()).AddAttribute("m_begintime",
					"The guard interval added at the end of the slot in us (no transmission time)",
					ns3::TimeValue(ns3::Seconds(2)),
					ns3::MakeTimeAccessor(&CgamemacMac::m_begintime),
					ns3::MakeTimeChecker()).AddAttribute("MaximumPacketSize",
					"The maximum number of bytes per slot (excludes MAC cfrmac and lower layer wrapping)."
							"If packet is greater than this, it will be dropped ",
					ns3::UintegerValue(500),
					ns3::MakeUintegerAccessor(&CgamemacMac::m_maxPacketSize),
					ns3::MakeUintegerChecker<uint32_t>()).AddAttribute(
					"m_initializedNumber",
					"The maximum number of bytes per slot (excludes MAC cfrmac and lower layer wrapping)."
							"If packet is greater than this, it will be dropped ",
					ns3::UintegerValue(10),
					ns3::MakeUintegerAccessor(
							&CgamemacMac::m_initializedNumber),
					ns3::MakeUintegerChecker<uint8_t>()).AddAttribute(
					"m_numdataitems",
					"The maximum number of bytes per slot (excludes MAC cfrmac and lower layer wrapping)."
							"If packet is greater than this, it will be dropped ",
					ns3::UintegerValue(50),
					ns3::MakeUintegerAccessor(&CgamemacMac::m_numdataitems),
					ns3::MakeUintegerChecker<uint8_t>()).AddAttribute(
					"isInilitialized",
					"The maximum number of bytes per slot (excludes MAC cfrmac and lower layer wrapping)."
							"If packet is greater than this, it will be dropped ",
					ns3::UintegerValue(0),
					ns3::MakeUintegerAccessor(&CgamemacMac::isInilitialized),
					ns3::MakeUintegerChecker<uint8_t>()).AddAttribute(
					"WifiPreamble", "The WiFi preamble mode",
					ns3::EnumValue(ns3::WIFI_PREAMBLE_LONG),
					ns3::MakeEnumAccessor(&CgamemacMac::m_wifiPreamble),
					ns3::MakeEnumChecker(ns3::WIFI_PREAMBLE_LONG,
							"Long WiFi preamble", ns3::WIFI_PREAMBLE_SHORT,
							"Short WiFi preamble")).AddTraceSource("txTrace",
					"This event is triggered whenever the station transmits a packet",
					ns3::MakeTraceSourceAccessor(&CgamemacMac::m_txTrace)).AddTraceSource(
					"rxTrace",
					"This event is triggered whenever the station receives a packet",
					ns3::MakeTraceSourceAccessor(&CgamemacMac::m_rxTrace));
	return tid;
}

CgamemacMac::CgamemacMac() {
	// Queue to hold packets in
	m_queue = ns3::CreateObject<ns3::WifiMacQueue>();
	m_upload = ns3::Create<cgame::GraphUpLoad>();
	isInilitialized = 0;

}

CgamemacMac::~CgamemacMac() {

}

void CgamemacMac::ForwardUp(ns3::Ptr<ns3::Packet> packet,
		ns3::Mac48Address from, ns3::Mac48Address to) {
	NS_LOG_FUNCTION(this << packet << from);
	m_forwardUp(packet, from, to);
}

void CgamemacMac::SetAddress(ns3::Mac48Address address) {
	m_self = address;
	SetBssid(address);
}

bool CgamemacMac::SupportsSendFrom(void) const {
	return false;
}

void CgamemacMac::SetWifiPhy(ns3::Ptr<ns3::WifiPhy> phy) {
	NS_LOG_FUNCTION(this << phy);
	m_phy = phy;
	// Stuff to do when events occur
	m_phy->SetReceiveOkCallback(ns3::MakeCallback(&CgamemacMac::Receive, this));

}

void CgamemacMac::Enqueue(ns3::Ptr<const ns3::Packet> packet,
		ns3::Mac48Address to, ns3::Mac48Address from) {
	NS_FATAL_ERROR(
			"This MAC entity (" << this << ", " << GetAddress () << ") does not support Enqueue() with from address");
}

void CgamemacMac::Enqueue(ns3::Ptr<const ns3::Packet> packet,
		ns3::Mac48Address to) {
	NS_LOG_FUNCTION(this << packet << to);
	ns3::WifiMacHeader hdr;
	hdr.SetTypeData();
	hdr.SetAddr1(to);
	hdr.SetAddr2(GetAddress());
	hdr.SetAddr3(GetBssid());
	hdr.SetDsNotFrom();
	hdr.SetDsNotTo();
	CgamemacHeader cgamemacHdr;
	ns3::WifiMacTrailer fcs;
	uint32_t numBytes = packet->GetSize() + cgamemacHdr.GetSerializedSize()
			+ hdr.GetSize() + fcs.GetSerializedSize();
	if (numBytes <= m_maxPacketSize) {
		m_queue->Enqueue(packet, hdr);
	} else {
	}
}

ns3::Mac48Address CgamemacMac::GetBssid(void) const {
	return m_bssid;
}

ns3::Mac48Address CgamemacMac::GetAddress(void) const {
	return m_self;
}

void CgamemacMac::SetBssid(ns3::Mac48Address bssid) {
	m_bssid = bssid;
}

ns3::Ssid CgamemacMac::GetSsid(void) const {
	return m_ssid;
}

void CgamemacMac::SetSsid(ns3::Ssid ssid) {
	m_ssid = ssid;
}

ns3::Time CgamemacMac::GetGuardInterval(void) const {
	return m_guardInterval;
}

void CgamemacMac::SetGuardInterval(const ns3::Time gi) {
	m_guardInterval = gi;
}

void CgamemacMac::ConfigureStandard(enum ns3::WifiPhyStandard standard) {
	switch (standard) {
	case ns3::WIFI_PHY_STANDARD_80211a:
		NS_FATAL_ERROR("80211a not supported in STDMA mode");
		break;
	case ns3::WIFI_PHY_STANDARD_80211b:
		NS_FATAL_ERROR("80211b not supported in STDMA mode");
		break;
	case ns3::WIFI_PHY_STANDARD_80211g:
		NS_FATAL_ERROR("80211g not supported in STDMA mode");
		break;
	case ns3::WIFI_PHY_STANDARD_80211_10MHZ:
		NS_FATAL_ERROR("Please use WIFI_PHY_STANDARD_8011p_CCH instead");
		break;
	case ns3::WIFI_PHY_STANDARD_80211_5MHZ:
		NS_FATAL_ERROR("80211 5Mhz not supported in STDMA mode");
		break;
	case ns3::WIFI_PHY_STANDARD_holland:
		NS_FATAL_ERROR("80211 - holland not supported in STDMA mode");
		break;
	case ns3::WIFI_PHY_STANDARD_80211p_CCH:
		Configure80211p_CCH();
		break;
	case ns3::WIFI_PHY_STANDARD_80211p_SCH:
		Configure80211p_SCH();
		break;
	default:
		NS_ASSERT(false);
		break;
	}
}

void CgamemacMac::Configure80211p_CCH(void) {
}

void CgamemacMac::Configure80211p_SCH(void) {
}

ns3::Time CgamemacMac::GetSlotDuration() {
	NS_LOG_FUNCTION_NOARGS();
	ns3::WifiTxVector txVector(m_wifiMode, 1, 0, false, 1, 1, false);
	return m_phy->CalculateTxDuration(m_maxPacketSize, txVector, m_wifiPreamble)
			+ m_guardInterval;
}

void CgamemacMac::SetForwardUpCallback(
		ns3::Callback<void, ns3::Ptr<ns3::Packet>, ns3::Mac48Address,
				ns3::Mac48Address> upCallback) {
	NS_LOG_FUNCTION(this);
	m_forwardUp = upCallback;
}

void CgamemacMac::SetLinkUpCallback(ns3::Callback<void> linkUp) {
	NS_LOG_FUNCTION(this);
	m_linkUp = linkUp;
}

void CgamemacMac::SetLinkDownCallback(ns3::Callback<void> linkDown) {
	NS_LOG_FUNCTION(this);
	m_linkDown = linkDown;
}

void CgamemacMac::DoTransmit(std::set<uint8_t> imti) {
	CgamemacHeader cgamemacHeader;
	ns3::WifiMacHeader wifiMacHdr;
	ns3::Ptr<ns3::Packet> packet;
	if (m_queue->IsEmpty()) {
		packet = ns3::Create<ns3::Packet>();
		ns3::LlcSnapHeader llc;
		llc.SetType(0x0800);
		packet->AddHeader(llc);
		ns3::Mac48Address to = ns3::Mac48Address::GetBroadcast();
		ns3::WifiMacHeader hdr;
		hdr.SetTypeData();
		hdr.SetAddr1(to);
		hdr.SetAddr2(GetAddress());
		hdr.SetAddr3(GetBssid());
		hdr.SetDsNotFrom();
		hdr.SetDsNotTo();

		m_queue->Enqueue(packet, hdr);

	}
	packet = m_queue->Dequeue(&wifiMacHdr)->Copy();
	ns3::Ptr<ns3::Node> myself = ns3::NodeList::GetNode(
			ns3::Simulator::GetContext());
	ns3::Ptr<ns3::MobilityModel> mobility =
			myself->GetObject<ns3::MobilityModel>();
	ns3::Vector3D position = mobility->GetPosition();
	ns3::Vector3D speed = mobility->GetVelocity();
	NS_ASSERT(position.x < 100000);
	NS_ASSERT(position.y < 100000);
	NS_ASSERT(position.z < 100000);
	NS_ASSERT(position.x > -100000);
	NS_ASSERT(position.y > -100000);
	NS_ASSERT(position.z > -100000);

	NS_ASSERT(speed.x < 100000);
	NS_ASSERT(speed.y < 100000);
	NS_ASSERT(speed.z < 100000);
	NS_ASSERT(speed.x > -100000);
	NS_ASSERT(speed.y > -100000);
	NS_ASSERT(speed.z > -100000);

	cgamemacHeader.setSpeed(speed);
	cgamemacHeader.setPosition(position);
	cgamemacHeader.setImtI(imti);
	cgamemacHeader.setImtSize((uint8_t) imti.size());
	cgamemacHeader.setType(m_type);

	ns3::WifiMacTrailer fcs;
	uint32_t slotBytes = packet->GetSize() + cgamemacHeader.GetSerializedSize()
			+ wifiMacHdr.GetSize() + fcs.GetSerializedSize();
	ns3::WifiTxVector txVector(m_wifiMode, 1, 0, false, 1, 1, false);
	ns3::Time txDuration;

	txDuration = m_phy->CalculateTxDuration(slotBytes, txVector,
			m_wifiPreamble);
	packet->AddHeader(cgamemacHeader);
	packet->AddHeader(wifiMacHdr);
	packet->AddTrailer(fcs);
	if (m_phy->IsStateTx()) {
	}
	NS_ASSERT_MSG(!m_phy->IsStateTx(),
			"cgameMac:DoTransmit() physical layer should not be transmitting already.");
	/*	NS_LOG_DEBUG(ns3::Simulator::Now() << " nodes " << ns3::Simulator::GetContext()
	 << " send packet no." << packet->GetUid());*/

	m_phy->SendPacket(packet, txVector, m_wifiPreamble);
	/*std::cout << "sernder Packet No." << packet->GetUid() << " posiiton ("
			<< cgamemacHeader.getPosition().x << ","
			<< cgamemacHeader.getPosition().y << ","
			<< cgamemacHeader.getPosition().z << ")" << " Speed is :("
			<< cgamemacHeader.getSpeed().x << "," << cgamemacHeader.getSpeed().y
			<< "," << cgamemacHeader.getSpeed().z << ")" << "\n";*/
	m_txTrace(ns3::Simulator::GetContext(), packet, imti, m_alphai, m_type);

	NS_ASSERT(m_phy->IsStateTx());

}

void CgamemacMac::Receive(ns3::Ptr<ns3::Packet> packet, double rxSnr,
		ns3::WifiMode txMode, ns3::WifiPreamble preamble) {
	NS_LOG_FUNCTION(this << packet << rxSnr << txMode << preamble);

// Step 1: remove WifiMac header and FCS trailer
	ns3::WifiMacHeader wifiMacHdr;
	packet->RemoveHeader(wifiMacHdr);
	ns3::WifiMacTrailer fcs;
	packet->RemoveTrailer(fcs);

// Step 2: continue depending on the type of packet that has been received
	if (wifiMacHdr.IsMgt()) {
		NS_FATAL_ERROR(
				"cgameMac:Receive() " << m_self << " received a management frame, which is unexpected.");
	}
	if (!wifiMacHdr.GetAddr1().IsGroup()) {
		NS_FATAL_ERROR(
				"cgameMac:Receive() " << m_self << " received a unicast packet, but all packets should be delivered to broadcast addresses.");
	}
	if (!wifiMacHdr.IsData()) {
		NS_FATAL_ERROR(
				"cgameMac:Receive() " << m_self << " the packet received was not data nor management, which is unexpected.");
	} else {
		// Try to decode the cfrmacHeader...
		CgamemacHeader cgamemacHeader;
		packet->RemoveHeader(cgamemacHeader);

		if (cgamemacHeader.GetTypeId() != CgamemacHeader::GetTypeId()) {
			NS_FATAL_ERROR(
					"cgameMac:Receive() " << m_self << " the packet received did not contain a cfrmac header, which is unexpected.");
		} else {
			ns3::Ptr<cgame::cgameWhetherReceive> mmcgameWhetherReceive =
					ns3::Create<cgame::cgameWhetherReceive>();

			std::set<uint8_t> m_alphaiTemp;

			m_alphaiTemp =
					mmcgameWhetherReceive->WhetherReceive(
							cgamemacHeader.getPosition(),
							cgamemacHeader.getImtI(),
							ns3::NodeList::GetNode(ns3::Simulator::GetContext())->GetObject<
									ns3::MobilityModel>()->GetPosition(),
							m_alphai, cgamemacHeader.getType(), m_type);
			NS_ASSERT(m_alphaiTemp.size() - m_alphai.size() >= 0);
			m_rxTrace(ns3::Simulator::GetContext(), cgamemacHeader.getType(),
					cgamemacHeader.getImtI(),
					m_alphaiTemp.size() - m_alphai.size(), m_type);
			m_alphai = m_alphaiTemp;

			/*std::cout << " receiver  Packet No." << packet->GetUid()
					<< " posiiton (" << cgamemacHeader.getPosition().x << ","
					<< cgamemacHeader.getPosition().y << ","
					<< cgamemacHeader.getPosition().z << ")" << " Speed is :("
					<< cgamemacHeader.getSpeed().x << ","
					<< cgamemacHeader.getSpeed().y << ","
					<< cgamemacHeader.getSpeed().z << ")" << "\n";*/

			m_betai.clear();
			for (uint8_t i = 0; i < m_numdataitems; i++) {
				if (m_alphai.find(i) == m_alphai.end()) {
					m_betai.insert(i);
				}
			}

			NS_ASSERT(m_betai.size() + m_alphai.size() == m_numdataitems);

		}

	}
}
void CgamemacMac::StartInitializationPhase() {
	NS_ASSERT_MSG(1 == 0, "CgamemacMac::StartInitializationPhase");

}

void CgamemacMac::StartInitializationPhaseRsuAndObu(uint8_t type) {
	m_type = type;
	ns3::Simulator::Schedule(m_begintime,
			&CgamemacMac::StartInitializationmBeginTime, this);
}

void CgamemacMac::StartInitializationmBeginTime() {

	InitializeAlphaBeta();
	GraphUpLoad();

}

void CgamemacMac::InitializeAlphaBeta() {
	m_alphai.clear();
	m_betai.clear();
	if (m_type == 0) {
		ns3::Ptr<ns3::UniformRandomVariable> uv = ns3::CreateObject<
				ns3::UniformRandomVariable>();
		while (m_alphai.size() < m_initializedNumber) {
			double selected = uv->GetValue(0, m_numdataitems);
			m_alphai.insert((uint8_t) selected);

		}

		for (uint8_t i = 0; i < m_numdataitems; i++) {
			if (m_alphai.find(i) == m_alphai.end()) {
				m_betai.insert(i);
			}
		}
		NS_ASSERT(m_betai.size() + m_alphai.size() == m_numdataitems);
	} else if (m_type == 1) {
		for (uint8_t i = 0; i < m_numdataitems; i++) {

			m_alphai.insert(i);

		}
		NS_ASSERT(m_betai.size() + m_alphai.size() == m_numdataitems);
	} else {
		NS_ASSERT(m_type <= 1);
	}
}
void CgamemacMac::GraphUpLoad() {

	ns3::Ptr<ns3::Node> myself = ns3::NodeList::GetNode(
			ns3::Simulator::GetContext());
	ns3::Ptr<ns3::MobilityModel> mobility =
			myself->GetObject<ns3::MobilityModel>();
	/*std::stringstream ss;
	 for (uint32_t i = 0; i < m_alphai.size(); i++) {
	 ss << i;
	 }
	 NS_LOG_DEBUG(
	 " upload: " << ns3::Simulator::Now() << ",node:" << ns3::Simulator::GetContext() << " at the position of (" << mobility->GetPosition().x << "," << mobility->GetPosition().y << "," << mobility->GetPosition().z << ")" << " Vehicle Type:" <<(int)m_type<< " cached packets:=("<<ss.str()<<")");
	 */
	/*std::cout << "Before ns3::Create\n";

	 std::cout << "Node Id:" << ns3::Simulator::GetContext() << " Position:"
	 << mobility->GetPosition().x << " mobility->GetVelocity():"
	 << mobility->GetVelocity().x << " m_alphai:" << m_alphai.size()
	 << " m_betai" << m_betai.size() << "\n";*/

	m_upload->setNodeId(ns3::Simulator::GetContext());
	m_upload->setPosition(mobility->GetPosition());
	m_upload->setSpeed(mobility->GetVelocity());
	m_upload->setAlphai(m_alphai);
	m_upload->setBetai(m_betai);

	m_upload->setRsuOrObu(m_type); //1 means for RSU and 0 means for OBU.

	m_GraphUpLoadcallack(m_upload);

	ns3::Simulator::Schedule(m_schedulingPeriod, &CgamemacMac::GraphUpLoad,
			this);

}
void CgamemacMac::SetGraphUpLoadCallback(
		ns3::Callback<void, ns3::Ptr<cgame::GraphUpLoad> > upcallack) {
	m_GraphUpLoadcallack = upcallack;
}
void CgamemacMac::SendtoMacLayer(ns3::Ptr<cgame::cgameDownload> download) {
	// received packet from edge cloud.

	NS_ASSERT(download->getNodeId() == ns3::Simulator::GetContext());
	DoTransmit(download->getImti());

}

} // namespace cgamemac
