/**CFile****************************************************************

  FileName    [trydcec.c]
  SystemName  [ABC: Logic synthesis and verification system.]
  PackageName [ABC as a static library.]
  Synopsis    [A demo program illustrating the use of ABC as a static library and the use of dcec command.]
  Author      [WanHsuan Lin]
  Date        [May 26, 2020.]
***********************************************************************/

#include <stdio.h>
#include <time.h>
#include "../../kernel/yosys.h"
#include <string>
#include "cirMgr.h"
#include <fstream>

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

#if defined(ABC_NAMESPACE)
namespace ABC_NAMESPACE
{
#elif defined(__cplusplus)
extern "C"
{
#endif

// procedures to start and stop the ABC framework
// (should be called before and after the ABC procedures are called)
void   Abc_Start();
void   Abc_Stop();

// procedures to get the ABC framework and execute commands in it
typedef struct Abc_Frame_t_ Abc_Frame_t;

Abc_Frame_t * Abc_FrameGetGlobalFrame();
int    Cmd_CommandExecute( Abc_Frame_t * pAbc, const char * sCommand );

#if defined(ABC_NAMESPACE)
}
using namespace ABC_NAMESPACE;
#elif defined(__cplusplus)
}
#endif

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [The main() procedure.]

***********************************************************************/
int main( int argc, char * argv[] )
{
	clock_t clkVer, clk, clkRead;
	clkRead = clock();
    Yosys::log_streams.push_back(&std::cout);
	Yosys::log_error_stderr = true;

	Yosys::yosys_setup();
	Yosys::yosys_banner();

	std::string gf(argv[1]);
	std::string rf(argv[2]);
	std::string outfile(argv[3]);
	
	Yosys::run_pass("read_verilog -overwrite "+gf);
	Yosys::run_pass("read_verilog bin/_DC.v");
	Yosys::run_pass("read_verilog bin/_HMUX.v");
	Yosys::run_pass("flatten");
	Yosys::run_pass("aigmap");
	Yosys::run_pass("setundef -params -anyconst");
	Yosys::run_pass("hierarchy -top \\top");
	Yosys::run_pass("opt_clean");
	Yosys::run_pass("opt_merge");
	Yosys::run_pass("write_aiger -ascii -symbols gf_opt.aag");

	Yosys::run_pass("read_verilog -overwrite "+rf);
	Yosys::run_pass("read_verilog bin/_DC.v");
	Yosys::run_pass("read_verilog bin/_HMUX.v");
	Yosys::run_pass("flatten");
	Yosys::run_pass("aigmap");
	Yosys::run_pass("setundef -params -anyconst");
	Yosys::run_pass("hierarchy -top \\top");
	Yosys::run_pass("opt_clean");
	Yosys::run_pass("opt_merge");
	Yosys::run_pass("write_aiger -ascii -symbols rf_opt.aag");

	CirMgr *cirMgr = new CirMgr;
	
	clock_t aag2aig = clock();

	cirMgr -> readCircuit("gf_opt.aag", GOLDEN);
	cirMgr -> readCircuit("rf_opt.aag", REVISED);

	cirMgr -> buildCircuitTwo(GOLDEN);	
	cirMgr -> buildCircuitTwo(REVISED);	

	if(!cirMgr -> randomSim(outfile,250)){
		clk = clock();
		printf( "Readfile = %6.2f sec\n", (float)(clk-clkRead)/(float)(CLOCKS_PER_SEC) );
    
		delete cirMgr; cirMgr =  NULL;
		return 0;
	}
	
	cirMgr -> writeBlif("gf_opt_bit2.blif", GOLDEN);
	cirMgr -> writeBlif("rf_opt_bit2.blif", REVISED);
	
	printf( "aag2aig = %6.2f sec\n", (float)(aag2aig-clock())/(float)(CLOCKS_PER_SEC) );


	delete cirMgr; cirMgr = NULL;
	
	Yosys::yosys_shutdown();
	
    /////start abc

    // variables
    Abc_Frame_t * pAbc;
    char * pFileName[2];
    char Command[1000];
   

    //////////////////////////////////////////////////////////////////////////
    // get the input file name
    pFileName[0] = "gf_opt_bit2.blif";
    pFileName[1] = "rf_opt_bit2.blif";

    //////////////////////////////////////////////////////////////////////////
    // start the ABC framework
    Abc_Start();
    pAbc = Abc_FrameGetGlobalFrame();

clk = clock();
    //////////////////////////////////////////////////////////////////////////
    // verification
    sprintf( Command, "dcec -W %s %s %s", argv[3], pFileName[0], pFileName[1] );
	// sprintf( Command, "cec %s %s", pFileName[0], pFileName[1] );
    if ( Cmd_CommandExecute( pAbc, Command ) )
    {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
        return 1;
    }
clkVer = clock() - clk;
	printf( "Readfile = %6.2f sec\n", (float)(clk-clkRead)/(float)(CLOCKS_PER_SEC) );
    printf( "Verification = %6.2f sec\n", (float)(clkVer)/(float)(CLOCKS_PER_SEC) );
	printf( "Total = %6.2f sec\n", (float)(clkVer+clk-clkRead)/(float)(CLOCKS_PER_SEC) );

    //////////////////////////////////////////////////////////////////////////
    // stop the ABC framework
    Abc_Stop();
    return 0;
}

