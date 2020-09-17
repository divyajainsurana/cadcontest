/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <string>
#include <iostream>
#include "cirDef.h"
#include <cassert>

using namespace std;

class CirGate;
class CirGateV;
class CirPiGate;
class CirPoGate;
class CirAigGate;
class CirLatchGate;
class CirMiterGate;
class CirConstGate;
class CirUndefGate;
//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGateV
{
	friend CirGate; 
	friend CirAigGate;
	friend CirPiGate;
	friend CirPoGate;
	friend CirLatchGate;
	friend CirMiterGate;
	friend CirConstGate;
	#define NEG 0x1
	CirGateV(){}
	CirGateV(CirGate *g, size_t phase):
		_gateV(size_t(g) + phase) {}
	~CirGateV(){}
	CirGate* gate() const{
		return (CirGate*)(_gateV & ~size_t(NEG));
	}
	bool isInv() const { return (_gateV & NEG); } //return 1 if inverted
	size_t _gateV;
};
class CirGate
{
public:
   CirGate(){}
   CirGate(size_t gateID, GateType type,CirType cirType):
	   _gateID(gateID),_gateType(type),_cirType(cirType), 
	   _signal(0), //_bool(0), _value3(0,1),
	   _faninList(NULL), _fanoutList(NULL),_sym(NULL),_ref(0){}
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const;
   GateType getType() const { return _gateType; }
   CirType  getCirType() const {  return _cirType;}
   size_t getID() const { return _gateID; }

   // Printing functions
   virtual void printGate() const = 0;
   //void reportGate() const;
   void reportFanin(int level) const;
   void recurFanin(int level,int space) const;
   void reportFanout(int level) const;
   void recurFanout(int level,int space) const;

   //Fanin related
   void setFanin(size_t var); //store size_t(id  of gate)
   void setFanin(CirGate *g,size_t phase,int i);//store CirGateV   
   size_t FaninSize(){ assert(_faninList!=NULL); return _faninList -> size();}
   size_t getFanin(int i); //return _faninList -> at(i) (id of gate),called when circuit not connected
   //called after circuit  connected
   size_t getFaninGateID(int i) { return _faninList->at(i)->gate()->getID(); }
   bool  getFaninGatePhase(int i) { return _faninList-> at(i)->isInv(); }
   CirType getFaninCirType(int i) { 
	   return _faninList->at(i)->gate()->getCirType();
   }
   //Fanout related
   void setFanout(CirGate *g,size_t phase);
   size_t FanoutSize(){ 
	   if(_fanoutList==NULL) return 0;
	   else return _fanoutList -> size();
   }

   //signal related
   void setSignal(const size_t& sig){ _signal = sig; }
   size_t getSignal(const bool& phase=0) const{ 
	   if(phase) return ~_signal;
	   else return _signal;
   }
   //related to test_random
   /*
   void setBool(const bool& b) { _bool = b; }
   bool getBool(const bool& phase=0) const{
	   if(phase) return  !_bool;
	   else return _bool;
   }
   void setValue3(const Value3& v) { _value3 = v;} 
   Value3 getValue3(const bool& phase=0) const{ 
	   if(phase) return ~_value3;
	   else return _value3; 
   }
   */

   //sym related
   void setSym(string &name) { _sym = new string(name);  }
   string * getSym() { return _sym; }
   
   //dfs  related
   static void setglobalRef(){ _globalRef++; }
   void dfs(vector<CirGate*> &_dfsList);

private:
   //gate related
   size_t _gateID;
   GateType _gateType;
   CirType _cirType;

   //signal related
   size_t _signal;
   //related to test_random
   //bool _bool;
   //Value3 _value3;

   //dfs related
   size_t _ref;
   static size_t _globalRef;
  

protected:
   string  *_sym;
   vector<CirGateV*> *_faninList;
   vector<CirGateV*> *_fanoutList;
};

class CirPiGate: public CirGate
{
public:
	CirPiGate(){}
	CirPiGate(size_t gateID):CirGate(gateID,PI_GATE,SHARED) {}
	~CirPiGate() {
		assert(_faninList == NULL);
		if(_fanoutList!=NULL) {
			for(int i=0; i<_fanoutList->size();++i)
				delete _fanoutList ->at(i);
			delete _fanoutList;
		}
		if(_sym!=NULL) delete _sym;
	}

	void printGate()const{ 
		cout<<" PI  "<<getID();
		if(_sym!=NULL) cout<<" ("<<*_sym<<")";
		cout<<endl; 
	}
};

class CirPoGate: public CirGate
{
public:
	CirPoGate(){}
	CirPoGate(size_t gateID,CirType cirType): 
		CirGate(gateID,PO_GATE,cirType) {}
	~CirPoGate() {
		assert(_fanoutList == NULL);
		if(_faninList!=NULL) { 
			for(int i=0;i<_faninList->size();++i)
				delete _faninList -> at(i);
			delete _faninList;
		}
		if(_sym!=NULL) delete _sym;
	}

	void printGate()const{ 
		cout<<" PO  "<<getID()<<" ";
		
		CirGate *g =  _faninList -> at(0) -> gate();
		if(g->getType()==UNDEF_GATE) cout<<"*";
		if(_faninList->at(0)->isInv()) cout<<"!";
		cout<<g->getID();
		
		if(_sym!=NULL) cout<<" ("<<*_sym<<")";
		cout<<endl;
	}
};
class CirAigGate: public CirGate
{
public:
	CirAigGate(){}
	CirAigGate(size_t gateID,CirType cirType):
		CirGate(gateID,AIG_GATE, cirType){}
	~CirAigGate() {
		assert(_sym==NULL);
		if(_faninList!=NULL) {
			for(int i=0;i<_faninList->size();++i)
				delete _faninList -> at(i);
			delete _faninList;
		}
		if(_fanoutList!=NULL) {
			for(int i=0;i<_fanoutList->size();++i)
				delete _fanoutList -> at(i);
			delete _fanoutList;
		}
	}
	
	void printGate()const{
		assert(_sym==NULL);
		cout<<" AIG "<<getID();
		for(int i=0;i<2;i++){
			cout<<" ";
			CirGate *g =  _faninList -> at(i) -> gate();
			if(g->getType()==UNDEF_GATE) cout<<"*";
			if(_faninList->at(i)->isInv()) cout<<"!";
			cout<<g->getID();
		}
		cout<<endl;
	}
};

class CirLatchGate: public CirGate
{
public:
	CirLatchGate(){}
	CirLatchGate(size_t gateID,CirType cirType):
		CirGate(gateID,LATCH_GATE,cirType){}
	~CirLatchGate() {
		assert(_sym==NULL);
		if(_faninList!=NULL) {
			for(int i=0;i<_faninList->size();++i)
				delete _faninList -> at(i);
			delete _faninList;
		}
		if(_fanoutList!=NULL) {
			for(int i=0;i<_fanoutList->size();++i)
				delete _fanoutList -> at(i);
			delete _fanoutList;
		}
	}
	
	void printGate() const{
		assert(_sym==NULL);
		cout<<" LATCH "<<getID();
		for(int i=0;i<1;i++){
			cout<<" ";
			CirGate *g =  _faninList -> at(i) -> gate();
			if(g->getType()==UNDEF_GATE) cout<<"*";
			if(_faninList->at(i)->isInv()) cout<<"!";
			cout<<g->getID();
		}
		cout<<endl;
	}
};

class CirMiterGate: public CirGate
{
public:
	CirMiterGate(){}
	CirMiterGate(size_t gateID,CirType cirType):
		CirGate(gateID,MITER_GATE, cirType){}
	~CirMiterGate() {
		assert(_sym==NULL);
		if(_faninList!=NULL) {
			for(int i=0;i<_faninList->size();++i)
				delete _faninList -> at(i);
			delete _faninList;
		}
		if(_fanoutList!=NULL) {
			for(int i=0;i<_fanoutList->size();++i)
				delete _fanoutList -> at(i);
			delete _fanoutList;
		}
	}
	
	void printGate()const{
		assert(_sym==NULL);
		cout<<" AIG "<<getID();
		for(int i=0;i<2;i++){
			cout<<" ";
			CirGate *g =  _faninList -> at(i) -> gate();
			if(g->getType()==UNDEF_GATE) cout<<"*";
			if(_faninList->at(i)->isInv()) cout<<"!";
			cout<<g->getID();
		}
		cout<<endl;
	}
};


class CirConstGate: public CirGate
{
public:
	CirConstGate(){}
	CirConstGate(size_t gateID):CirGate(gateID,CONST_GATE,SHARED){}

	~CirConstGate(){
		assert(_faninList == NULL);
		if(_fanoutList!=NULL) {
			for(int i=0; i<_fanoutList->size();++i)
				delete _fanoutList->at(i);
			delete _fanoutList;
		}
		if(_sym!=NULL) delete _sym;
	}
	void printGate() const{ cout<<" CONST0"<<endl;}
};
class CirUndefGate: public CirGate
{
public:
	CirUndefGate(){}
	CirUndefGate(size_t gateID,CirType cirType): 
		CirGate(gateID,UNDEF_GATE,cirType){}

	~CirUndefGate(){
		assert(_faninList==NULL);
		assert(_fanoutList==NULL);
		assert(_sym ==NULL);
	}
	void printGate() const{}
};

#endif // CIR_GATE_H
