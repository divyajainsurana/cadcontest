/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include "cirGate.h"
#include <time.h>
#include <stdlib.h>
using namespace std;

#include "cirDef.h"

extern CirMgr *cirMgr;

// TODO: Define your own data members and member functions
class CirMgr
{
public:
   CirMgr(){ srand(time(NULL)); }
   ~CirMgr();
   bool readCircuit(const string&,CirType);
   void buildCircuitTwo(CirType);
   bool randomSim(const string& outfile, int max);
   //void buildCMiter();
   //void test_randomSim(int max);
   //void writeBlifCMiter(const string&);
   //void writeBlifAllZero(const string&);
   void writeAag(const string&,CirType);
   void writeBlif(const string&, CirType);

private:
   /***Helper functions related to reading circuit ***/
   bool readHeader(ifstream &fin,CirType);
   bool readInput(ifstream &fin,CirType);
   bool readOutput(ifstream &fin, CirType);
   bool readAIG(ifstream &fin,CirType);
   bool readLatch(ifstream &fin,CirType);
   bool readSym(ifstream &fin, CirType);
   bool readComment(ifstream &fin);
   void connect(CirType);
   void dfsCircuitOne(CirType);
   
   /***Helper functions related to building circuit two***/
   void dfsCircuitTwo(CirType); //include ckt one & ckt two
   void dfsBuildCircuitTwo(CirType);
   void setCircuitTwoPO(CirType);

   /***Helper function related to random simulation***/
   void randSignal();
   void simulate(CirType cirType);
   bool isCompatibleEQ(const string& outfile);
   //void test_randSignal();
   //void test_simulate(CirType cirType);
   //bool test_isCompatibleEQ()
  
   /*****Helper function related to build Cmiter****/
   void buildEachCMiter();
   void dfsAllMiter();

   int M,I,L,O,A,AG,AR; //Aw is for write operation
   
   static  CirGate * _const0;
   vector<CirPiGate*> _piList;
   vector<CirGate*> _floatList;
   vector<CirGate*> _unuseList;

   vector<CirPoGate*> _poListG1;
   vector<CirPoGate*> _poListG2;
   vector<CirPoGate*> _poListR1;
   vector<CirPoGate*> _poListR2;
   
   vector<CirGate*> _dfsListG;
   vector<CirGate*> _dfsListR;

   vector<CirGate*> _gateListG;
   vector<CirGate*> _gateListR;
   
   //map c1_Id to c2_Id
   vector<size_t> _CktOneToCktTwoG;    
   vector<size_t> _CktOneToCktTwoR;

   unordered_map<string,size_t> _GSymToGateId;
   vector<string> _RGateIdToSym;

   //signal list
   vector<size_t>  _sigList;
   //vector<bool>	_boolList;
   //vector<Value3>	_value3List;

   //cmiter list
   //vector<CirGate*> _miterList;
   //vector<CirGate*> _dfsListM;
   //vector<CirGate*> _gateListM;
};

#endif // CIR_MGR_H
