#include "cgame-inout-greedyImproved.h"
#include "ns3/cgamemac-module.h"
#include "ns3/log.h"
#include "ns3/node-list.h"
#include "ns3/object.h"
#include "ns3/callback.h"
#include "ns3/core-module.h"
#include "ns3/vector.h"
NS_LOG_COMPONENT_DEFINE("cgameinoutgreedyImproved");
namespace cgame {

ns3::TypeId cgameinoutgreedyImproved::GetTypeId(void) {
	ns3::TypeId tid =
			ns3::TypeId("cgame::cgameinoutgreedyImproved").SetParent<ns3::Object>().AddConstructor<
					cgame::cgameinoutgreedyImproved>().AddAttribute(
					"m_NeighborNumber",
					"The maximum number of bytes per slot (excludes MAC cfrmac and lower layer wrapping)."
							"If packet is greater than this, it will be dropped ",
					ns3::UintegerValue(10),
					ns3::MakeUintegerAccessor(
							&cgameinoutgreedyImproved::m_NeighborNumber),
					ns3::MakeUintegerChecker<uint32_t>());
	return tid;
}
cgameinoutgreedyImproved::cgameinoutgreedyImproved() {
	NS_ASSERT(false);
	setCallbackForReceiveFromMaclayer();

}
cgameinoutgreedyImproved::cgameinoutgreedyImproved(double comRangeRsu,
		double comRangeObu, uint32_t nodeNumber, uint32_t neighborNumber,
		std::string extra) {
	m_NeighborNumber = neighborNumber;
	setCallbackForReceiveFromMaclayer();
	channelRelated = ns3::Create<cgame::cgameWhetherReceive>();
	m_comRangeRsu = comRangeRsu;
	m_comRangeObu = comRangeObu;
	m_nodeNumber = nodeNumber;
	m_NodeInfor.clear();
	std::stringstream ss1;
	std::string ss2;
	ss1 << "numberOfTTs";
	ss1 << "_greedyImprove";
	ss1 << "_";
	ss1 << extra;
	ss1 << "_";
	ss1 << nodeNumber;
	ss1 << ".txt";
	ss1 >> ss2;
	numberresultFile.open(ss2.data(), std::ios::out);

}

void cgameinoutgreedyImproved::receiveFromMaclayer(
		ns3::Ptr<cgame::GraphUpLoad> up) {
	m_receivedTimes++;
	//up->print();
	m_NodeInfor.insert(
			std::make_pair<uint32_t, ns3::Ptr<cgame::GraphUpLoad> >(
					up->getNodeId(), up));

	if (m_NodeInfor.size() == m_nodeNumber) {
		m_receivedTimes = 0;
		SchedulingBegin();
	}

}
void cgameinoutgreedyImproved::printAllTT() {

	std::cout << "The number of TTs is " << m_allTTs.size() << " its are :\n";
	uint32_t ijk = 0;
	for (std::vector<TT>::iterator it = m_allTTs.begin(); it != m_allTTs.end();
			it++) {
		std::cout << "Number " << ijk << ", which is <" << (*it).sender << ",(";
		for (std::set<uint8_t>::iterator a = (*it).dataitems.begin();
				a != (*it).dataitems.end(); a++) {
			std::cout << (int) *a << ",";

		}
		std::cout << ")," << (*it).receiver << " it's weight is: "
				<< (int) m_weightForTT[ijk] << "\n";
		ijk++;
	}

}
void cgameinoutgreedyImproved::printCOnflictgraph() {
	uint32_t first = 0, second = 0;
	std::cout << "The confilct graph is:\n";
	for (first = 0; first < m_allTTs.size(); first++) {
		for (second = 0; second < m_allTTs.size(); second++) {
			std::cout << m_confilctGraph[first][second] << "";
		}
		std::cout << "\n";
	}

}
void cgameinoutgreedyImproved::printDirectedNeighborGraph() {
	uint32_t first = 0, second = 0;
	std::cout << "The Directed neighbor graph is:\n";
	for (first = 0; first < m_nodeNumber; first++) {
		for (second = 0; second < m_nodeNumber; second++) {
			std::cout << m_channelInfor[first][second] << "    ";
		}
		std::cout << "\n";
	}

}

void cgameinoutgreedyImproved::printNodeInfor() {
	std::cout << "All the node infor is:\n";
	for (std::map<uint32_t, ns3::Ptr<cgame::GraphUpLoad> >::iterator it =
			m_NodeInfor.begin(); it != m_NodeInfor.end(); it++) {
		std::cout << "Node id " << (*it).first;
		(*it).second->print();
		std::cout << std::endl;
	}

}
void cgameinoutgreedyImproved::SchedulingBegin() {

	m_channelInfor = new double*[m_nodeNumber];
	for (uint32_t i = 0; i < m_nodeNumber; i++) {
		m_channelInfor[i] = new double[m_nodeNumber];
	}

	uint32_t first = 0, second = 0;
	for (first = 0; first < m_nodeNumber; first++) {
		for (second = 0; second < m_nodeNumber; second++) {
			m_channelInfor[first][second] = 0;
		}
	}
	printNodeInfor();
	GetChannelInfor();

	//printDirectedNeighborGraph();
	m_allTTs.clear();
	GetAllTTs();
	numberresultFile << ns3::Simulator::Now().GetSeconds() << "  "
			<< m_allTTs.size() << "  ";
	m_allTTs.clear();

	GetAllTTsReduced();

	numberresultFile << m_allTTs.size() << "  ";
	DeleteSameForAllTTs();
	numberresultFile << "  " << m_allTTs.size() << "\n";

	//After get all the TT, allocate m_confilctGraph according to the size of TT.
	uint32_t allTTSize = m_allTTs.size();
	m_confilctGraph = new bool*[allTTSize];
	for (uint32_t i = 0; i < allTTSize; i++) {
		m_confilctGraph[i] = new bool[allTTSize];
	}
	m_weightForTT = new uint8_t[allTTSize];
	for (first = 0; first < m_allTTs.size(); first++) {
		for (second = 0; second < m_allTTs.size(); second++) {
			m_confilctGraph[first][second] = false;
		}

	}

	CheckForConfilct();
	//printCOnflictgraph();
	// already get the conflict graph.

	getWeightForEachTT();
	printAllTT();
	std::cout << " all TT size=" << allTTSize << "\n";
	GreedyAlgorithm();

	HandlingResults();
	//Scheduling for transmission.

	sendAdToMaclayer (Schedulingresults);

	ClearForPerScheduling();
}
void cgameinoutgreedyImproved::DeleteSameForAllTTs() {
	uint32_t first, second;
	std::set < uint32_t > deletedNodes;
	deletedNodes.clear();
	for (first = 0; first < m_allTTs.size(); first++) {
		for (second = first + 1; second < m_allTTs.size(); second++) {
			if (m_allTTs[first].sender == m_allTTs[second].sender
					&& m_allTTs[first].receiver == m_allTTs[second].receiver) {
				if (AndforTwoSet((m_allTTs[first]).dataitems,
						m_allTTs[second].dataitems).size()
						== m_allTTs[first].dataitems.size()) {
					deletedNodes.insert(second);
				}
			}
		}
	}
	/*	std::cout << "\n deletedNodes is";
	 for (std::set<uint32_t>::iterator de = deletedNodes.begin();
	 de != deletedNodes.end(); de++) {
	 std::cout << *de << " ";

	 }*/
	int ijk = 0;
	for (std::vector<TT>::iterator it = m_allTTs.begin(); it != m_allTTs.end();
			) {
		if (deletedNodes.find(ijk) != deletedNodes.end()) {
			it = m_allTTs.erase(it);
		} else {
			it++;
		}
		ijk++;
	}

}

void cgameinoutgreedyImproved::HandlingResults() {
	std::cout << "\n\n\nCurrent Time is " << ns3::Simulator::Now().GetSeconds()
			<< "\n";
	for (std::set<uint32_t>::iterator i = m_ChoosedIdsForTTs.begin();
			i != m_ChoosedIdsForTTs.end(); i++) {
		TT cur = m_allTTs[*i];
		std::cout << "scheduled TT=<" << cur.sender << "; ";
		for (std::set<uint8_t>::iterator it = cur.dataitems.begin();
				it != cur.dataitems.end(); it++) {
			std::cout << (int) *it << ".";
		}
		std::cout << "\n";

		bool isneedadded = true;
		for (std::vector<ns3::Ptr<cgame::cgameDownload> >::iterator it =
				Schedulingresults.begin(); it != Schedulingresults.end();
				it++) {
			if ((*it)->getNodeId() == cur.sender) {
				/*NS_ASSERT(
				 AndforTwoSet((*it)->getImti(), cur.dataitems).size()
				 == cur.dataitems.size());*/
				isneedadded = false;
			}
		}
		if (isneedadded == false) {
			continue;
		}

		//std::cout << "EXE-----------------1-------------";
		///// Improve for the scheduling!!  context-aware greedy scheduling.
		std::set < uint32_t > SetOfReceivers;
		SetOfReceivers.clear();
		for (std::set<uint32_t>::iterator temp = m_ChoosedIdsForTTs.begin();
				temp != m_ChoosedIdsForTTs.end(); temp++) {
			if (m_allTTs[*temp].sender == cur.sender) {
				NS_ASSERT(
						AndforTwoSet(m_allTTs[*temp].dataitems, cur.dataitems).size()
								== cur.dataitems.size());
				SetOfReceivers.insert(m_allTTs[*temp].receiver);
				//std::cout << " receivers is " << m_allTTs[*temp].receiver << "\n";
			}

		}
		//std::cout << "EXE-----------------2-------------";
		//////// All possible transmissions.
		std::set < uint8_t > possibileTxItems;
		for (std::set<uint32_t>::iterator s = SetOfReceivers.begin();
				s != SetOfReceivers.end(); s++) {
			possibileTxItems = OrforTwoSet(possibileTxItems,
					m_NodeInfor[*s]->getBetai());
		}
		std::cout << "\n  possibileTxItems.size()" << possibileTxItems.size()
				<< "\n";
		possibileTxItems = AndforTwoSet(m_NodeInfor[cur.sender]->getAlphai(),
				possibileTxItems);
		if (possibileTxItems.size() == 0) {
			NS_ASSERT(1 == 0);
			return;
		}
		m_allCachedDatas.clear();
		m_allPossibleDataItems.clear();
		for (std::set<uint8_t>::iterator it = possibileTxItems.begin();
				it != possibileTxItems.end(); it++) {
			m_allCachedDatas.push_back(*it);
		}

		std::vector < uint8_t > tttemp;
		alpha2TimesReducedForItems(0, tttemp);
		//std::cout << "EXE-----------------3-------------";
		///Get all the possible transmission.!!!
		int currentBalancedrate = 0;
		int maximumNDataitemsReceived = 0;
		std::set < uint8_t > CorrespondingImti;
		for (std::vector<std::set<uint8_t> >::iterator imti =
				m_allPossibleDataItems.begin();
				imti != m_allPossibleDataItems.end(); imti++) {
			if ((*imti).size() < cur.dataitems.size()) {
				continue;
			}
			int currentWeightSum = 0;
			for (std::set<uint32_t>::iterator s = SetOfReceivers.begin();
					s != SetOfReceivers.end(); s++) {
				if (m_channelInfor[cur.sender][*s] >= (*imti).size()) {
					currentWeightSum =
							currentWeightSum
									+ AndforTwoSet(*imti,
											m_NodeInfor[*s]->getBetai()).size();
				}
			}

			if (currentWeightSum > maximumNDataitemsReceived) {
				maximumNDataitemsReceived = currentWeightSum;
				CorrespondingImti = *imti;
				currentBalancedrate = BalancedRate(cur.sender,
						CorrespondingImti, SetOfReceivers);

			}
			if (currentWeightSum == maximumNDataitemsReceived) {
				int curBrate = BalancedRate(cur.sender, *imti, SetOfReceivers);
				if (curBrate > currentBalancedrate) {
					CorrespondingImti = *imti;
					currentBalancedrate = curBrate;
				}

			}

		}
		if (maximumNDataitemsReceived <= 0) {
			NS_ASSERT(1 == 0);
			return;
		}
		//std::cout << "EXE-----------------4-------------";
		if (isneedadded == true) {
			Schedulingresults.push_back(
					ns3::Create < cgame::cgameDownload
							> (cur.sender, CorrespondingImti));
			std::cout << "Actual scheduled scheduled TT=<" << cur.sender
					<< "; ";
			for (std::set<uint8_t>::iterator it = CorrespondingImti.begin();
					it != CorrespondingImti.end(); it++) {
				std::cout << (int) *it << ".";
			}
			std::cout << "\n";
		}

	}
}

int cgameinoutgreedyImproved::BalancedRate(uint32_t sender,
		std::set<uint8_t> items, std::set<uint32_t> receivers) {
	int balancedRate = 0;
	for (std::set<uint32_t>::iterator re = receivers.begin();
			re != receivers.end(); re++) {

		// find all the neighbors for receiver re.
		std::vector < uint32_t > allneighbors;
		allneighbors.clear();
		for (uint32_t noden = 0; noden < m_nodeNumber; noden++) {
			if (m_channelInfor[sender][noden] > 0) {
				allneighbors.push_back(noden);
			}
		}
		// for each item, find the possible valid recived degree.
		for (std::set<uint8_t>::iterator item = items.begin();
				item != items.end(); item++) {

			for (std::vector<uint32_t>::iterator recei = allneighbors.begin();
					recei != allneighbors.end(); recei++) {
				if (receivers.find(*recei) != receivers.end()) {
					continue;
				} else {
					std::set < uint8_t > betai =
							m_NodeInfor[*recei]->getBetai();
					if (betai.find(*item) == betai.end()) {
						balancedRate++;
					}

				}

			}

		}
	}
	return balancedRate;
}
void cgameinoutgreedyImproved::GreedyAlgorithm() {
	std::set < uint32_t > remainedTTs;
	for (uint32_t i = 0; i < m_allTTs.size(); i++) {
		remainedTTs.insert(i);
	}
	while (remainedTTs.size() > 0) {
		double maxvalue = -1;
		uint32_t maxindex = -1;
		for (std::set<uint32_t>::iterator first = remainedTTs.begin();
				first != remainedTTs.end(); first++) {
			double weight = (double) m_weightForTT[*first];
			uint32_t degreetemp = 0;
			for (std::set<uint32_t>::iterator second = remainedTTs.begin();
					second != remainedTTs.end(); second++) {
				if (m_confilctGraph[*first][*second] == true) {
					degreetemp++;
				}
			}
			double targetvalue = weight / ((double) degreetemp + 1);
			if (targetvalue > maxvalue) {
				maxvalue = targetvalue;
				maxindex = *first;
			}
		}
		m_ChoosedIdsForTTs.insert(maxindex);
		remainedTTs.erase(maxindex);
		for (uint32_t i = 0; i < m_allTTs.size(); i++) {
			if (m_confilctGraph[maxindex][i] == true) {
				remainedTTs.erase(i);
			}
		}
	}

}
void cgameinoutgreedyImproved::getWeightForEachTT() {
	for (uint32_t first = 0; first < m_allTTs.size(); first++) {
		TT cur = m_allTTs[first];

		std::set < uint8_t > afterand = AndforTwoSet(cur.dataitems,
				m_NodeInfor[cur.receiver]->getBetai());
		m_weightForTT[first] = (uint8_t) afterand.size();
		/*	/////////////////////////////////////////////////////
		 std::cout << " getWeightForEachTT, and is =";
		 for (std::set<uint8_t>::iterator a = afterand.begin();
		 a != afterand.end(); a++) {
		 std::cout << (int) *a << ",";

		 }

		 std::cout << " Size=" << (int) m_weightForTT[first] << "\n\n";*/

	}

}
void cgameinoutgreedyImproved::CheckForConfilct() {
//std::vector<TT> m_allTTs;
	uint32_t first = 0, second = 0;
	for (first = 0; first < m_allTTs.size(); first++) {
		for (second = 0; second < m_allTTs.size() && second <= first;
				second++) {
			// constraint 1, one node can transmit a packet at a time;
			bool finalResults = false;
			if (m_allTTs[first].sender == m_allTTs[second].sender
					&& !isEqualForTwoSet(m_allTTs[first].dataitems,
							m_allTTs[second].dataitems)) {
				finalResults = true;

			}
			// constraint2  can not be both sender and receiver.
			else if (m_allTTs[first].sender == m_allTTs[second].receiver
					|| m_allTTs[first].receiver == m_allTTs[second].sender) {
				finalResults = true;

			}
			// constraint 3 , data collisions.
			else if (m_channelInfor[m_allTTs[first].sender][m_allTTs[second].receiver]
					> 0
					|| m_channelInfor[m_allTTs[second].sender][m_allTTs[first].receiver]
							> 0) {

				finalResults = true;

			} else {
				finalResults = false;
			}
			if (m_allTTs[first].sender == m_allTTs[second].sender

					&& isEqualForTwoSet(m_allTTs[first].dataitems,
							m_allTTs[second].dataitems)) {
				finalResults = false;
			}
			if (first == second) {
				finalResults = false;
			}

			m_confilctGraph[first][second] = finalResults;
			m_confilctGraph[second][first] = finalResults;

		}
	}

}
bool cgameinoutgreedyImproved::isEqualForTwoSet(std::set<uint8_t> set1,
		std::set<uint8_t> set2) {
	for (std::set<uint8_t>::iterator it1 = set1.begin(); it1 != set1.end();
			it1++) {
		if (set2.find(*it1) == set2.end()) {
			return false;
		}

	}

	for (std::set<uint8_t>::iterator it1 = set2.begin(); it1 != set2.end();
			it1++) {
		if (set1.find(*it1) == set1.end()) {
			return false;
		}

	}
	return true;

}

void cgameinoutgreedyImproved::GetAllTTs() {
	std::map<uint32_t, ns3::Ptr<cgame::GraphUpLoad> >::iterator sender;

	for (sender = m_NodeInfor.begin(); sender != m_NodeInfor.end(); sender++) {
		uint32_t senderId = sender->first;
		std::vector < uint32_t > allneighbors;
		allneighbors.clear();

		for (uint32_t noden = 0; noden < m_nodeNumber; noden++) {
			if (m_channelInfor[senderId][noden] > 0) {
				allneighbors.push_back(noden);

			}

		}
		GetTTsForCertainNode(senderId, sender->second->getAlphai(),
				allneighbors);

	}

}

void cgameinoutgreedyImproved::GetAllTTsReduced() {
	std::map<uint32_t, ns3::Ptr<cgame::GraphUpLoad> >::iterator sender;

	for (sender = m_NodeInfor.begin(); sender != m_NodeInfor.end(); sender++) {
		uint32_t senderId = sender->first;
		std::vector < uint32_t > allneighbors;
		allneighbors.clear();

		for (uint32_t noden = 0; noden < m_nodeNumber; noden++) {
			if (m_channelInfor[senderId][noden] > 0) {
				allneighbors.push_back(noden);

			}

		}
		m_allReceivers.clear();
		m_allReceivers = allneighbors;
		m_allPossibleReceivers.clear();
		std::vector < uint32_t > tttemp;
		alpha2TimesReducedForReceivers(0, tttemp);
		for (std::vector<std::vector<uint32_t> >::iterator receivers =
				m_allPossibleReceivers.begin();
				receivers != m_allPossibleReceivers.end(); receivers++) {
			GetTTsForCertainReceivers(senderId, *receivers);
		}

	}

}
void cgameinoutgreedyImproved::GetTTsForCertainReceivers(uint32_t sender,
		std::vector<uint32_t> receivers) {
	std::set < uint8_t > possibileTxItems;
	for (std::vector<uint32_t>::iterator s = receivers.begin();
			s != receivers.end(); s++) {
		possibileTxItems = OrforTwoSet(possibileTxItems,
				m_NodeInfor[*s]->getBetai());
	}
	possibileTxItems = AndforTwoSet(m_NodeInfor[sender]->getAlphai(),
			possibileTxItems);
	if (possibileTxItems.size() == 0) {
		return;
	}
	m_allCachedDatas.clear();
	m_allPossibleDataItems.clear();
	for (std::set<uint8_t>::iterator it = possibileTxItems.begin();
			it != possibileTxItems.end(); it++) {
		m_allCachedDatas.push_back(*it);
	}

	std::vector < uint8_t > tttemp;
	alpha2TimesReducedForItems(0, tttemp);

	uint32_t maximumNDataitems = 0;
	std::set < uint8_t > CorrespondingImti;
	for (std::vector<std::set<uint8_t> >::iterator imti =
			m_allPossibleDataItems.begin();
			imti != m_allPossibleDataItems.end(); imti++) {
		uint32_t currentWeightSum = 0;
		for (std::vector<uint32_t>::iterator s = receivers.begin();
				s != receivers.end(); s++) {
			if (m_channelInfor[sender][*s] >= (*imti).size()) {
				currentWeightSum =
						currentWeightSum
								+ AndforTwoSet(*imti,
										m_NodeInfor[*s]->getBetai()).size();
			}
		}
		if (maximumNDataitems < currentWeightSum) {
			maximumNDataitems = currentWeightSum;
			CorrespondingImti = *imti;
		}

	}
	if (maximumNDataitems <= 0) {
		return;
	}

///Add this to TTs
	for (std::vector<uint32_t>::iterator s = receivers.begin();
			s != receivers.end(); s++) {
		if (m_channelInfor[sender][*s] >= CorrespondingImti.size()
				&& AndforTwoSet(CorrespondingImti, m_NodeInfor[*s]->getBetai()).size()
						> 0) {
			TT* a = new TT();
			a->sender = sender;
			a->dataitems = CorrespondingImti;
			a->receiver = *s;
			m_allTTs.push_back(*a);
		}
	}
}

void cgameinoutgreedyImproved::alpha2TimesReducedForItems(uint32_t i,
		std::vector<uint8_t> Temp) {
	std::vector < uint8_t > reTemp;
	reTemp = Temp;
	if (i == m_allCachedDatas.size()) {
		m_allPossibleDataItems.push_back(ConvertVectorToSet(Temp));
		return;
	} else {
		alpha2TimesReducedForItems(i + 1, reTemp);
		reTemp.push_back(m_allCachedDatas[i]);
		alpha2TimesReducedForItems(i + 1, reTemp);

	}
}

void cgameinoutgreedyImproved::alpha2TimesReducedForReceivers(uint32_t i,
		std::vector<uint32_t> Temp) {
	std::vector < uint32_t > reTemp;
	reTemp = Temp;
	if (i == m_allReceivers.size()) {
		m_allPossibleReceivers.push_back(Temp);
		return;
	} else {
		alpha2TimesReducedForReceivers(i + 1, reTemp);
		reTemp.push_back(m_allReceivers[i]);
		alpha2TimesReducedForReceivers(i + 1, reTemp);

	}
}
std::set<uint8_t> cgameinoutgreedyImproved::OrforTwoSet(std::set<uint8_t> alpha,
		std::set<uint8_t> beta) {
	std::set < uint8_t > results;
	results.clear();
	std::set<uint8_t>::iterator it1;
	for (it1 = alpha.begin(); it1 != alpha.end(); ++it1) {
		results.insert((*it1));

	}
	for (it1 = beta.begin(); it1 != beta.end(); ++it1) {
		results.insert((*it1));

	}
	return results;
}

void cgameinoutgreedyImproved::GetTTsForCertainNode(uint32_t senderId,
		std::set<uint8_t> alpha, std::vector<uint32_t> allneighbors) {
	std::set < uint32_t > allneigborsOfChannel;
	allneigborsOfChannel = VectorToSet(senderId, allneighbors,
			allneigborsOfChannel);
	/*////////////////////////////////////////////////////////////////////////
	 std::cout << "all the neighbor foer node " << senderId << "is  ";
	 for (std::set<uint32_t>::iterator it = allneigborsOfChannel.begin();
	 it != allneigborsOfChannel.end(); it++) {
	 std::cout << (int) *it << ",";

	 }
	 std::cout <<"\n";
	 ////////////////////////////////////////////////////////////////////////*/

	GetAllPossibleTransmissionSet(senderId, alpha, allneighbors,
			allneigborsOfChannel);

}

void cgameinoutgreedyImproved::alpha2Times(uint32_t i,
		std::vector<uint8_t> Temp) {
	std::vector < uint8_t > reTemp;
	reTemp = Temp;
	if (i == m_alpha.size()) {
		m_allPossibleTransmission.push_back(ConvertVectorToSet(Temp));
		return;
	} else {
		alpha2Times(i + 1, reTemp);
		reTemp.push_back(m_alpha[i]);
		alpha2Times(i + 1, reTemp);

	}
}
std::set<uint8_t> cgameinoutgreedyImproved::ConvertVectorToSet(
		std::vector<uint8_t> vector1) {
	std::set < uint8_t > set;
	for (std::vector<uint8_t>::const_iterator iter = vector1.begin();
			iter != vector1.end(); iter++) {
		set.insert(*iter);
	}
	return set;
}
void cgameinoutgreedyImproved::SelectOnExponentTTs(uint32_t senderId,
		std::set<uint8_t> alpha, std::vector<uint32_t> allneighbors,
		std::set<uint32_t> allneigborsOfChannel) {
	if (allneigborsOfChannel.size() == 0)
		return;
	for (std::set<uint32_t>::iterator numberofimti =
			allneigborsOfChannel.begin();
			numberofimti != allneigborsOfChannel.end(); numberofimti++) {
		std::set<uint8_t> remined, temp, theAndfotSets = alpha;
		uint32_t numbercanbereceived = 0;
		std::set < uint32_t > correspondingNeighbors;
		for (std::vector<uint32_t>::iterator nei = allneighbors.begin();
				nei != allneighbors.end(); nei++) {
			if (m_channelInfor[senderId][*nei] >= *numberofimti) {
				//temp = m_NodeInfor[*nei]->getBetai();
				theAndfotSets = AndforTwoSet(theAndfotSets,
						m_NodeInfor[*nei]->getBetai());
			}

		}
		std::set < uint8_t > abc;
		for (std::vector<std::set<uint8_t> >::iterator ittemp =
				m_allPossibleTransmission.begin();
				ittemp != m_allPossibleTransmission.end();) {

			abc = (*ittemp);
			/*	std::cout << "\n if (allneigborsOfChannel.find((abc).size()) "
			 << (abc).size() << "  ";
			 for (std::set<uint8_t>::iterator ita = (abc).begin();
			 ita != (abc).end(); ita++) {
			 std::cout << (int) *ita << " ";

			 }*/
			if (allneigborsOfChannel.find((abc).size())
					== allneigborsOfChannel.end()) {
				//NS_ASSERT((*it).size() != 0);
				ittemp = m_allPossibleTransmission.erase(ittemp);
				/*continue;*/

			} else {
				if ((abc).size() == *numberofimti
						&& AndforTwoSet(theAndfotSets, abc).size() != 0) {
					for (std::vector<uint32_t>::iterator neitemp =
							allneighbors.begin(); neitemp != allneighbors.end();
							neitemp++) {
						uint32_t tempreceived = 0;
						if (m_channelInfor[senderId][*neitemp]
								>= *numberofimti) {
							//temp = m_NodeInfor[*nei]->getBetai();
							tempreceived =
									tempreceived
											+ AndforTwoSet(abc,
													m_NodeInfor[*neitemp]->getBetai()).size();
						}
						if (tempreceived > numbercanbereceived) {
							remined = abc;
						}

					}
					ittemp = m_allPossibleTransmission.erase(ittemp);

				} else {
					ittemp++;
				}

			}

		}
		m_allPossibleTransmission.push_back(remined);
	}

}
int cgameinoutgreedyImproved::PrintNumberOfOriginsTT(uint32_t senderId,
		std::set<uint8_t> alpha, std::vector<uint32_t> allneighbors,
		std::set<uint32_t> allneigborsOfChannel) {
	int totalNumber = 0;
	for (std::vector<std::set<uint8_t> >::iterator it =
			m_allPossibleTransmission.begin();
			it != m_allPossibleTransmission.end(); it++) {
		//
		if (allneigborsOfChannel.find((*it).size())
				!= allneigborsOfChannel.end()) {
			for (std::vector<uint32_t>::iterator receiver =
					allneighbors.begin(); receiver != allneighbors.end();
					receiver++) {
				NS_ASSERT(m_channelInfor[senderId][*receiver] > 0);
				std::set < uint8_t > uncacheddata =
						m_NodeInfor[*receiver]->getBetai();
				std::set < uint8_t > Validdata = AndforTwoSet(*it,
						uncacheddata);
				if (m_channelInfor[senderId][*receiver] >= (*it).size()
						&& Validdata.size() > 0) {
					totalNumber++;
				}

			}

		}

	}

	return totalNumber;

}
void cgameinoutgreedyImproved::GetAllPossibleTransmissionSet(uint32_t senderId,
		std::set<uint8_t> alpha, std::vector<uint32_t> allneighbors,
		std::set<uint32_t> allneigborsOfChannel) {
	m_alpha.clear();
	m_allPossibleTransmission.clear();
	for (std::set<uint8_t>::iterator it = alpha.begin(); it != alpha.end();
			it++) {
		m_alpha.push_back(*it);
	}
	std::vector < uint8_t > tttemp;
	alpha2Times(0, tttemp);

////

// 2^n possible transmission for each alpha, stored in the m_allPossibleTransmission
//Selection on the m_allPossibleTransmission
//PrintNumberOfOriginsTT(senderId, alpha, allneighbors, allneigborsOfChannel);
//SelectOnExponentTTs(senderId, alpha, allneighbors, allneigborsOfChannel);

	int totalNumber = 0;
	for (std::vector<std::set<uint8_t> >::iterator it =
			m_allPossibleTransmission.begin();
			it != m_allPossibleTransmission.end(); it++) {
		//
		if (allneigborsOfChannel.find((*it).size())
				!= allneigborsOfChannel.end()) {
			for (std::vector<uint32_t>::iterator receiver =
					allneighbors.begin(); receiver != allneighbors.end();
					receiver++) {
				NS_ASSERT(m_channelInfor[senderId][*receiver] > 0);
				std::set < uint8_t > uncacheddata =
						m_NodeInfor[*receiver]->getBetai();
				std::set < uint8_t > Validdata = AndforTwoSet(*it,
						uncacheddata);
				if (m_channelInfor[senderId][*receiver] >= (*it).size()
						&& Validdata.size() > 0) {
					TT* a = new TT();
					a->sender = senderId;
					a->dataitems = (*it);
					a->receiver = *receiver;
					m_allTTs.push_back(*a);
					totalNumber++;

				}

			}

		}

	}

}
std::set<uint32_t> cgameinoutgreedyImproved::VectorToSet(uint32_t senderId,
		std::vector<uint32_t> vector1, std::set<uint32_t> set1) {
	set1.clear();
	for (std::vector<uint32_t>::const_iterator iter = vector1.begin();
			iter != vector1.end(); iter++) {
		set1.insert(m_channelInfor[senderId][*iter]);
	}
	return set1;
}

void cgameinoutgreedyImproved::GetChannelInfor() {
	std::map<uint32_t, ns3::Ptr<cgame::GraphUpLoad> >::iterator sender,
			receiver;
	std::vector<double> alldistance;

	for (sender = m_NodeInfor.begin(); sender != m_NodeInfor.end(); sender++) {
		alldistance.clear();

		//get all the distance to its all the nodes. stored in the alldistance.
		uint32_t ijk = 0;
		for (receiver = m_NodeInfor.begin(); receiver != m_NodeInfor.end();
				receiver++) {
			alldistance.push_back(
					channelRelated->CalDistance(sender->second->getPosition(),
							receiver->second->getPosition()));
			/*	std::cout << " sender Position is :("
			 << sender->second->getPosition().x << ","
			 << sender->second->getPosition().y << ","
			 << sender->second->getPosition().z << ")"
			 << " reciever Position is :("
			 << receiver->second->getPosition().x << ","
			 << receiver->second->getPosition().y << ","
			 << receiver->second->getPosition().z << ")";
			 std::cout << "distance between (" << sender->first << ","
			 << receiver->first << " ) is " << alldistance[ijk] << "\n";*/
			ijk++;
		}

		std::set < uint32_t > SelectedNeighborNodes;
		SelectedNeighborNodes.clear();

		while (SelectedNeighborNodes.size() < m_NeighborNumber) {
			uint32_t SelectedNodes = 0;
			double minDistance = m_comRangeRsu * 100000;
			for (uint32_t index = 0; index < m_NodeInfor.size(); index++) {
				if (alldistance[index] < minDistance) {
					SelectedNodes = index;
					minDistance = alldistance[index];
				}
			}

			double Actualdistance;
			int temp = sender->second->getRsuOrObu()
					+ m_NodeInfor[SelectedNodes]->getRsuOrObu();
			if (temp == 1) {
				Actualdistance = m_comRangeRsu;
			} else {
				Actualdistance = m_comRangeObu;
			}

			if (alldistance[SelectedNodes] < Actualdistance) {
				if (SelectedNodes != sender->first) {
					SelectedNeighborNodes.insert(SelectedNodes);
				}
			} else {
				break;
			}
			alldistance[SelectedNodes] = Actualdistance;
		}

		for (std::set<uint32_t>::iterator neighbor =
				SelectedNeighborNodes.begin();
				neighbor != SelectedNeighborNodes.end(); neighbor++) {
			uint32_t channel = channelRelated->ChannelCapacity(
					sender->second->getPosition(),
					sender->second->getRsuOrObu(),
					m_NodeInfor[*neighbor]->getPosition(),
					m_NodeInfor[*neighbor]->getRsuOrObu());
			std::set < uint8_t > alpha = sender->second->getAlphai();
			std::set < uint8_t > beta = m_NodeInfor[*neighbor]->getBetai();

			if (AndforTwoSet(alpha, beta).size() > 0 && channel > 0) {
				m_channelInfor[sender->first][*neighbor] =
						alpha.size() < channel ? alpha.size() : channel;
			}
		}

	}

}

///
std::set<uint8_t> cgameinoutgreedyImproved::AndforTwoSet(std::set<uint8_t> alpha,
		std::set<uint8_t> beta) {
	std::set < uint8_t > results;
	results.clear();
	std::set<uint8_t>::iterator it1;
	for (it1 = alpha.begin(); it1 != alpha.end(); ++it1) {
		if (beta.find(*it1) != beta.end()) {
			results.insert((*it1));
		}

	}

	return results;
}

void cgameinoutgreedyImproved::setCallbackForReceiveFromMaclayer() {

	for (ns3::NodeList::Iterator i = ns3::NodeList::Begin();
			i != ns3::NodeList::End(); ++i) {
		(*i)->GetDevice(0)->GetObject<cgamemac::CgamemacNetDevice>()->SetGraphUpLoadCallback(
				ns3::MakeCallback(&cgameinoutgreedyImproved::receiveFromMaclayer,
						this));

	}

}
void cgameinoutgreedyImproved::SendToMaclayer(
		ns3::Ptr<cgame::cgameDownload> down) {
	/*	std::cout << " reallocate anser. node: " << down->getNodeId() << " slot:"
	 << (double) down->getNewSlot() << "\n";*/
	ns3::Simulator::ScheduleWithContext(down->getNodeId(), ns3::Seconds(0),
			&cgamemac::CgamemacNetDevice::SendtoMacLayer,
			ns3::NodeList::GetNode(down->getNodeId())->GetDevice(0)->GetObject<
					cgamemac::CgamemacNetDevice>(), down);

}

void cgameinoutgreedyImproved::sendAdToMaclayer(
		std::vector<ns3::Ptr<cgame::cgameDownload> > sentevents) {
	for (std::vector<ns3::Ptr<cgame::cgameDownload> >::iterator it =
			sentevents.begin(); it != sentevents.end(); it++) {

		std::set < uint8_t > a = ((*it))->getImti();
		for (std::set<uint8_t>::iterator i = a.begin(); i != a.end(); i++) {

		}

		SendToMaclayer(*it);
	}

}
void cgameinoutgreedyImproved::ClearForPerScheduling() {

	m_NodeInfor.clear();
	uint32_t allTTSize = m_allTTs.size();
	delete[] m_weightForTT;
	for (uint32_t i = 0; i < m_nodeNumber; i++) {
		delete[] m_channelInfor[i];
	}
	for (uint32_t i = 0; i < allTTSize; i++) {
		delete[] m_confilctGraph[i];
	}
	delete[] m_channelInfor;
	delete[] m_confilctGraph;

	m_allTTs.clear();
	Schedulingresults.clear();
	m_ChoosedIdsForTTs.clear();
	m_alpha.clear();
	m_allPossibleTransmission.clear();
}

} // namespace cgame

