// Note: Set ENABLE_LIBYOSYS=1 in Makefile or Makefile.conf to build libyosys.so
// yosys-config --exec --cxx -o demomain --cxxflags --ldflags demomain.cc -lyosys -lstdc++

#include "../../kernel/yosys.h"
#include <string>
#include "cirMgr.h"
#include <fstream>

int main(int argc, char**argv)
{
	Yosys::log_streams.push_back(&std::cout);
	Yosys::log_error_stderr = true;

	Yosys::yosys_setup();
	Yosys::yosys_banner();

	std::string gf(argv[1]);
	std::string rf(argv[2]);
	std::string outfile(argv[3]);
	//std::string dir(argv[1]);
	//std::string outfile(argv[2]);
	
	Yosys::run_pass("read_verilog -overwrite "+gf);
	Yosys::run_pass("read_verilog _DC.v");
	Yosys::run_pass("read_verilog _HMUX.v");
	Yosys::run_pass("flatten");
	Yosys::run_pass("aigmap");
	Yosys::run_pass("setundef -params -anyconst");
	Yosys::run_pass("hierarchy -top \\top");
	Yosys::run_pass("opt_clean");
	Yosys::run_pass("opt_merge");
	Yosys::run_pass("write_aiger -ascii -symbols gf_opt.aag");

	Yosys::run_pass("read_verilog -overwrite "+rf);
	Yosys::run_pass("read_verilog _DC.v");
	Yosys::run_pass("read_verilog _HMUX.v");
	Yosys::run_pass("flatten");
	Yosys::run_pass("aigmap");
	Yosys::run_pass("setundef -params -anyconst");
	Yosys::run_pass("hierarchy -top \\top");
	Yosys::run_pass("opt_clean");
	Yosys::run_pass("opt_merge");
	Yosys::run_pass("write_aiger -ascii -symbols rf_opt.aag");

	CirMgr *cirMgr = new CirMgr;
	
	cirMgr -> readCircuit("gf_opt.aag", GOLDEN);
	cirMgr -> readCircuit("rf_opt.aag", REVISED);

	cirMgr -> buildCircuitTwo(GOLDEN);	
	cirMgr -> buildCircuitTwo(REVISED);	

	if(!cirMgr -> randomSim(outfile,1000)){
		delete cirMgr; cirMgr =  NULL;
		return 0;
	}

	cirMgr -> writeBlif("gf_opt_bit2.blif", GOLDEN);
	cirMgr -> writeBlif("rf_opt_bit2.blif", REVISED);
	
	//cirMgr -> buildCMiter();
	//cirMgr -> writeBlifCMiter("case/"+dir+"cMiter.blif");
   	//cirMgr -> writeBlifAllZero("case/"+dir+"const0.blif");

	delete cirMgr; cirMgr = NULL;

	//Yosys::run_pass("read_aiger case/"+dir+"gf_opt_bit2.aag");
	//Yosys::run_pass("hierarchy -top \\case/"+dir+"gf_opt_bit2.aag");
	//Yosys::run_pass("opt_clean");
	//Yosys::run_pass("opt_merge");
	//Yosys::run_pass("write_aiger -symbols case/"+dir+"gf_opt_bit2.aig");

	//Yosys::run_pass("read_aiger case/"+dir+"rf_opt_bit2.aag");
	//Yosys::run_pass("hierarchy -top \\case/"+dir+"rf_opt_bit2.aag");
	//Yosys::run_pass("opt_clean");
	//Yosys::run_pass("opt_merge");
	//Yosys::run_pass("write_aiger -symbols case/"+dir+"rf_opt_bit2.aig");
	
	Yosys::yosys_shutdown();
	return 0;
}

