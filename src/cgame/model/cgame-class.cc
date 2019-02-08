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

#include "cgame-class.h"
#include "cgame-inout-greedy.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("cgameClass");

namespace cgame {

ns3::TypeId cgameWhetherReceive::GetTypeId(void) {
	ns3::TypeId tid = ns3::TypeId("cgame::cgameWhetherReceive").SetParent<
			ns3::Object>().AddConstructor<cgame::cgameWhetherReceive>();
	return tid;
}
cgameWhetherReceive::cgameWhetherReceive() {

	ns3::IntegerValue abc1, abc2;
	ns3::GlobalValue::GetValueByName("m_communcationRange", abc1);
	ns3::GlobalValue::GetValueByName("m_maximumChannel", abc2);
	m_communcationRange = abc1.Get();
	m_maximumChannel = abc2.Get();

}
cgameWhetherReceive::cgameWhetherReceive(uint32_t maximumChannel,
		uint32_t communcationRange) {
	ns3::IntegerValue abc1, abc2;
	ns3::GlobalValue::GetValueByName("m_communcationRange", abc1);
	ns3::GlobalValue::GetValueByName("m_maximumChannel", abc2);
	m_communcationRange = abc1.Get();
	m_maximumChannel = abc2.Get();

}

double cgameWhetherReceive::CalDistance(ns3::Vector3D senderposition,
		ns3::Vector3D receiverposition) {
	return std::sqrt(
			std::pow(senderposition.x - receiverposition.x, 2)
					+ std::pow(senderposition.y - receiverposition.y, 2)
					+ std::pow(senderposition.z - receiverposition.z, 2));

}
uint32_t cgameWhetherReceive::ChannelCapacity(ns3::Vector3D senderposition,
		uint8_t senderType, ns3::Vector3D receiverposition, //1 means for RSU and 0 means for OBU.
		uint8_t receiverType) {

	double dist = CalDistance(senderposition, receiverposition);

	if (senderType == 1 && receiverType == 1) {
		return 0;
	} else if (senderType + receiverType <= 1) {

		uint32_t i = m_maximumChannel;

		double beginDistance = m_communcationRange / m_maximumChannel;
		if (dist > m_communcationRange) {
			return 0;
		}
		while (beginDistance < dist) {
			i--;
			beginDistance = beginDistance
					+ (double) m_communcationRange / m_maximumChannel;

			if (i > m_maximumChannel) {
				std::cout << " sender Position is :(" << senderposition.x << ","
						<< senderposition.y << "," << senderposition.z << ")"
						<< " reciever Position is :(" << receiverposition.x
						<< "," << receiverposition.y << ","
						<< receiverposition.z << ")";
				std::cout << "distance is " << dist << "\n";

				NS_ASSERT_MSG(i <= m_maximumChannel, "");
			}

		}
		NS_ASSERT_MSG(i >= 0, "");

		return i;

	} else {
		NS_ASSERT_MSG(1 == 0,
				"in cgameinout::ChannelCapacity,  mtype can only be 0 or 1");
	}
	return 0;
}

std::set<uint8_t> cgameWhetherReceive::WhetherReceive(
		ns3::Vector3D senderPosition, std::set<uint8_t> imti,
		ns3::Vector3D receiverPosition, std::set<uint8_t> cacheddata,
		uint8_t senderType, uint8_t receiverType) {
	uint32_t capacity = ChannelCapacity(senderPosition, senderType,
			receiverPosition, receiverType);
	/*	std::cout << " senderPosition (" << senderPosition.x << ","
	 << senderPosition.y << "," << senderPosition.z << ")"
	 << " receiverPosition is :(" << receiverPosition.x << ","
	 << receiverPosition.y << "," << receiverPosition.z << ")" << "\n";

	 std::cout << "channel capacity is: " << capacity;*/
	if (imti.size() > capacity) {
		//std::cout << "  no Add\n";
		return cacheddata;
	} else {
		for (std::set<uint8_t>::iterator it = imti.begin(); it != imti.end();
				it++) {
			cacheddata.insert(*it);

		}
		//std::cout << "  Add\n";
		return cacheddata;
	}

}

ns3::TypeId GraphUpLoad::GetTypeId(void) {
	ns3::TypeId tid =
			ns3::TypeId("cgame::GraphUpLoad").SetParent<ns3::Object>().AddConstructor<
					cgame::GraphUpLoad>();
	return tid;
}
GraphUpLoad::GraphUpLoad() {

}
GraphUpLoad::~GraphUpLoad() {

}
GraphUpLoad::GraphUpLoad(uint32_t nodeId, ns3::Vector3D position,
		ns3::Vector3D speed, std::set<uint8_t> alphai,
		std::set<uint8_t> betai) {

	this->m_nodeId = nodeId;
	this->m_position = position;
	this->m_speed = speed;
	this->m_alphai = alphai;
	this->m_betai = betai;
}

ns3::TypeId cgameDownload::GetTypeId(void) {
	ns3::TypeId tid =
			ns3::TypeId("cgame::cgameDownload").SetParent<ns3::Object>().AddConstructor<
					cgame::cgameDownload>();
	return tid;
}
cgameDownload::cgameDownload() {
	m_nodeId = 0;

}
cgameDownload::cgameDownload(uint32_t nodeId) {
	m_nodeId = nodeId;
}
cgameDownload::cgameDownload(uint32_t nodeId, std::set<uint8_t> imti) {
	m_nodeId = nodeId;
	m_imti = imti;

}

const std::set<uint8_t>& GraphUpLoad::getAlphai() const {
	return m_alphai;
}

void GraphUpLoad::setAlphai(const std::set<uint8_t>& alphai) {
	this->m_alphai = alphai;
}

const std::set<uint8_t>& GraphUpLoad::getBetai() const {
	return m_betai;
}

void GraphUpLoad::print() {
	std::cout << "  Nodeid:" << m_nodeId << " m_type:" << (int) m_rsuOrObu
			<< " Position=(" << m_position.x << "," << m_position.y << ","
			<< m_position.z << ",)" << "m_alphai=(";

	for (std::set<uint8_t>::iterator it = m_alphai.begin();
			it != m_alphai.end(); it++) {
		std::cout << (int) *it << ",";
	}
	std::cout << ")" << "m_betai=(";

	for (std::set<uint8_t>::iterator it = m_betai.begin(); it != m_betai.end();
			it++) {
		std::cout << (int) *it << ",";
	}
	std::cout << ")\n";
}
void GraphUpLoad::setBetai(const std::set<uint8_t>& betai) {
	this->m_betai = betai;
}

uint32_t GraphUpLoad::getNodeId() const {
	return m_nodeId;
}

void GraphUpLoad::setNodeId(uint32_t nodeId) {
	m_nodeId = nodeId;
}

const ns3::Vector3D& GraphUpLoad::getPosition() const {
	return m_position;
}

void GraphUpLoad::setPosition(const ns3::Vector3D& position) {
	m_position = position;
}

const ns3::Vector3D& GraphUpLoad::getSpeed() const {
	return m_speed;
}

void GraphUpLoad::setSpeed(const ns3::Vector3D& speed) {
	m_speed = speed;
}

uint8_t GraphUpLoad::getRsuOrObu() const {
	return m_rsuOrObu;
}

void GraphUpLoad::setRsuOrObu(uint8_t rsuOrObu) {
	m_rsuOrObu = rsuOrObu;
}

} // namespace cgame

const std::set<uint8_t>& cgame::cgameDownload::getImti() const {
	return m_imti;
}

void cgame::cgameDownload::setImti(const std::set<uint8_t>& imti) {
	this->m_imti = imti;
}

uint32_t cgame::cgameDownload::getNodeId() const {
	return m_nodeId;
}

void cgame::cgameDownload::setNodeId(uint32_t nodeId) {
	m_nodeId = nodeId;
}
void cgame::cgameDownload::print() {
	std::cout << "Nodeid:" << m_nodeId << "m_imti=(";

	for (std::set<uint8_t>::iterator it = m_imti.begin(); it != m_imti.end();
			it++) {
		std::cout << (int) *it << ",";
	}
	std::cout << "\n";
}
