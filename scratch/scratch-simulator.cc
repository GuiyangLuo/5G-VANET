/*
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
 #include "Python.h"
 using namespace ns3;

 int great_function_from_python(int a) {
 int res;
 PyObject *pModule,*pFunc;
 PyObject *pArgs, *pValue;

 import
 pModule = PyImport_Import(PyString_FromString("ttt"));

 great_module.great_function
 pFunc = PyObject_GetAttrString(pModule, "great_function");

 build args
 pArgs = PyTuple_New(1);
 PyTuple_SetItem(pArgs,0, PyInt_FromLong(a));

 call
 pValue = PyObject_CallObject(pFunc, pArgs);
 std::std::cout<<"hehe !";

 res = PyInt_AsLong(pValue);
 return res;
 }



 int main()
 {
 std::std::cout<<"hehe 1!";
 Py_Initialize();
 std::std::cout<<"hehe 2!";
 printf("%d",great_function_from_python(2));
 Py_Finalize();


 }
 */
#include "Python.h"
#include "ns3/assert.h"
#include <string>
#include "ns3/random-variable.h"
#include<iostream>
using namespace std;
uint8_t *abc(uint32_t m_allTTsNumber, uint8_t *m_weightForTT,
		bool **m_confilctGraph) {
	Py_Initialize();
	uint8_t *results = new uint8_t[m_allTTsNumber];
	std::string path =
			"/home/lgy/eclipse_workspace/ns3/ns3.21/ns-3-allinone/ns-3.21/scratch";
	std::string chdir_cmd = std::string("sys.path.append(\"") + path + "\")";
	const char* cstr_cmd = chdir_cmd.c_str();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString(cstr_cmd);

	PyObject* moduleName = PyString_FromString("integerWmis");
	PyObject* pModule = PyImport_Import(moduleName);
	PyObject* pv = PyObject_GetAttrString(pModule, "runwmis");
	PyObject* args = PyTuple_New(3);
	PyObject * ttnumber = PyInt_FromLong(m_allTTsNumber);
	PyObject * PyListGraph = PyList_New(m_allTTsNumber * m_allTTsNumber);
	PyObject *PyListweight = PyList_New(m_allTTsNumber);
	PyObject* listresult;
	PyObject *ListItem;

	if (!pModule) {
		NS_ASSERT(1 == 0);
		return 0;
	}

	if (!pv || !PyCallable_Check(pv)) {
		NS_ASSERT(1 == 0);
		return 0;
	}

	for (uint32_t Index_i = 0; Index_i < m_allTTsNumber; Index_i++) {

		PyList_SetItem(PyListweight, Index_i,
				PyFloat_FromDouble(m_weightForTT[Index_i]));
	}

	for (uint32_t i = 0; i < m_allTTsNumber; i++) {

		for (uint32_t j = 0; j < m_allTTsNumber; j++) {

			PyList_SetItem(PyListGraph, i * m_allTTsNumber + j,
					PyInt_FromLong((double) m_confilctGraph[i][j]));
		}
	}

	PyTuple_SetItem(args, 0, ttnumber);
	PyTuple_SetItem(args, 1, PyListweight);
	PyTuple_SetItem(args, 2, PyListGraph);
	std::cout << "begin call!";
	listresult = PyObject_CallObject(pv, args);
	if (PyList_Check(listresult)) {

		int SizeOfList = PyList_Size(listresult);

		for (int Index_i = 0; Index_i < SizeOfList; Index_i++) {

			ListItem = PyList_GetItem(listresult, Index_i);

			results[Index_i] = (uint8_t) PyInt_AsLong(ListItem);

		}

		std::cout << endl;

	}

	Py_DECREF(moduleName);
	Py_DECREF(pModule);
	Py_DECREF(pv);
	Py_DECREF(args);
	Py_DECREF(ttnumber);
	Py_DECREF(PyListGraph);
	Py_DECREF(PyListweight);
	Py_DECREF(listresult);
	Py_DECREF(ListItem);

	Py_Finalize();
	return results;
}

int main(int argc, char *argv[]) {
	int m_allTTsNumber = 20;
	uint8_t *m_weightForTT = new uint8_t[m_allTTsNumber];
	bool **m_confilctGraph;
	m_confilctGraph = new bool *[m_allTTsNumber];
	for (int i = 0; i < m_allTTsNumber; i++)
		m_confilctGraph[i] = new bool[m_allTTsNumber];

	ns3::UniformVariable m_uniRanVar;

	for (int i = 0; i < m_allTTsNumber; i++) {
		for (int j = 0; j < m_allTTsNumber; j++) {

			m_confilctGraph[i][j] = (int) m_uniRanVar.GetValue(0, 2);
		}
	}

	for (int i = 0; i < m_allTTsNumber; i++) {
		m_weightForTT[i] = (int) m_uniRanVar.GetValue(0, 89);
	}
	std::cout << "begin!";

	for (int aa = 1; aa < 30; aa++) {
		uint8_t * rr = abc(m_allTTsNumber, m_weightForTT, m_confilctGraph);
		std::cout << "begin return!!";
		for (int i = 0; i < m_allTTsNumber; i++)
			std::cout << (int) rr[i];
	}
	for (int i = 0; i < m_allTTsNumber; i++) {
		delete[] m_confilctGraph[i];
	}
	delete[] m_weightForTT;

	return 0;
}

