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

#ifndef CGAMEMAC_MAC_H
#define CGAMEMAC_MAC_H

#include "ns3/packet.h"
#include "ns3/mac48-address.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/event-id.h"
#include "ns3/random-variable.h"
#include "ns3/vector.h"
#include "ns3/wifi-mac-queue.h"
#include "ns3/cgame-class.h"
#include "ns3/wifi-phy.h"
#include "ns3/ssid.h"
#include "ns3/qos-utils.h"

namespace cgamemac {

class CgamemacMac: public ns3::Object {

public:
	static ns3::TypeId
	GetTypeId(void);

	CgamemacMac();
	virtual ~CgamemacMac();

	ns3::Time
	GetGuardInterval(void) const;

	void
	SetGuardInterval(const ns3::Time gi);

	void
	SetAddress(ns3::Mac48Address address);

	ns3::Mac48Address
	GetAddress(void) const;

	void
	SetSsid(ns3::Ssid ssid);

	ns3::Ssid
	GetSsid(void) const;

	void
	SetBssid(ns3::Mac48Address bssid);

	ns3::Mac48Address
	GetBssid(void) const;

	/**
	 * Is supposed to enqueue a unicast packet into the transmission queue, but as this MAC implementation
	 * supports only broadcast packets, the packets is simply discarded/ignored.
	 *
	 * \param packet The packet that should be transmitted
	 * \param to The destination MAC address of the packet
	 * \param from The source MAC address of this packet
	 */
	void
	Enqueue(ns3::Ptr<const ns3::Packet> packet, ns3::Mac48Address to,
			ns3::Mac48Address from);

	/**
	 * Enqueues a broadcast packet into the transmission queue. This packet will be transmitted in one
	 * of the slots reserved by CGAMEMAC.
	 *
	 * \param packet The packet that should be transmitted
	 * \param to The destination MAC address of the packet
	 */
	void
	Enqueue(ns3::Ptr<const ns3::Packet> packet, ns3::Mac48Address to);

	/**
	 * This method starts up the CGAMEMAC logic and triggers the start of the initialization phase
	 * as defined in the standard. It is typically scheduled by CgamemacHelper::Install when connecting
	 * all protocol layers and network devices together.
	 */
	void
	StartInitializationPhase();
	void
	StartInitializationmBeginTime();
	void
	StartInitializationPhaseRsuAndObu(uint8_t type);

	/**
	 * Will always return false in the current implementation as the feature is not supported.
	 */
	bool
	SupportsSendFrom(void) const;

	void
	SetWifiPhy(ns3::Ptr<ns3::WifiPhy> phy);

	/**
	 * Sets the callback function exposed by the layer to which received packets shall be forwarded. This is typically
	 * a CgamemacNetDevice and attached by the WifiPhyHelper.
	 *
	 * \param upCallback The reference to the object and the method to call whenever a packet is successfully received
	 */
	void
	SetForwardUpCallback(
			ns3::Callback<void, ns3::Ptr<ns3::Packet>, ns3::Mac48Address,
					ns3::Mac48Address> upCallback);

	/**
	 * Sets the callback function exposed by the layer to which a notification shall be sent whenever the link is up.
	 * This is typically a CgamemacNetDevice and attached by the WifiPhyHelper.
	 *
	 * \param linkUp The reference to the object and the method to call whenever a "link" is established
	 */
	void
	SetLinkUpCallback(ns3::Callback<void> linkUp);

	/**
	 * Sets the callback function exposed by the layer to which a notification shall be sent whenever the link is down.
	 * This is typically a CgamemacNetDevice and attached by the WifiPhyHelper.
	 *
	 * \param linkUp The reference to the object and the method to call whenever the "link" gets down.
	 */
	void
	SetLinkDownCallback(ns3::Callback<void> linkDown);

	/**
	 * This method is called whenever the physical layer successfully receives a message and passes it up to
	 * the medium access control layer.
	 *
	 * \param packet	The packet which was successfully received by the physical layer
	 * \param rxSnr		The signal-to-noise ratio (SNR) of the received packet
	 * \param txMode	The WIFI mode that was used to transmit the packet
	 * \param preamble	The type of preamble used by the transmitter for this packet
	 */
	void
	Receive(ns3::Ptr<ns3::Packet> packet, double rxSnr, ns3::WifiMode txMode,
			ns3::WifiPreamble preamble);

	void
	ConfigureStandard(enum ns3::WifiPhyStandard standard);
	void DoTransmit(std::set<uint8_t> imti);

	void SetGraphUpLoadCallback(
			ns3::Callback<void, ns3::Ptr<cgame::GraphUpLoad> > upcallack);
	void GraphUpLoad();
	void SendtoMacLayer(ns3::Ptr<cgame::cgameDownload> download);

	bool containTheAds(uint16_t content);

protected:

	/**
	 * This method takes care of packet forwarding to higher layers and encapsulates all details
	 * that go along with this activity.
	 */
	void
	ForwardUp(ns3::Ptr<ns3::Packet> packet, ns3::Mac48Address from,
			ns3::Mac48Address to);

	ns3::Ptr<ns3::WifiPhy> m_phy;
	ns3::Ptr<ns3::WifiMacQueue> m_queue;

	ns3::Callback<void, ns3::Ptr<ns3::Packet>, ns3::Mac48Address,
			ns3::Mac48Address> m_forwardUp;
	ns3::Callback<void> m_linkUp;
	ns3::Callback<void> m_linkDown;
	ns3::Mac48Address m_self;
	ns3::Mac48Address m_bssid;
	ns3::Ssid m_ssid;

private:

	void
	Configure80211p_CCH(void);
	void
	Configure80211p_SCH(void);

	ns3::Time
	GetSlotDuration();
	uint8_t getCurrentSlot();
	void InitializeAlphaBeta();

	uint8_t m_numdataitems;
	uint8_t m_initializedNumber;
	uint8_t isInilitialized;
	uint8_t m_type; //0 means for RSU and 1 means for OBU.
	ns3::Time m_begintime;
	ns3::Time m_schedulingPeriod;
	ns3::Time m_guardInterval;
	std::set<uint8_t> m_alphai;
	std::set<uint8_t> m_betai;
	ns3::WifiPreamble m_wifiPreamble;
	ns3::WifiMode m_wifiMode;
	uint32_t m_maxPacketSize;
	ns3::EventId m_nextTransmissionEvent;
	uint8_t m_selfSlotIndex;
	ns3::UniformVariable m_uniRanVar;
	ns3::Callback<void, ns3::Ptr<cgame::GraphUpLoad> > m_GraphUpLoadcallack;

	ns3::TracedCallback<uint32_t, ns3::Ptr<const ns3::Packet>,
			std::set<uint8_t>, std::set<uint8_t>, uint8_t> m_txTrace; //<id, packet,sendDataItems, CachedDataItems, m_type>
	ns3::TracedCallback<uint32_t, uint8_t, std::set<uint8_t>,uint32_t,
			uint8_t> m_rxTrace; //<id, packet,ReceivedDataItems, CachedDataItems, m_type>
	ns3::Ptr<cgame::GraphUpLoad> m_upload;
};

} // namespace cgamemac

#endif /* CGAMEMAC_MAC_H */
