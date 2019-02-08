

#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/node-list.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-address.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/network-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/onoff-application.h"
#include "ns3/cgame-module.h"
#include "ns3/cgamemac-module.h"
#include "ns3/cgamemac-header.h"
#include "ns3/packet.h"
#include <string.h>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <numeric>
#include <string>
#include "ns3/vector.h"
#include "GraphCensus.h"
using namespace ns3;

#define COMRANGE_RSU  50
#define COMRANGE_OBU  100
#define NODENUMBER_RSU  10

#define SchedulingPeriod 1
#define BeginTime 10
#define EndTime 200
#define  NumberOfNeighor 10
#define MaximumCapacity 20
#define TotalNumberOfDataItems 100
#define InitializedNumberOfDataItems 5

class Experiment {
private:
	ns3::Ptr<GraphCensus> sta;
	std::string CurrentAlgorithms;
	double censusTime;

public:
	Experiment() {
		ns3::GlobalValue m_communcationRange("m_communcationRange",
				"The global seed of all rng streams",
				ns3::IntegerValue(COMRANGE_RSU),
				ns3::MakeIntegerChecker<uint32_t>());

		ns3::GlobalValue m_maximumChannel("m_maximumChannel",
				"The global seed of all rng streams",
				ns3::IntegerValue(MaximumCapacity),
				ns3::MakeIntegerChecker<uint32_t>());

		std::vector<int> TclNumber;

		CurrentAlgorithms = "greedy";


		TclNumber.push_back(2);
		TclNumber.push_back(4);


		for (std::vector<int>::iterator i = TclNumber.begin();
				i != TclNumber.end(); i++) {

			SeedManager::SetSeed(10); // Changes seed from default of 1 to 3
			SeedManager::SetRun(*i);
			sta = ns3::Create<GraphCensus>(TotalNumberOfDataItems,
			InitializedNumberOfDataItems, SchedulingPeriod,
					(*i) * InitializedNumberOfDataItems
							+ NODENUMBER_RSU * TotalNumberOfDataItems);
			sta->setBETime(BeginTime, EndTime);
			std::stringstream extratemp;
			std::string extra;
			extratemp << NumberOfNeighor;
			extratemp << "_";
			extratemp << MaximumCapacity;
			extratemp << "_";
			extratemp << TotalNumberOfDataItems;
			extratemp << "_";
			extratemp << InitializedNumberOfDataItems;
			extratemp >> extra;
			std::stringstream ss1;
			std::string ss2;
			ss1 << CurrentAlgorithms;
			ss1 << "_";
			ss1 << extra.data();
			ss1 >> ss2;
			sta->OpenOutPut(*i + NODENUMBER_RSU, ss2.data());
			std::cout << " Algotirhm is " << CurrentAlgorithms
					<< ", The number of vehicles=" << *i << "\n\n";
			Run(NODENUMBER_RSU, *i, extra.data());
			sta->CloseFile();
		}

	}

	void Run(uint32_t numberOfRsu, uint32_t numberOfObu, std::string extra) {

		cgamemac::CgamemacHelper cgamem;
		cgamem.SetStandard(ns3::WIFI_PHY_STANDARD_80211p_CCH);
		cgamemac::CgamemacMacHelper cgamemacMac =
				cgamemac::CgamemacMacHelper::Default();
		bool is_log = false;
		if (is_log) {
			LogComponentEnable("CgamemacMac", LOG_LEVEL_INFO);
			LogComponentEnable("CgamemacHeader", LOG_LEVEL_INFO);
			LogComponentEnable("YansWifiPhy", LOG_LEVEL_INFO);
		}

		// Create network nodes
		ns3::NodeContainer m_nodesRsu, m_nodesObu;
		m_nodesRsu.Create(numberOfRsu);
		m_nodesObu.Create(numberOfObu);

		/*	ns3::Config::SetDefault("cgame::cgameinoutgreedy::m_NeighborNumber",
		 ns3::UintegerValue(NumberOfNeighor));
		 ns3::Config::SetDefault("cgame::cgameinoutlinerpr::m_NeighborNumber",
		 ns3::UintegerValue(NumberOfNeighor));
		 ns3::Config::SetDefault("cgame::cgameinoutrandomly::m_NeighborNumber",
		 ns3::UintegerValue(NumberOfNeighor));*/

		ns3::Config::SetDefault("cgamemac::CgamemacMac::MaximumPacketSize",
				ns3::UintegerValue(453));
		ns3::Config::SetDefault("cgamemac::CgamemacMac::m_begintime",
				ns3::TimeValue(ns3::Seconds(BeginTime)));
		ns3::Config::SetDefault("cgamemac::CgamemacMac::m_schedulingPeriod",
				ns3::TimeValue(ns3::Seconds(SchedulingPeriod)));
		ns3::Config::SetDefault("cgamemac::CgamemacMac::m_initializedNumber",
				ns3::UintegerValue(InitializedNumberOfDataItems));
		ns3::Config::SetDefault("cgamemac::CgamemacMac::m_numdataitems",
				ns3::UintegerValue(TotalNumberOfDataItems));
		ns3::Config::SetDefault("cgamemac::CgamemacMac::GuardInterval",
				ns3::TimeValue(ns3::MicroSeconds(6)));
		ns3::Config::SetDefault("ns3::YansWifiChannel::m_distancecommunication",
				ns3::UintegerValue(COMRANGE_RSU));

		// Configure the wireless channel characteristics
		ns3::YansWifiChannelHelper wifiChannel;
		wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel");
		ns3::Config::SetDefault(
				"ns3::LogDistancePropagationLossModel::Exponent",
				ns3::DoubleValue(1.85));
		ns3::Config::SetDefault(
				"ns3::LogDistancePropagationLossModel::ReferenceLoss",
				ns3::DoubleValue(59.7));
		wifiChannel.SetPropagationDelay(
				"ns3::ConstantSpeedPropagationDelayModel");
		ns3::Config::SetDefault(
				"ns3::LogDistancePropagationLossModel::ReferenceLoss",
				ns3::DoubleValue(59.7));

		ns3::YansWifiPhyHelper wifiPhy = ns3::YansWifiPhyHelper::Default();
		wifiPhy.SetChannel(wifiChannel.Create());

		// Install wifiPhy and link it with cgamemac medium access control layer implementation
		NetDeviceContainer scar_rsu = cgamem.InstallRsuAndObu(wifiPhy,
				cgamemacMac, m_nodesRsu, 1); //1 for RSU
		NetDeviceContainer scar_obu = cgamem.InstallRsuAndObu(wifiPhy,
				cgamemacMac, m_nodesObu, 0); //0 for Obu

		// Define positions of the two nodes
		std::stringstream ss1;
		std::string str1;
		ss1 << "scratch/";
		ss1 << numberOfObu;
		ss1 << ".tcl";
		ss1 >> str1;
		std::string traceFile = str1;

		Ns2MobilityHelper ns2car = Ns2MobilityHelper(traceFile);
		ns2car.Install(m_nodesObu.Begin(), m_nodesObu.End());

		ns3::MobilityHelper mobility;
		ns3::Ptr<ns3::ListPositionAllocator> positionAlloc = ns3::CreateObject<
				ns3::ListPositionAllocator>();

		positionAlloc->Add(ns3::Vector(0, 0.0, 0.0));
		positionAlloc->Add(ns3::Vector(500, 0.0, 0.0));
		positionAlloc->Add(ns3::Vector(500, 500, 0.0));
		positionAlloc->Add(ns3::Vector(0, 1000, 0.0));
		positionAlloc->Add(ns3::Vector(1000, 1000, 0.0));

		mobility.SetPositionAllocator(positionAlloc);
		mobility.Install(m_nodesRsu);

		InternetStackHelper internet;
		internet.Install(m_nodesRsu);
		internet.Install(m_nodesObu);

		Ipv4AddressHelper ipv4;
		ipv4.SetBase("10.0.0.0", "255.255.0.0");
		ipv4.Assign(scar_rsu);
		ipv4.Assign(scar_obu);
		//{ "greedy", "lpr", "randomly","coalition" };

		ns3::Ptr<cgame::cgameinoutgreedy> greedy = ns3::Create<
				cgame::cgameinoutgreedy>(COMRANGE_RSU,
		COMRANGE_OBU, NODENUMBER_RSU + numberOfObu, NumberOfNeighor, extra);

		ns3::Config::ConnectWithoutContext(
				"/NodeList/*/DeviceList/*/$cgamemac::CgamemacNetDevice/Mac/txTrace",
				ns3::MakeCallback(&Experiment::m_txTrace, this));
		ns3::Config::ConnectWithoutContext(
				"/NodeList/*/DeviceList/*/$cgamemac::CgamemacNetDevice/Mac/rxTrace",
				ns3::MakeCallback(&Experiment::m_rxTrace, this));

		ns3::Simulator::Stop(ns3::Seconds(EndTime));
		ns3::Simulator::Run();
		ns3::Simulator::Destroy();

	}

private:

	std::string Set2String(std::set<uint8_t> set) {
		std::stringstream ss;
		for (std::set<uint8_t>::iterator i = set.begin(); i != set.end(); i++) {
			ss << (int) *i<<" ";
		}
		return ss.str();
	}

	void m_txTrace(uint32_t nodeId, ns3::Ptr<const ns3::Packet> packet,
			std::set<uint8_t> sendDataItems, std::set<uint8_t> cachedDataitems,
			uint8_t m_type) {
		int currentTime = ns3::Simulator::Now().GetSeconds();

		std::cout << "NodeId " << nodeId << " type " << (int) m_type << " at "
		 << ns3::Simulator::Now() << " send a packet "
		 << packet->GetUid() << " sent content: "
		 << Set2String(sendDataItems) << " cached content: "
		 << Set2String(cachedDataitems) << std::endl;
		sta->SendAPacket(nodeId, currentTime, sendDataItems, cachedDataitems,
				m_type);

	}
	void m_rxTrace(uint32_t nodeId, uint8_t Sendertype,
			std::set<uint8_t> ReceivedDataItems,
			uint32_t ActualreceivedPackets, uint8_t Receivertype) {
		int currentTime = ns3::Simulator::Now().GetSeconds();

		std::cout << "NodeId " << nodeId << " type " << (int) Receivertype
		 << " at " << ns3::Simulator::Now() << " receive a packet from "
		 << (int) Sendertype << " receive content: "
		 << Set2String(ReceivedDataItems) << std::endl;
		sta->ReceivedAPacket(nodeId, currentTime, Sendertype, ReceivedDataItems,
				ActualreceivedPackets, Receivertype);

	}

};

int main(int argc, char *argv[]) {

	new Experiment();

	return 0;
}
