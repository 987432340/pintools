#include <iostream>
#include <fstream>
#include "pin.H"
#include "TraceLog.h"
#include "ProcessInfo.h"
#include "ModuleInfo.h"

ofstream OutFile;
TraceLog traceLog;
ProcessInfo processInfo;

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static UINT64 icount = 0;

// This function is called before every instruction is executed
VOID docount() { icount++; }


VOID ImageLoad(IMG Image, VOID *v)
{
	processInfo.addModule(Image);
}

// 每有一条函数执行时，pin会回调到此函数
VOID Instruction(INS ins, VOID *v)
{
    // Insert a call to docount before every instruction, no arguments are passed
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
	if ( icount < 100)
	{
		traceLog.logIns(INS_Address(ins), INS_Disassemble(ins));
		OutFile << std::hex << std::showbase << INS_Address(ins) << " " << INS_Disassemble(ins) << endl;
		cout << "0x" << std::hex << INS_Address(ins) << "\t" << INS_Disassemble(ins) << endl;
	}
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "inscount.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Write to a file since cout and cerr maybe closed by the application
    //OutFile.setf(ios::showbase);
	OutFile << "Count " << std::dec << icount << endl;
    OutFile.close();
	cout << "Count " << std::dec << icount << endl;
}

INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char * argv[])
{
	// doc: Initialize symbol table code with an explicitly specified mode of symbol support. 
	PIN_InitSymbols();

    // doc: Initialize Pin system. Must be called before PIN_StartProgram
    if (PIN_Init(argc, argv)) 
		return Usage();
	
	traceLog.init("", false);
	//traceLog.init(KnobOutputFile.Value(), false);

	OutFile.open(KnobOutputFile.Value().c_str());

	// Register function to be called for every loaded module
	IMG_AddInstrumentFunction(ImageLoad, 0);
    
	// Register Instruction to be called to instrument instructions
	INS_AddInstrumentFunction(Instruction, 0);
	//INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
