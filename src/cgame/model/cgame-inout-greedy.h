#ifndef CGAME_INOUT_GREEDY_H
#define CGAME_INOUT_GREEDY_H
#include <fstream>
#include "cgame-class.h"
#include "ns3/object.h"
#include "ns3/buffer.h"
#include "ns3/callback.h"
#include "ns3/core-module.h"
#include "ns3/nstime.h"
#include  "set"
#include  "vector"
namespace cgame {

class cgameinoutgreedy: public ns3::Object {
public:
	static ns3::TypeId
	GetTypeId(void);
	cgameinoutgreedy();
	cgameinoutgreedy(double comRangeRsu, double comRangeObu,
			uint32_t nodeNumber, uint32_t neighborNumber, std::string extra);
	void SchedulingBegin();
	void GetChannelInfor();
	std::set<uint8_t> ConvertVectorToSet(std::vector<uint8_t> vector1);
	virtual void setCallbackForReceiveFromMaclayer();
	virtual void SendToMaclayer(ns3::Ptr<cgame::cgameDownload> down);
	void sendAdToMaclayer(
			std::vector<ns3::Ptr<cgame::cgameDownload> > sentevents);
	virtual void receiveFromMaclayer(ns3::Ptr<cgame::GraphUpLoad> up);
	void GetAllTTs();

	void GetTTsForCertainNode(uint32_t senderId, std::set<uint8_t> alpha,
			std::vector<uint32_t> allneighbors);
	std::set<uint32_t> VectorToSet(uint32_t senderId,
			std::vector<uint32_t> vector1, std::set<uint32_t> set1);
	std::set<uint8_t> AndforTwoSet(std::set<uint8_t> alpha,
			std::set<uint8_t> beta);
	std::set<uint8_t> OrforTwoSet(std::set<uint8_t> alpha,
			std::set<uint8_t> beta);
	void GetAllPossibleTransmissionSet(uint32_t senderId,
			std::set<uint8_t> alpha, std::vector<uint32_t> allneighbors,
			std::set<uint32_t> allneigborsOfChannel);
	void SelectOnExponentTTs(uint32_t senderId, std::set<uint8_t> alpha,
			std::vector<uint32_t> allneighbors,
			std::set<uint32_t> allneigborsOfChannel);
	void alpha2Times(uint32_t i, std::vector<uint8_t> Temp);
	void CheckForConfilct();
	bool isEqualForTwoSet(std::set<uint8_t> set1, std::set<uint8_t> set2);

	int PrintNumberOfOriginsTT(uint32_t senderId, std::set<uint8_t> alpha,
			std::vector<uint32_t> allneighbors,
			std::set<uint32_t> allneigborsOfChannel);
	void getWeightForEachTT();
	void GreedyAlgorithm();
	void ClearForPerScheduling();

	void printAllTT();
	void printCOnflictgraph();
	void printDirectedNeighborGraph();
	void printNodeInfor();
	void HandlingResults();

	double m_comRangeRsu;
	double m_comRangeObu;

	struct TT {    //tentative transmission
		uint32_t sender;
		std::set<uint8_t> dataitems;
		uint32_t receiver;

	};

	void GetAllTTsReduced();
	void alpha2TimesReducedForReceivers(uint32_t i, std::vector<uint32_t> Temp);
	void alpha2TimesReducedForItems(uint32_t i, std::vector<uint8_t> Temp);
	std::vector<std::vector<uint32_t> > m_allPossibleReceivers;
	std::vector<uint32_t> m_allReceivers;

	std::vector<std::set<uint8_t> > m_allPossibleDataItems;
	std::vector<uint8_t> m_allCachedDatas;
	void GetTTsForCertainReceivers(uint32_t sender,
			std::vector<uint32_t> receivers);
	void DeleteSameForAllTTs();
private:
	uint32_t m_nodeNumber;
	uint32_t m_NeighborNumber;

	uint32_t m_receivedTimes;
	std::map<uint32_t, ns3::Ptr<cgame::GraphUpLoad> > m_NodeInfor; //<nodeNumber,uploadedInformation>
	double **m_channelInfor; // Nodea, Nodeb, channel capacity.  dimension: m_nodeNumber*m_nodeNumber
	bool **m_confilctGraph; // size=m_allTTs.size, size*size. true means two sets is connected.
	uint8_t *m_weightForTT; //m_allTTs.size
	ns3::Ptr<cgame::cgameWhetherReceive> channelRelated;
	std::vector<TT> m_allTTs;
	std::vector<ns3::Ptr<cgame::cgameDownload> > Schedulingresults;
	std::set<uint32_t> m_ChoosedIdsForTTs;
	std::vector<uint8_t> m_alpha;
	std::vector<std::set<uint8_t> > m_allPossibleTransmission;

	//output
	std::ofstream numberresultFile;
};
}
#endif
