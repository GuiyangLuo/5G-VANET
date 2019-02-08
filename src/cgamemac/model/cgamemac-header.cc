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

#include "cgamemac-header.h"
#include "ns3/log.h"
#include "ns3/cgame-class.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/uinteger.h"
NS_LOG_COMPONENT_DEFINE("CgamemacHeader");

namespace cgamemac {

ns3::TypeId CgamemacHeader::GetTypeId(void) {
	static ns3::TypeId tid = ns3::TypeId("cgamemac::CgamemacHeader").SetParent<
			ns3::Header>().AddConstructor<CgamemacHeader>();
	return tid;
}

CgamemacHeader::CgamemacHeader() :
		m_speed(ns3::Vector3D(0, 0, 0)), m_position(ns3::Vector3D(0, 0, 0)) {

}

CgamemacHeader::~CgamemacHeader() {
}

ns3::TypeId CgamemacHeader::GetInstanceTypeId(void) const {
	return GetTypeId();
}
void CgamemacHeader::Print(std::ostream &os) const {

}
uint32_t CgamemacHeader::GetSerializedSize(void) const {
	return 8 * 3 * 2 + imt_i.size() + 2 + 6;
}
void CgamemacHeader::print() {
	std::cout << "CgamemacHeader::print() m_type=" << (int) m_type
			<< " imti size=" << imt_i.size() << "transmitted data:";

	for (uint32_t i = 0; i < imt_i.size(); i++) {
		std::cout << i;
	}
	std::cout << std::endl;
}

void CgamemacHeader::Serialize(ns3::Buffer::Iterator start) const {
	ns3::Buffer::Iterator i = start;

	uint8_t fuhao = m_speed.x >= 0 ? 1 : 0;
	i.WriteU8((uint8_t) fuhao);
	i.WriteHtolsbU64((uint64_t) abs(1000 * m_speed.x));
	fuhao = m_speed.y >= 0 ? 1 : 0;
	i.WriteU8((uint8_t) fuhao);
	i.WriteHtolsbU64((uint64_t) abs(1000 * m_speed.y));
	fuhao = m_speed.z >= 0 ? 1 : 0;
	i.WriteU8((uint8_t) fuhao);
	i.WriteHtolsbU64((uint64_t) abs(1000 * m_speed.z));

	fuhao = m_position.x >= 0 ? 1 : 0;
	i.WriteU8((uint8_t) fuhao);
	i.WriteHtolsbU64((uint64_t) abs(1000 * m_position.x));

	fuhao = m_position.y >= 0 ? 1 : 0;
	i.WriteU8((uint8_t) fuhao);
	i.WriteHtolsbU64((uint64_t) abs(1000 * m_position.y));

	fuhao = m_position.z >= 0 ? 1 : 0;
	i.WriteU8((uint8_t) fuhao);
	i.WriteHtolsbU64((uint64_t) abs(1000 * m_position.z));
	i.WriteU8((uint8_t) m_type);
	i.WriteU8((uint8_t) imt_i.size());

	std::set<uint8_t>::iterator iter;

	for (iter = imt_i.begin(); iter != imt_i.end(); iter++) {
		i.WriteU8((uint8_t) *iter);

	}

}

uint32_t CgamemacHeader::Deserialize(ns3::Buffer::Iterator start) {
	uint8_t fuhao = start.ReadU8();
	uint64_t value = start.ReadLsbtohU64();
	m_speed.x = value;
	m_speed.x = m_speed.x / 1000;
	if (fuhao == 0) {
		m_speed.x = m_speed.x * -1;
	}

	fuhao = start.ReadU8();
	value = start.ReadLsbtohU64();
	m_speed.y = value;
	m_speed.y = m_speed.y / 1000;
	if (fuhao == 0) {
		m_speed.y = m_speed.y * -1;
	}

	fuhao = start.ReadU8();
	value = start.ReadLsbtohU64();
	m_speed.z = value;
	m_speed.z = m_speed.z / 1000;
	if (fuhao == 0) {
		m_speed.z = m_speed.z * -1;
	}

	fuhao = start.ReadU8();
	value = start.ReadLsbtohU64();
	m_position.x = value;
	m_position.x = m_position.x / 1000;
	if (fuhao == 0) {
		m_position.x = m_position.x * -1;
	}

	fuhao = start.ReadU8();
	value = start.ReadLsbtohU64();
	m_position.y = value;
	m_position.y = m_position.y / 1000;
	if (fuhao == 0) {
		m_position.y = m_position.y * -1;
	}

	fuhao = start.ReadU8();
	value = start.ReadLsbtohU64();
	m_position.z = m_position.z / 1000;
	if (fuhao == 0) {
		m_position.z = m_position.z * -1;
	}

	m_type = start.ReadU8();
	imtSize = start.ReadU8();
	for (uint8_t i = 0; i < imtSize; i++)
		imt_i.insert(start.ReadU8());
	NS_ASSERT(imtSize == imt_i.size());

	return 8 * 3 * 2 + imt_i.size() + 2 + 6;

}

const ns3::Vector3D& CgamemacHeader::getPosition() const {
	return m_position;
}

void CgamemacHeader::setPosition(const ns3::Vector3D& position) {
	m_position = position;
}

const ns3::Vector3D& CgamemacHeader::getSpeed() const {
	return m_speed;
}

void CgamemacHeader::setSpeed(const ns3::Vector3D& speed) {
	m_speed = speed;
}

const std::set<uint8_t>& CgamemacHeader::getImtI() const {
	return imt_i;
}

void CgamemacHeader::setImtI(const std::set<uint8_t>& imtI) {
	imt_i = imtI;
}

uint8_t CgamemacHeader::getImtSize() const {
	return imtSize;
}

void CgamemacHeader::setImtSize(uint8_t imtSize) {
	this->imtSize = imtSize;
}

uint8_t CgamemacHeader::getType() const {
	return m_type;
}

void CgamemacHeader::setType(uint8_t type) {
	m_type = type;
}

} // namespace cgamemac
