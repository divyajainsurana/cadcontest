/****************************************************************************
  FileName     [ cirDef.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic data or var for cir package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2010-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_DEF_H
#define CIR_DEF_H

#include <vector>

using namespace std;

class CirGate;
class CirMgr;

typedef vector<CirGate*>           GateList;
typedef vector<unsigned>           IdList;

enum GateType
{
   UNDEF_GATE = 0,
   PI_GATE    = 1,
   PO_GATE    = 2,
   AIG_GATE   = 3,
   CONST_GATE = 4,
   LATCH_GATE = 5, 
   MITER_GATE = 6,
   TOT_GATE
};
enum CirType
{
	GOLDEN 	= 0,
	REVISED = 1,
	MITER	= 2,
	SHARED	= 3
};

class Value3 {
  // This is not dual rail encoding,
  // If the _dontCare is one it means it's X,
  // Otherwise it's decided by _bit
 public:
  Value3() : _bit(0), _dontCare(1) {}
  Value3(bool b, bool d): _bit(b), _dontCare(d) {}
  Value3(const Value3& a) {
    _bit = a._bit;
    _dontCare = a._dontCare;
  }
  Value3 operator & (Value3 a) const {
    if ((_bit == 0 && _dontCare == 0) || a == Value3(0, 0)) 
		return Value3(0, 0);
    else if (a._dontCare || _dontCare) 
		return Value3(0, 1);
    else return Value3(1, 0);
  }
  Value3 operator & (bool a) const {
    if (a == 0) return Value3(0, 0);
    else if (_dontCare) return Value3(0, 1);
    else return Value3(1, 0);
  }
  Value3 operator | (Value3 a) const {
    if ((_bit == 1 && _dontCare == 0) || a == Value3(1,0)) 
		return Value3(1, 0);
    else if (a._dontCare || _dontCare) 
		return Value3(0, 1);
    else return Value3(0, 0);
  }
  Value3 operator | (bool a) const {
    if (a) return Value3(1, 0);
    else if (_dontCare) return Value3(0, 1);
    else return Value3(0, 0);
  }
  Value3 operator ~ () const {
    if (_dontCare) return Value3(0, 1);
    else return Value3(!_bit, 0);
  }
  bool operator == (const Value3& a) const {
    if (_dontCare ^ a._dontCare) return false;
    else if (_dontCare && a._dontCare) return true;
    else if (_bit == a._bit) return true;
    else return false;
  }
  bool operator != (const Value3& a) const {
    return !((*this) == a);
  }
  bool _bit;
  bool _dontCare;
};



#endif // CIR_DEF_H
