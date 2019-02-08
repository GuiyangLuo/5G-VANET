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

#ifndef CGAMEMAC_HEADER_H_
#define CGAMEMAC_HEADER_H_

#include "ns3/header.h"
#include "ns3/buffer.h"
#include "ns3/cgame-class.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"
namespace cgamemac {

/**
 * \brief Defines the contents of a CGAMEMAC packet header
 *
 * This class defines the contents of a CGAMEMAC packet header, which includes GPS coordinates
 * (latitude and longitude), offset information to the next transmission slot to be used, and
 * the timeout value of the reservation duration.
 *
 * \ingroup cgamemac
 */

class CgamemacHeader: public ns3::Header {

public:

	CgamemacHeader();
	~CgamemacHeader();
	static ns3::TypeId GetTypeId(void);
	virtual void Print(std::ostream &os) const;
	virtual ns3::TypeId GetInstanceTypeId(void) const;
	virtual uint32_t GetSerializedSize(void) const;
	virtual void Serialize(ns3::Buffer::Iterator start) const;
	virtual uint32_t Deserialize(ns3::Buffer::Iterator start);
	const ns3::Vector3D& getPosition() const;
	void setPosition(const ns3::Vector3D& position);
	const ns3::Vector3D& getSpeed() const;
	void setSpeed(const ns3::Vector3D& speed);
	const std::set<uint8_t>& getImtI() const;
	void setImtI(const std::set<uint8_t>& imtI);
	uint8_t getImtSize() const;
	void setImtSize(uint8_t imtSize);
	uint8_t getType() const;
	void setType(uint8_t type);
	void print() ;
private:

	ns3::Vector3D m_speed;
	ns3::Vector3D m_position;
	uint8_t imtSize;
	std::set<uint8_t> imt_i;
	uint8_t m_type;


};

} // namespace cgamemac

#endif /* CGAMEMAC_HEADER_H_ */
