#include <map>
#include <fstream>
#include "ns3/ptr.h"
#include "ns3/pointer.h"
#include "ns3/vector.h"
#include <vector>
#define NumberOfTcl 5
#define CommunicationRange  200
#define STEPRANGE 10
using namespace ns3;

class GraphCensus: public ns3::Object {

private:
	int m_numberOfDataItems;
	int m_initNumberDataItems;
	int m_schedulePeriod;

	int m_beginTime;
	int m_endTime;

	///////////////////////////////////////////
	int m_currentSlot;
	int m_originalPossessedData;
	int m_totalTransmission;
	int m_servedNForV2V;
	int m_servedNForV2I;
	int m_rxNFromV2V;
	int m_rxNFromV2I;

	/////////////////////////////////////////
	std::ofstream resultFile;

public:

	GraphCensus(int numberOfDataItems, int initNumberDataItems,
			int schedulePeriod, int oriPossessedData) {
		m_numberOfDataItems = numberOfDataItems;
		m_initNumberDataItems = initNumberDataItems;
		m_schedulePeriod = schedulePeriod;
		m_originalPossessedData = oriPossessedData;
		m_currentSlot = 0;
	}

	void OpenOutPut(int number, std::string algorithm);
	void setBETime(int begin, int end);
	void SendAPacket(uint32_t nodeId, int time, std::set<uint8_t> sendDataItems,
			std::set<uint8_t> cachedDataitems, uint8_t m_type);
	void ReceivedAPacket(uint32_t nodeId, int time, uint8_t sendertype,
			std::set<uint8_t> ReceivedDataItems, uint32_t ActualreceivedPackets,
			uint8_t receivertype);
	int ActualReceivedDta(std::set<uint8_t> ReceivedDataItems,
			std::set<uint8_t> cachedDataitems);
	void FirstplayData();
	void DisplayData();
	void ClearData();
	void CloseFile();
	std::set<uint8_t> AndforTwoSet(std::set<uint8_t> alpha,
			std::set<uint8_t> beta);
};
void GraphCensus::SendAPacket(uint32_t nodeId, int time,
		std::set<uint8_t> sendDataItems, std::set<uint8_t> cachedDataitems,
		uint8_t m_type) {
	int ActualSlot = (time - m_beginTime) / m_schedulePeriod;

	while (ActualSlot > m_currentSlot) {
		DisplayData();
		ClearData();
		m_currentSlot++;
	}
	m_totalTransmission++;
}
int GraphCensus::ActualReceivedDta(std::set<uint8_t> ReceivedDataItems,
		std::set<uint8_t> cachedDataitems) {

	std::set<uint8_t> uncachedData;
	uncachedData.clear();
	for (uint8_t i = 0; i < m_numberOfDataItems; i++) {
		if (cachedDataitems.find(i) == cachedDataitems.end()) {
			uncachedData.insert(i);
		}
	}
	return AndforTwoSet(ReceivedDataItems, uncachedData).size();

}
void GraphCensus::ReceivedAPacket(uint32_t nodeId, int time, uint8_t sendertype,
		std::set<uint8_t> ReceivedDataItems, uint32_t ActualreceivedPackets,
		uint8_t receivertype) {
//	NS_ASSERT((time - m_beginTime) / m_schedulePeriod == m_currentSlot);
	NS_ASSERT(receivertype <= 1);
	NS_ASSERT(sendertype <= 1);

	if (ActualreceivedPackets <= 0) {
		return;
	}
	if (sendertype == 1) { //RSu
		m_servedNForV2I++;
		m_rxNFromV2I += ActualreceivedPackets;
	} else {
		m_servedNForV2V++;
		m_rxNFromV2V += ActualreceivedPackets;
	}
}
void GraphCensus::setBETime(int begin, int end) {
	m_beginTime = begin;
	m_endTime = end;

}
void GraphCensus::OpenOutPut(int number, std::string algorithm) {
	std::stringstream ss1;
	std::string ss2;
	ss1 << "results_";
	ss1 << algorithm;
	ss1 << "_";
	ss1 << number;
	ss1 << ".txt";
	ss1 >> ss2;
	resultFile.open(ss2.data(), std::ios::out);
	ClearData();

	FirstplayData();
}
void GraphCensus::FirstplayData() {
	/*resultFile << "Slot" << "  " << "OPPackets" << "  " << "TTNumber" << "  "
	 << "SNodeNForV2V" << "  " << "SNodeNForV2I" << "  " << "RxPNFromV2V"
	 << "  " << "RxPtNFromV2I";*/

}
std::set<uint8_t> GraphCensus::AndforTwoSet(std::set<uint8_t> alpha,
		std::set<uint8_t> beta) {
	std::set<uint8_t> results;
	results.clear();
	std::set<uint8_t>::iterator it1;
	for (it1 = alpha.begin(); it1 != alpha.end(); ++it1) {
		if (beta.find(*it1) != beta.end()) {
			results.insert((*it1));
		}

	}

	return results;
}
void GraphCensus::DisplayData() {
	resultFile << m_currentSlot << "  " << m_originalPossessedData << "  "
			<< m_totalTransmission << "  " << m_servedNForV2V << "  "
			<< m_servedNForV2I << "  " << m_rxNFromV2V << "  " << m_rxNFromV2I
			<< "\n  ";

}
void GraphCensus::CloseFile() {

	while (m_currentSlot < (m_endTime - m_beginTime) / m_schedulePeriod - 1) {
		DisplayData();
		ClearData();
		m_currentSlot++;
	}
	resultFile << m_currentSlot << "  " << m_originalPossessedData << "  "
			<< m_totalTransmission << "  " << m_servedNForV2V << "  "
			<< m_servedNForV2I << "  " << m_rxNFromV2V << "  " << m_rxNFromV2I;
	resultFile.close();
}
void GraphCensus::ClearData() {

	m_totalTransmission = 0;
	m_servedNForV2V = 0;
	m_servedNForV2I = 0;
	m_rxNFromV2V = 0;
	m_rxNFromV2I = 0;

}
