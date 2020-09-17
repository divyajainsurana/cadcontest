/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include <climits>

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
CirGate* CirMgr::_const0 = new CirConstGate(0);

/**************************************/
/*   Constructors & Destructors       */
/**************************************/
CirMgr::~CirMgr() {
   for(int i=1; i<_gateListG.size(); ++i){
	   //Don't delete _const0
	   if(_gateListG[i]!=NULL){
		   delete _gateListG[i];
		   _gateListG[i] = NULL;
	   }
   }
   for(int i=I+1; i<_gateListR.size(); ++i){
	   //Don't delete _const0 and don't redelete PI
	   if(_gateListR[i]!=NULL){
		   delete _gateListR[i];
		   _gateListR[i] = NULL;
	   }
   }
   /*
   for(int i=0; i<_gateListM.size(); ++i){
	   if(_gateListM[i]!=NULL){
		   delete _gateListM[i];
		   _gateListM[i] = NULL;
	   }
   }
   */
}

/**************************************************************/
/*   Public functions										  */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName, CirType cirType)
{
	ifstream fin(fileName);
	if(	readHeader(fin,cirType) && readInput(fin,cirType) 
		&& readLatch(fin,cirType) && readOutput(fin, cirType) 
		&& readAIG(fin,cirType) && readSym(fin,cirType) && 
		readComment(fin)){
		connect(cirType); 
		dfsCircuitOne(cirType);
		return true;
	}
	fin.close();
	return false;
}

void
CirMgr::buildCircuitTwo(CirType cirType){
	vector<CirGate*>& _gateList =
	(cirType == GOLDEN) ? _gateListG : _gateListR;
	vector<size_t>& _CktOneToCktTwo = 
	(cirType == GOLDEN) ? _CktOneToCktTwoG : _CktOneToCktTwoR;

	_CktOneToCktTwo.resize(_gateList.size());
	dfsBuildCircuitTwo(cirType);
	setCircuitTwoPO(cirType);
	dfsCircuitTwo(cirType);
}

bool
CirMgr::randomSim(const string& outfile, int max=-1)
{
	if(max == -1) max = _piList.size();
	
	for(int m=0; m<max; ++m){
		randSignal();
		simulate(GOLDEN); //simulate 64 pattern
		simulate(REVISED);
		if(!isCompatibleEQ(outfile))
			return false;
	}
	return true;
}

/*
void
CirMgr::test_randomSim(int max = -1)
{
	if(max == -1) max = _piList.size();
		
	for(int m=0; m<max; ++m){
		test_randSignal();
		test_simulate(GOLDEN); 
		test_simulate(REVISED);
		if(!test_isCompatibleEQ()) return;

	}
}
*/
/*
void 
CirMgr::buildCMiter(){
	buildEachCMiter();
	dfsAllMiter();
}

void 
CirMgr::writeBlifCMiter(const string& fileName){
	
	ofstream outfile(fileName);
	outfile<<".model cmiter\n";

	//PI names
	outfile<<".inputs";
	for(int i=0;i<I;i++)
		outfile<<" "<<*(_piList[i]->getSym());
	outfile<<"\n";

	//PO names
	outfile<<".outputs";
	assert(_miterList.size() == _poListG1.size());
	for(int i=0; i< _miterList.size();++i)
		outfile<<" o"<<i;
	outfile<<"\n";

	//CONST0
	outfile<<".names const0"<<"\n";

	//AIG
	for(int i=0;i<_dfsListM.size();i++){
		if(_dfsListM[i]!=NULL && (_dfsListM[i] -> getType()==AIG_GATE
			|| _dfsListM[i]-> getType()==LATCH_GATE)){
			
			outfile<<".names";
			
			for(int j=0;j<2;++j){
				
				int id= _dfsListM[i] -> getFaninGateID(j);
				CirType cirType = _dfsListM[i]->getFaninCirType(j);
				
				const vector<CirGate*>& _gateList =
				(cirType == MITER)? _gateListM :
				(cirType == GOLDEN)?  _gateListG : _gateListR;
				
				char  gate = (cirType == MITER)? 'o'  :
							 (cirType == GOLDEN)? 'g' : 'r'; 

				if(_gateList[id]->getType() == PI_GATE)
					outfile<<" "<<*(_gateList[id]->getSym());
				else if(_gateList[id]->getType() == CONST_GATE)
					outfile<<" const0";
				else
					outfile<<" "<<gate<<id;
			}
			CirType cirType = _dfsListM[i]->getCirType();
			char gate = (cirType == MITER)? 'o'  :
						(cirType == GOLDEN)? 'g' : 'r'; 

			outfile<<" "<<gate<<(_dfsListM[i]-> getID())<<"\n";
			
			for(int j=0; j<2;++j){
				bool phase = _dfsListM[i]-> getFaninGatePhase(j);
				outfile<<!phase;
			}
			outfile<<" "<<1<<"\n";
		}
	}
	outfile<<".end"<<endl;
}

void 
CirMgr::writeBlifAllZero(const string& fileName){
	
	ofstream outfile(fileName);
	outfile<<".model zero\n";

	//PI names
	outfile<<".inputs";
	for(int i=0;i<I;i++)
		outfile<<" "<<*(_piList[i]->getSym());
	outfile<<"\n";

	//PO names
	outfile<<".outputs";
	assert(_miterList.size() == _poListG1.size());
	for(int i=0; i< _miterList.size();++i)
		outfile<<" o"<<i;
	outfile<<"\n";

	//CONST0
	outfile<<".names const0"<<"\n";

	//PO
	for(int i=0; i<_miterList.size();++i){
		outfile<<".names const0 o"<<i<<endl;
		outfile<<1<<" "<<1<<endl;
	}
	outfile<<".end"<<endl;
}
*/

void
CirMgr::writeAag(const string& fileName, CirType cirType)
{
	const int& Aw = (cirType == GOLDEN)? AG : AR;
	const vector<CirPoGate*>& _poList1 = 
	(cirType == GOLDEN)? _poListG1 : _poListR1;
	const vector<CirPoGate*>& _poList2 = 
	(cirType == GOLDEN)? _poListG2 : _poListR2;
	const vector<CirGate*>& _dfsList =
	(cirType == GOLDEN)? _dfsListG : _dfsListR;

	ofstream outfile(fileName);
	outfile<<"aag "<<I+Aw<<" "<<I<<" "<<0<<" "<<O*2<<" "<<Aw<<endl; 
	//Modified for CAD

	//PI
	for(int i=0;i<I;i++)
		outfile<<(_piList[i]->getID())*2<<endl;
	//PO
	for(int i=0;i<O;i++){
		int k = i;
		if(cirType == REVISED){
			assert(_poListG1[i]->getSym()!=NULL);
			const string& name = *(_poListG1[i] -> getSym());
			assert(name!="");
			assert(_GSymToGateId.find(name) != _GSymToGateId.end());
			k = _GSymToGateId[name];
			assert(k < _poListR1.size());
			assert( *(_poListG1[i] -> getSym()) == 
					*(_poListR1[k] -> getSym()));
		}

		int id= _poList1[k] -> getFaninGateID(0) ;
		bool phase = _poList1[k]-> getFaninGatePhase(0);
		outfile<<id*2+phase<<endl;
	}
	for(int i=0;i<O;i++){
		int k = i;
		if(cirType == REVISED){
			assert(_poListG2[i]->getSym()!=NULL);
			const string& name = *(_poListG2[i] -> getSym());
			assert(name!="");
			assert(_GSymToGateId.find(name) != _GSymToGateId.end());
			k = _GSymToGateId[name];
			assert(k < _poListR2.size());
			assert( *(_poListG2[i] -> getSym()) == 
					*(_poListR2[k] -> getSym()));
		}
		int id= _poList2[k] ->getFaninGateID(0) ;
		bool phase = _poList2[k]-> getFaninGatePhase(0);
		outfile<<id*2+phase<<endl;
	}
	//AIG
	for(int i=0;i<_dfsList.size();i++){
		if(_dfsList[i]!=NULL && (_dfsList[i] -> getType()==AIG_GATE
			|| _dfsList[i]-> getType()==LATCH_GATE)){
			outfile<<(_dfsList[i]-> getID())*2;
			for(int j=0;j<2;j++){
				outfile<<" ";
				int id= _dfsList[i] -> getFaninGateID(j) ;
				bool phase = _dfsList[i]-> getFaninGatePhase(j);
				outfile<<id*2+phase;
			}
			outfile<<endl;
		}
	}
	//symbol
	//Since we print rf PO as the same order of gf PO according to their 
	//symbol, so we can print symbol by gf
	for(int i=0;i<I;i++){
		if(_piList[i]->getSym()!=NULL )
			outfile<<"i"<<i<<" "<<*(_piList[i]->getSym())<<endl;
	}
	for(int i=0;i<O*2;i++){
		if(i<O){
			if(_poListG1[i]->getSym()!=NULL)
				outfile<<"o"<<i<<" "<<*(_poListG1[i]->getSym())<<endl;
		}
		else{
			if(_poListG2[i-O]->getSym()!=NULL)
				outfile<<"o"<<i<<" "<<*(_poListG2[i-O]->getSym())<<endl;
		}
	}
}


void
CirMgr::writeBlif(const string& fileName, CirType cirType)
{
	const vector<CirPoGate*>& _poList1 = 
	(cirType == GOLDEN)? _poListG1 : _poListR1;
	const vector<CirPoGate*>& _poList2 = 
	(cirType == GOLDEN)? _poListG2 : _poListR2;
	const vector<CirGate*>& _dfsList =
	(cirType == GOLDEN)? _dfsListG : _dfsListR;
	const vector<CirGate*>& _gateList =
	(cirType == GOLDEN)? _gateListG : _gateListR;

	ofstream outfile(fileName);
	outfile<<".model "<<((cirType ==  GOLDEN)? "gf" : "rf")<<"\n";

	//PI names
	outfile<<".inputs";
	for(int i=0;i<I;i++)
		outfile<<" "<<*(_piList[i]->getSym());
	outfile<<"\n";
	
	//PO names
	outfile<<".outputs";
	for(int i=0;i<O;i++){
		int k = i;
		if(cirType == REVISED){
			assert(_poListG1[i]->getSym()!=NULL);
			const string& name = *(_poListG1[i] -> getSym());
			assert(name!="");
			assert(_GSymToGateId.find(name) != _GSymToGateId.end());
			k = _GSymToGateId[name];
			assert(k < _poListR1.size());
			assert( *(_poListG1[i] -> getSym()) == 
					*(_poListR1[k] -> getSym()));
		}
		outfile<<" "<<*(_poList1[k] -> getSym());
	}
	for(int i=0;i<O;i++){
		int k = i;
		if(cirType == REVISED){
			assert(_poListG2[i]->getSym()!=NULL);
			const string& name = *(_poListG2[i] -> getSym());
			assert(name!="");
			assert(_GSymToGateId.find(name) != _GSymToGateId.end());
			k = _GSymToGateId[name];
			assert(k < _poListR2.size());
			assert( *(_poListG2[i] -> getSym()) == 
					*(_poListR2[k] -> getSym()));
		}
		outfile<<" "<<*(_poList2[k] -> getSym());
	}
	outfile<<"\n";
	outfile<<".names aig0"<<"\n";
	//AIG
	for(int i=0;i<_dfsList.size();i++){
		if(_dfsList[i]!=NULL && (_dfsList[i] -> getType()==AIG_GATE
			|| _dfsList[i]-> getType()==LATCH_GATE)){
			outfile<<".names";
			for(int j=0;j<2;++j){
				int id= _dfsList[i] -> getFaninGateID(j) ;
				if(_gateList[id]->getType() == PI_GATE)
					outfile<<" "<<*(_gateList[id]->getSym());
				else{
					outfile<<" aig";
					outfile<<id;
				}
			}
			outfile<<" aig"<<(_dfsList[i]-> getID());
			outfile<<"\n";
			
			for(int j=0; j<2;++j){
				bool phase = _dfsList[i]-> getFaninGatePhase(j);
				outfile<<!phase;
			}
			outfile<<" "<<1<<"\n";
		}
	}

	//PO 
	for(int i=0;i<O;i++){
		int k = i;
		if(cirType == REVISED){
			assert(_poListG1[i]->getSym()!=NULL);
			const string& name = *(_poListG1[i] -> getSym());
			assert(name!="");
			assert(_GSymToGateId.find(name) != _GSymToGateId.end());
			k = _GSymToGateId[name];
			assert(k < _poListR1.size());
			assert( *(_poListG1[i] -> getSym()) == 
					*(_poListR1[k] -> getSym()));
		}
		outfile<<".names";
		int id= _poList1[k] -> getFaninGateID(0) ;
		if(_gateList[id]->getType() == PI_GATE)
			outfile<<" "<<*(_gateList[id]->getSym());
		else{
			outfile<<" aig";
			outfile<<id;
		}
		outfile<<" "<<*(_poList1[k] -> getSym())<<"\n";
		bool phase = _poList1[k]-> getFaninGatePhase(0);
		outfile<<!phase<<" "<<1<<"\n";
	}
	for(int i=0;i<O;i++){
		int k = i;
		if(cirType == REVISED){
			assert(_poListG2[i]->getSym()!=NULL);
			const string& name = *(_poListG2[i] -> getSym());
			assert(name!="");
			assert(_GSymToGateId.find(name) != _GSymToGateId.end());
			k = _GSymToGateId[name];
			assert(k < _poListR2.size());
			assert( *(_poListG2[i] -> getSym()) == 
					*(_poListR2[k] -> getSym()));
		}
		outfile<<".names";
		int id= _poList2[k] -> getFaninGateID(0) ;
		if(_gateList[id]->getType() == PI_GATE)
			outfile<<" "<<*(_gateList[id]->getSym());
		else{
			outfile<<" aig";
			outfile<<id;
		}
		outfile<<" "<<*(_poList2[k] -> getSym())<<"\n";
		bool phase = _poList2[k]-> getFaninGatePhase(0);
		outfile<<!phase<<" "<<1<<"\n";

	}
	outfile<<".end"<<"\n";
}

/***Helper functions related to reading circuit ***/
bool 
CirMgr::readHeader(ifstream &fin,CirType cirType){
	string aag;
	int IG=0 ,OG=0;
	if(cirType == REVISED){ IG = I; OG = O; }
	fin>>aag>>M>>I>>L>>O>>A;
	if(cirType == REVISED)
		assert(IG == I && OG == O);
	return true;
}
bool
CirMgr::readInput(ifstream &fin, CirType cirType){
	size_t gateID;
	for(int i=0;i<I;i++){
		fin>>gateID; gateID = gateID>>1; //gateID = gateID/2
		
		if(cirType == GOLDEN){
			
			CirPiGate* pi = new CirPiGate(gateID);
			_piList.push_back(pi);
			
			if(_gateListG.size()<gateID+1) 
				_gateListG.resize(gateID+1,NULL);
			_gateListG[gateID] = pi;
			
			if(_gateListR.size()<gateID+1) 
				_gateListR.resize(gateID+1,NULL);
			_gateListR[gateID] = pi; //we also let _gateListR store it
		}
	}
	return true;
}
bool
CirMgr::readOutput(ifstream &fin, CirType cirType){
	size_t gateID,var;
	
	vector<CirPoGate*> &_poList1 = 
	(cirType == GOLDEN)? _poListG1 : _poListR1;
	vector<CirGate*>& _gateList = 
	(cirType == GOLDEN)? _gateListG : _gateListR;

	for(int i=0;i<O;i++){
		gateID = M+1+i;
		fin>>var;
		
		CirPoGate *po = new CirPoGate(gateID,cirType);
		po -> setFanin(var);
				
		_poList1.push_back(po);
		if(_gateList.size()<gateID+1) 
			_gateList.resize(gateID+1,NULL);
		_gateList[gateID] = po;
	}
	return true;
}

bool
CirMgr::readAIG(ifstream &fin, CirType cirType){
	size_t gateID,var1,var2;
	
	vector<CirGate*>& _gateList = 
	(cirType == GOLDEN)? _gateListG : _gateListR;

	for(int i=0;i<A;i++){
		fin>>gateID>>var1>>var2; 
		gateID=gateID>>1;

		CirGate *a = new CirAigGate(gateID,cirType);
		a -> setFanin(var1);
		a -> setFanin(var2);
		
		if(_gateList.size()<gateID+1) 
			_gateList.resize(gateID+1,NULL);
		_gateList[gateID] = a;
	}
	return true;
}
bool
CirMgr::readLatch(ifstream &fin, CirType cirType){
	size_t gateID,var1,var2;
	
	vector<CirGate*>& _gateList = 
	(cirType == GOLDEN)? _gateListG : _gateListR;

	for(int i=0;i<L;i++){
		fin>>gateID>>var1>>var2;
		assert(var1 == var2);
		gateID=gateID>>1;
		
		CirGate *a = new CirLatchGate(gateID,cirType);
		a -> setFanin(1);
		a -> setFanin(1);

		if(_gateList.size()<gateID+1) 
			_gateList.resize(gateID+1,NULL);
		_gateList[gateID] = a;
	}
	return true;
}

bool
CirMgr::readSym(ifstream &fin, CirType cirType){
	string pos, name;
	int  id;
	getline(fin,pos); //to remove the '\n' of previos line
	
	while(getline(fin,pos,' ') && getline(fin,name)){
		if(pos[0]!='i' && pos[0]!='o') break;
		if(pos[0]=='i'){
			id = stoi(pos.erase(0,1)); 
			assert(id<_piList.size());

			size_t gid = _piList[id] -> getID();
			if(cirType == GOLDEN){
				_piList[id] -> setSym(name);
				_GSymToGateId[name] = gid;
			}
			else if(cirType == REVISED){
				if(_RGateIdToSym.size()<gid+1)
					_RGateIdToSym.resize(gid+1,"");
				_RGateIdToSym[gid] = name;
			}
		}
		else if(pos[0]=='o'){
			id = stoi(pos.erase(0,1));
			if(cirType == GOLDEN){
				_poListG1[id] -> setSym(name);
			}
			else if(cirType == REVISED){
				_poListR1[id] -> setSym(name);
				_GSymToGateId[name] = id; //watch out!!!
			}
		}
	}
	if(cirType == GOLDEN)
		assert(_GSymToGateId.size() == I);
	else if(cirType == REVISED){
		assert(_RGateIdToSym.size() == _piList.size()+1);
		assert(_GSymToGateId.size() == I+O);
	}
	return true;
}
bool
CirMgr::readComment(ifstream &fin){
	string comment;
	while(fin>>comment){}
	return true;
}
void
CirMgr::connect(CirType cirType){

	vector<CirGate*>& _gateList = 
	(cirType == GOLDEN)? _gateListG : _gateListR;

	//set const 0
	_gateList[0] = _const0;

	//set fanin/fanout, check floating
	for(int i=0;i<_gateList.size();i++){
		CirGate *g = _gateList[i];
		if(g == NULL) continue;
		else if(g->getType() == PO_GATE || g-> getType() == AIG_GATE
				|| g->getType() ==  LATCH_GATE ){
			
			for(int i=0;i<g->FaninSize();i++){
				size_t n = g->getFanin(i);
				size_t id = n/2; size_t phase = n%2;
				
				//map to the same PI with Gf
				if(cirType == REVISED && id <= _piList.size() && id != 0){
				    assert(_gateList[id] -> getType() == PI_GATE);
					assert(id < _RGateIdToSym.size());
					const string& name = _RGateIdToSym[id];
					assert(name!="");
					assert(_GSymToGateId.find(name)!=_GSymToGateId.end());
					id = _GSymToGateId[name];
					assert(_gateList[id] -> getType() == PI_GATE);
				}
				
				//floating case: b,c,d
				if(id>= _gateList.size())
					_gateList.resize(id+1,NULL);
				
				if(_gateList[id] == NULL 
				   || _gateList[id]-> getType() == UNDEF_GATE) {
					
					if(_floatList.empty()||_floatList.back()!=g)
						_floatList.push_back(g);
					if(_gateList[id]== NULL) 
						_gateList[id] = new CirUndefGate(id,cirType);
				}
				
				//even undefined gate has to set fanout
				_gateList[id] -> setFanout(g,phase);		
				g -> setFanin(_gateList[id],phase,i);
			}
		}
	}
	//check undefined:case a,c
	for(int i=0;i<_gateList.size();i++){
		CirGate *g = _gateList[i];
		if(g == NULL) continue;
		else if(g -> getType() == PI_GATE || g-> getType()== AIG_GATE
				|| g->getType() == LATCH_GATE){
			
			if(g -> FanoutSize() == 0)
				_unuseList.push_back(g);
		}
	}
}
void
CirMgr::dfsCircuitOne(CirType cirType){
	
	CirGate::setglobalRef();
	
	vector<CirPoGate*>& _poList1  = 
	(cirType == GOLDEN)? _poListG1 : _poListR1;
	vector<CirGate*>& _dfsList =
	(cirType == GOLDEN)? _dfsListG : _dfsListR;

	assert(_dfsList.empty());
	for(int i=0;i<_poList1.size();i++)
		_poList1[i] -> dfs(_dfsList);
}

/***Helper functions related to building circuit two***/

void
CirMgr::dfsCircuitTwo(CirType cirType){
	
	CirGate::setglobalRef();

	vector<CirPoGate*>& _poList1  = 
	(cirType == GOLDEN)? _poListG1 : _poListR1;
	vector<CirPoGate*>& _poList2  = 
	(cirType == GOLDEN)? _poListG2 : _poListR2;
	vector<CirGate*>& _dfsList =
	(cirType == GOLDEN)? _dfsListG : _dfsListR;

	_dfsList.clear();
	for(int i=0;i<_poList1.size();i++)
		_poList1[i] -> dfs(_dfsList);
	for(int i=0;i<_poList2.size();i++)
		_poList2[i] -> dfs(_dfsList);

	int& Aw =  (cirType == GOLDEN)? AG : AR;
	Aw = 0;
	for(int i=0;i<_dfsList.size();i++){
		if(_dfsList[i]->getType() == AIG_GATE ||
		   _dfsList[i]->getType() == LATCH_GATE)
			Aw++;
	}
}

void
CirMgr::dfsBuildCircuitTwo(CirType cirType){

	vector<CirGate*>& _dfsList = 
	(cirType == GOLDEN) ? _dfsListG : _dfsListR;
	vector<CirGate*>& _gateList = 
	(cirType == GOLDEN) ? _gateListG : _gateListR;
	vector<size_t>& _CktOneToCktTwo =  
	(cirType == GOLDEN) ? _CktOneToCktTwoG : _CktOneToCktTwoR; 

	for(int i=0; i<_dfsList.size();++i){
		if(_dfsList[i]!=NULL){
			
			if(_dfsList[i]->getType() == PI_GATE ||
			   _dfsList[i]->getType() == LATCH_GATE ||
			   _dfsList[i]->getType() == CONST_GATE){
				
				size_t c1_Id = _dfsList[i]->getID(); //c1
				_CktOneToCktTwo[c1_Id] = 0; //map c1 to c2
			}
			else if(_dfsList[i]->getType() == AIG_GATE){
				size_t c1_Id = _dfsList[i]->getID(); //c1
				CirGate *c1 = _gateList[c1_Id];
				
				size_t a1_Id = c1 -> getFaninGateID(0);
				size_t b1_Id = c1 -> getFaninGateID(1);
				
				size_t a2_Id = _CktOneToCktTwo[a1_Id];
				size_t b2_Id = _CktOneToCktTwo[b1_Id];

				bool a1_phase = c1 -> getFaninGatePhase(0);
				bool b1_phase = c1 -> getFaninGatePhase(1);
			
				
				bool a2_phase = (_gateList[a1_Id] -> getType() == AIG_GATE
				|| _gateList[a1_Id] -> getType() == LATCH_GATE ) ? 1 : 0;
				bool b2_phase = (_gateList[b1_Id] -> getType() == AIG_GATE
				|| _gateList[b1_Id] -> getType() == LATCH_GATE ) ? 1 : 0;
				
				//Construct g1
				size_t g1_Id = _gateList.size();
				CirGate  *g1 = new CirAigGate(g1_Id,cirType);
				
				_gateList[a1_Id] -> setFanout(g1,a1_phase);
				g1 -> setFanin(_gateList[a1_Id],a1_phase,0);
				
				_gateList[b2_Id] -> setFanout(g1,b2_phase);
				g1 -> setFanin(_gateList[b2_Id],b2_phase,1);
				
				
				if(_gateList.size()<g1_Id+1) 
					_gateList.resize(g1_Id+1,NULL);
				_gateList[g1_Id] = g1;

				//Construct g2
				size_t g2_Id = _gateList.size();
				CirGate  *g2 = new CirAigGate(g2_Id,cirType);

				_gateList[b1_Id] -> setFanout(g2,!b1_phase);
				g2 -> setFanin(_gateList[b1_Id],!b1_phase,0);
				
				_gateList[b2_Id] -> setFanout(g2,!b2_phase);
				g2 -> setFanin(_gateList[b2_Id],!b2_phase,1);
				
				if(_gateList.size()<g2_Id+1) 
					_gateList.resize(g2_Id+1,NULL);
				_gateList[g2_Id] = g2;

				//Construct g3
				size_t g3_Id = _gateList.size();
				CirGate *g3 = new CirAigGate(g3_Id,cirType);

				_gateList[g2_Id] -> setFanout(g3,1);
				g3 ->  setFanin(_gateList[g2_Id],1,0);
				
				_gateList[a2_Id] -> setFanout(g3,a2_phase);
				g3 ->  setFanin(_gateList[a2_Id],a2_phase,1);
				
				if(_gateList.size()<g3_Id+1) 
					_gateList.resize(g3_Id+1,NULL);
				_gateList[g3_Id] = g3;

				//Construct c2
				size_t c2_Id = _gateList.size();
				CirGate *c2 = new CirAigGate(c2_Id,cirType);

				_gateList[g1_Id] -> setFanout(c2,1);
				c2  -> setFanin(_gateList[g1_Id],1,0);
				
				_gateList[g3_Id] -> setFanout(c2,1);
				c2  -> setFanin(_gateList[g3_Id],1,1);
				
				if(_gateList.size()<c2_Id+1) 
					_gateList.resize(c2_Id+1,NULL);
				_gateList[c2_Id] = c2;

				//map c1 to c2
				_CktOneToCktTwo[c1_Id] = c2_Id;
			}
			else if(_dfsList[i]->getType() == PO_GATE){
				size_t c1_Id = _dfsList[i]->getID(); //c1
				CirGate *c1 = _gateList[c1_Id];
				
				size_t a1_Id = c1 -> getFaninGateID(0);
				size_t a2_Id = _CktOneToCktTwo[a1_Id];
				
				bool a2_phase = (_gateList[a1_Id] -> getType() == AIG_GATE
				|| _gateList[a1_Id] -> getType() == LATCH_GATE ) ? 1 : 0;
				
				size_t c2_Id = _gateList.size();
				CirGate *c2 = new CirPoGate(c2_Id,cirType);
				
				_gateList[a2_Id] -> setFanout(c2,a2_phase);
				c2  -> setFanin(_gateList[a2_Id],a2_phase,0);
				
				if(_gateList.size()<c2_Id+1) 
					_gateList.resize(c2_Id+1,NULL);
				_gateList[c2_Id] = c2;

				//map c1 to c2
				_CktOneToCktTwo[c1_Id] = c2_Id;
			}
		}
	}
}
void
CirMgr::setCircuitTwoPO(CirType cirType){
	vector<CirPoGate*>& _poList1 = 
	(cirType == GOLDEN)? _poListG1 : _poListR1;
	vector<CirPoGate*>& _poList2 = 
	(cirType == GOLDEN)? _poListG2 : _poListR2;
	vector<CirGate*>& _gateList = 
	(cirType == GOLDEN)? _gateListG : _gateListR;
	vector<size_t>& _CktOneToCktTwo =  
	(cirType == GOLDEN) ? _CktOneToCktTwoG : _CktOneToCktTwoR; 

	_poList2.resize(_poList1.size(),NULL);
	for(int i=0; i<_poList1.size(); ++i){
		size_t c1_Id = _poList1[i] -> getID();
		size_t c2_Id = _CktOneToCktTwo[c1_Id];
		assert(_gateList[c2_Id] -> getType() == PO_GATE);
		_poList2[i] = (CirPoGate *)_gateList[c2_Id];
		
		assert(_poList1[i]->getSym()!=NULL);
		string name = *(_poList1[i]->getSym()) + "_2";
		_poList2[i] -> setSym(name);

		if(cirType == REVISED)
			_GSymToGateId[name] = i; //watch out!!!
	}
}

/***Helper function related to random simulation***/
void
CirMgr::randSignal()
{
	//Generate random signal
	_sigList.clear();
	size_t randint;
	for(int i=0;i<2;i++){
		for(int j=0;j<_piList.size();j++){
			//1st gernerated pattern will be leftmost bit of signal 
			randint = rand() % INT_MAX;
			if(i==0) 
				_sigList.push_back(randint);
			else
				_sigList[j] = (_sigList[j]<<32) + randint;
		}
	}

	//Set PI signal
	assert(_sigList.size()==_piList.size());
	for(int i=0;i<_piList.size();i++)
		_piList[i]->setSignal(_sigList[i]);

}

void
CirMgr::simulate(CirType cirType)
{
	int id0,id1; bool ph0,ph1; size_t w0,w1;

	vector<CirGate*>& _gateList =
	(cirType == GOLDEN) ? _gateListG : _gateListR;
	vector<CirGate*>& _dfsList =
	(cirType == GOLDEN) ? _dfsListG : _dfsListR;

	_gateList[0]->setSignal(0);
	
	for(int i=0;i<_dfsList.size();i++){
		CirGate *g = _dfsList[i];
		if(g->getType() == AIG_GATE || g->getType() == LATCH_GATE){

			id0 = g->getFaninGateID(0); ph0 = g->getFaninGatePhase(0);
			id1 = g->getFaninGateID(1); ph1 = g->getFaninGatePhase(1);
			
			w0 = _gateList[id0]->getSignal(ph0);
			w1 = _gateList[id1]->getSignal(ph1);
			g->setSignal( w0 & w1 );
		}
		else if(g->getType()==PO_GATE){
		
			id0 = g->getFaninGateID(0); ph0 = g->getFaninGatePhase(0);
			
			w0 = _gateList[id0]->getSignal(ph0);
			g->setSignal(w0);
		}
		else if(g->getType()==UNDEF_GATE)
			g->setSignal(0);
	}
}

bool 
CirMgr::isCompatibleEQ(const string& outfile){

	assert(_poListG2.size() == _poListR2.size());
	for(int i=0; i< _poListG2.size(); ++i){
		//retrieve gf's corresponding rf po
		assert(_poListG2[i]->getSym()!=NULL);
		const string& name = *(_poListG2[i] -> getSym());
		assert(name!="");
		assert(_GSymToGateId.find(name) != _GSymToGateId.end());
		int k = _GSymToGateId[name];
		assert(k < _poListR2.size());
		assert( *(_poListG2[i] -> getSym()) == *(_poListR2[k] -> getSym()));
		
		//check for cases (0,x),(1,x)
		size_t n = 	_poListG2[i]->getSignal(true) &
				  	_poListR2[k]->getSignal(false);
		if(n){
			for(int shift = 0; shift < 64;  ++shift){
				if( (n>>shift) & 1){
					ofstream fout(outfile);
					fout<<"NEQ"<<endl;
					for(int j=0; j<_piList.size();++j){
						fout<<*(_piList[j]->getSym())<<" ";
						fout<< ((_sigList[j]>>shift) & 1)<<endl;
						//fout<< ((_sigList[j]>>shift) & 1);
					}
					//fout<<1<<endl;
					fout<<endl;
					return false;
				}
			}
		}
		else{
			n = _poListG2[i]->getSignal(true) & 
				_poListR2[k]->getSignal(true);
			
			if(n == 0) continue; //case (x,0),(x,1),(x,x)
			else{
				//There is some (0,0), (0,1), (1,0), (1,1)
				for(int shift =0; shift<64; ++shift){
					if( (n>>shift) & 1){
						
						assert(*(_poListG1[i]-> getSym())+"_2" == 
							   *(_poListG2[i]-> getSym()));
						assert(*(_poListG1[i]->getSym()) == 
							   *(_poListR1[k]->getSym()));

						size_t gbit1 = _poListG1[i] -> getSignal();
						size_t rbit1 = _poListR1[k] -> getSignal();
						if(((gbit1>>shift) & 1) != ((rbit1>>shift) & 1)){
							//case (0,1), (1,0)
							ofstream fout(outfile);
							fout<<"NEQ"<<endl;
							for(int j=0; j<_piList.size();++j){
								fout<<*(_piList[j]->getSym())<<" ";
								fout<< ((_sigList[j]>>shift) & 1)<<endl;
								//fout<< ((_sigList[j]>>shift) & 1);
							}
							//fout<<2<<endl;
							fout<<endl;
							return false;
						}
					}
				}
			}
		}
	}
	return true;
}

/*
void
CirMgr::test_randSignal()
{
	//random PI _bool, _value3
	_boolList.clear();
	_value3List.clear();
	bool randbool;
	cout<<"Simulating pattern: ";
	for(int i=0; i<_piList.size(); i++){
		randbool = rand() % 2;
		cout<<randbool;
		_boolList.push_back(randbool);
		_value3List.push_back(Value3(randbool,0));
	}
	cout<<endl;

	//Set PI _bool, _valu3
	assert(_boolList.size() ==_piList.size() &&
		   _value3List.size() == _piList.size());

	for(int i=0;i<_piList.size();i++){
		_piList[i]->setBool(_boolList[i]);
		_piList[i]->setValue3(_value3List[i]);
	}
}
*/

/*
void
CirMgr::test_simulate(CirType cirType)
{
	int id0,id1; bool ph0,ph1; bool b0,b1; Value3 v0,v1;

	vector<CirGate*>& _gateList =
	(cirType == GOLDEN) ? _gateListG : _gateListR;
	vector<CirGate*>& _dfsList =
	(cirType == GOLDEN) ? _dfsListG : _dfsListR;
	vector<CirPoGate*> _poList1 = 
	(cirType == GOLDEN) ? _poListG1 : _poListR1;
	vector<CirPoGate*> _poList2 = 
	(cirType == GOLDEN) ? _poListG2 : _poListR2;
	
	_gateList[0]->setBool(0);
	_gateList[0]->setValue3(Value3(0,0));

	for(int i=0;i<_dfsList.size();i++){
		CirGate *g = _dfsList[i];
		if(g->getType()  == AIG_GATE || g->getType()  ==  LATCH_GATE){
			
			id0 = g->getFaninGateID(0); ph0 = g->getFaninGatePhase(0);
			id1 = g->getFaninGateID(1); ph1 = g->getFaninGatePhase(1);
			
			b0 = _gateList[id0]->getBool(ph0);
			b1 = _gateList[id1]->getBool(ph1);
			g->setBool( b0 & b1 );

			if(g->getType() == AIG_GATE){
				v0 = _gateList[id0]->getValue3(ph0);
				v1 = _gateList[id1]->getValue3(ph1);
				g->setValue3(v0 & v1);
			}
			else if(g->getType() == LATCH_GATE)
				g->setValue3(Value3(1,1));
		}
		else if(g->getType()==PO_GATE){

			id0 = g->getFaninGateID(0); ph0 = g->getFaninGatePhase(0);

			b0 = _gateList[id0]->getBool(ph0);
			g->setBool(b0);

			v0 = _gateList[id0]->getValue3(ph0);
			g->setValue3(v0);
		}
		else if(g->getType()==UNDEF_GATE){
			g->setBool(0);
			g->setValue3(Value3(1,1));
		}
	}

	assert(_poList1.size() ==  _poList2.size());
	for(int i=0; i< _poList1.size(); ++i){
		if( _poList2[i]->getBool() == 1)
			assert(_poList1[i]->getValue3()._dontCare == 1);
		else{
			assert(_poList1[i]->getValue3()._dontCare == 0);
			assert(_poList1[i]->getValue3()._bit == _poList1[i]->getBool());
		}
	}
}
*/

/*****Helper function related to build Cmiter****/
/*
void 
CirMgr::buildEachCMiter(){
	for(int i=0;i<_poListG1.size();++i){
		int k = i;
		assert(_poListG1[i]->getSym()!=NULL);
		const string& name = *(_poListG1[i] -> getSym());
		assert(name!="");
		assert(_GSymToGateId.find(name) != _GSymToGateId.end());
		k = _GSymToGateId[name];
		assert(k < _poListR1.size());
		assert( *(_poListG1[i] -> getSym()) == 
				*(_poListR1[k] -> getSym()));
		assert( *(_poListG2[i] -> getSym()) == 
				*(_poListR2[k] -> getSym()));
		assert( *(_poListG2[i] -> getSym()) == 
				*(_poListG1[i] -> getSym())+"_2");
		assert( *(_poListR2[k] -> getSym()) == 
				*(_poListR1[k] -> getSym())+"_2");
		
		size_t g1_Id = _poListG1[i] -> getFaninGateID(0);
		bool   g1_phase = _poListG1[i] -> getFaninGatePhase(0);
		
		size_t g2_Id = _poListG2[i] -> getFaninGateID(0);
		bool   g2_phase = _poListG2[i] -> getFaninGatePhase(0);

		size_t r1_Id  = _poListR1[k] -> getFaninGateID(0);
		bool r1_phase = _poListR1[k] -> getFaninGatePhase(0);
		
		size_t r2_Id = _poListR2[k] -> getFaninGateID(0);
		bool r2_phase = _poListR2[k] -> getFaninGatePhase(0);

		//c1 = g1!r1
		size_t c1_Id = _gateListM.size();
		CirGate *c1 = new CirMiterGate(c1_Id,MITER);
				
		c1 -> setFanin(_gateListG[g1_Id],g1_phase,0);
		c1 -> setFanin(_gateListR[r1_Id],!r1_phase,1);
	
		if(_gateListM.size()<c1_Id+1) 
			_gateListM.resize(c1_Id+1,NULL);
		_gateListM[c1_Id] = c1;
		
		//c2 = !g1r1
		size_t c2_Id = _gateListM.size();
		CirGate *c2 = new CirMiterGate(c2_Id,MITER);
				
		c2 -> setFanin(_gateListG[g1_Id],!g1_phase,0);
		c2 -> setFanin(_gateListR[r1_Id],r1_phase,1);
	
		if(_gateListM.size()<c2_Id+1) 
			_gateListM.resize(c2_Id+1,NULL);
		_gateListM[c2_Id] = c2;

		//c3 = !c1 !c2
		size_t c3_Id = _gateListM.size();
		CirGate *c3 = new CirMiterGate(c3_Id,MITER);
				
		c3 -> setFanin(_gateListM[c1_Id],1,0);
		c3 -> setFanin(_gateListM[c2_Id],1,1);
	
		if(_gateListM.size()<c3_Id+1) 
			_gateListM.resize(c3_Id+1,NULL);
		_gateListM[c3_Id] = c3;

		//c4 = !r2 c3
		size_t c4_Id = _gateListM.size();
		CirGate *c4 = new CirMiterGate(c4_Id,MITER);
				
		c4 -> setFanin(_gateListR[r2_Id],!r2_phase,0);
		c4 -> setFanin(_gateListM[c3_Id],0,1);
	
		if(_gateListM.size()<c4_Id+1) 
			_gateListM.resize(c4_Id+1,NULL);
		_gateListM[c4_Id] = c4;

		//cMiter = !g2 !c4
		size_t cMiter_Id = _gateListM.size();
		CirGate *cMiter = new CirMiterGate(cMiter_Id,MITER);
				
		cMiter -> setFanin(_gateListG[g2_Id],!g2_phase,0);
		cMiter -> setFanin(_gateListM[c4_Id],1,1);
	
		if(_gateListM.size()<cMiter_Id+1) 
			_gateListM.resize(cMiter_Id+1,NULL);
		_gateListM[cMiter_Id] = cMiter;

		_miterList.push_back(cMiter);
	}
}

void 
CirMgr::dfsAllMiter(){
	
	CirGate::setglobalRef();
	
	assert(_dfsListM.empty());
	for(int i=0;i<_miterList.size();i++)
		_miterList[i] -> dfs(_dfsListM);
}
*/
