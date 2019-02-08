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

#ifndef CGAME_CLASS_H
#define CGAME_CLASS_H

#include "ns3/object.h"
#include "ns3/buffer.h"
#include "ns3/vector.h"
#include "ns3/ptr.h"
#include "ns3/core-module.h"
#include "ns3/nstime.h"
#include <deque>
namespace cgame {

class cgameWhetherReceive: public ns3::Object {
public:
	uint32_t m_maximumChannel;
	uint32_t m_communcationRange;

	static ns3::TypeId
	GetTypeId(void);
	cgameWhetherReceive();
	cgameWhetherReceive(uint32_t maximumChannel, uint32_t communcationRange);
	std::set<uint8_t> WhetherReceive(ns3::Vector3D senderPosition,
			std::set<uint8_t> imti, ns3::Vector3D receiverPosition,
			std::set<uint8_t> dataitemsInNeed, uint8_t senderType,
			uint8_t receiverType);

	double CalDistance(ns3::Vector3D senderposition,
			ns3::Vector3D receiverposition);
	uint32_t ChannelCapacity(ns3::Vector3D senderposition, uint8_t senderType,
			ns3::Vector3D receiverposition, uint8_t receiverType);

private:
	ns3::Vector3D m_senderPosition;
	std::set<uint8_t> m_imti;
	ns3::Vector3D m_receiverPosition;
	std::set<uint8_t> m_dataitemsInNeed;

};

class cgameDownload: public ns3::Object {
public:

	static ns3::TypeId
	GetTypeId(void);
	cgameDownload();
	cgameDownload(uint32_t m_nodeId);
	cgameDownload(uint32_t m_nodeId, std::set<uint8_t> imti);
	const std::set<uint8_t>& getImti() const;
	void setImti(const std::set<uint8_t>& imti);
	uint32_t getNodeId() const;
	void setNodeId(uint32_t nodeId);
	void print();

private:
	uint32_t m_nodeId;
	std::set<uint8_t> m_imti;

};

class GraphUpLoad: public ns3::Object {

public:
	static ns3::TypeId
	GetTypeId(void);
	GraphUpLoad();
	~GraphUpLoad();
	GraphUpLoad(uint32_t nodeId, ns3::Vector3D position, ns3::Vector3D speed,
			std::set<uint8_t> alphai, std::set<uint8_t> betai);
	const std::set<uint8_t>& getAlphai() const;
	void setAlphai(const std::set<uint8_t>& alphai);
	const std::set<uint8_t>& getBetai() const;
	void setBetai(const std::set<uint8_t>& betai);
	uint32_t getNodeId() const;
	void setNodeId(uint32_t nodeId);
	const ns3::Vector3D& getPosition() const;
	void setPosition(const ns3::Vector3D& position);
	const ns3::Vector3D& getSpeed() const;
	void setSpeed(const ns3::Vector3D& speed);
	uint8_t getRsuOrObu() const;
	void setRsuOrObu(uint8_t rsuOrObu);
	void print();
private:

	uint32_t m_nodeId;
	ns3::Vector3D m_position;
	ns3::Vector3D m_speed;
	std::set<uint8_t> m_alphai;
	std::set<uint8_t> m_betai;
	uint8_t m_rsuOrObu; //0 means for RSU and 1 means for OBU.

	////////////////////////

};

}

#endif
