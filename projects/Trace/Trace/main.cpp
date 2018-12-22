#include <iostream>
#include <fstream>
#include "pin.H"
#include "TraceLog.h"
#include "ProcessInfo.h"
#include "ModuleInfo.h"
namespace WINDOWS {
#include <windows.h>
#include <Shlwapi.h>
};
#pragma comment(lib, "Shlwapi.lib")

extern TraceLog traceLog;

ofstream OutFile;
ProcessInfo processInfo;

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static UINT64 icount = 0;
static ADDRINT start = 0;
static ADDRINT end = 0;


// This function is called before every instruction is executed
VOID docount() { icount++; }


VOID ImageLoad(IMG Image, VOID *v)
{
	processInfo.addModule(Image);
}

VOID SaveTransitions(ADDRINT Address, UINT32 numInstInBbl)
{
	//PIN_LockClient();

	//static bool is_prevMy = false;
	//static ADDRINT prevAddr = UNKNOWN_ADDR;

	//const s_module *mod_ptr = pInfo.getModByAddr(Address);
	//bool is_currMy = pInfo.isMyAddress(Address);

	////is it a transition from the traced module to a foreign module?
	//if (!is_currMy && is_prevMy && prevAddr != UNKNOWN_ADDR) {
	//	if (!mod_ptr) {
	//		//not in any of the mapped modules:
	//		traceLog.logCall(prevAddr, Address);
	//	}
	//	else {
	//		const string func = get_func_at(Address);
	//		std::string dll_name = mod_ptr->name;
	//		traceLog.logCall(prevAddr, dll_name, func);
	//	}

	//}
	////is the address within the traced module?
	//if (is_currMy) {
	//	ADDRINT addr = Address - mod_ptr->start; // substract module's ImageBase
	//	const s_module* sec = pInfo.getSecByAddr(addr);
	//	// is it a transition from one section to another?
	//	if (pInfo.isSectionChanged(addr)) {
	//		std::string name = (sec) ? sec->name : "?";
	//		if (prevAddr != UNKNOWN_ADDR && is_prevMy) {
	//			const s_module* prev_sec = pInfo.getSecByAddr(prevAddr);
	//			traceLog.logNewSectionCalled(prevAddr, prev_sec->name, sec->name);
	//		}
	//		traceLog.logSectionChange(addr, name);
	//	}
	//	prevAddr = addr; /* update saved */
	//}

	///* update saved */
	//is_prevMy = is_currMy;

	//PIN_UnlockClient();
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

VOID Trace(TRACE trace, VOID *v)
{
	int i = 0, j = 0;
	// Visit every basic block in the trace
	for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl), i++)
	{
		cout << "BBL" << i << endl;

		// Insert a call to SaveTranitions() before every basic block
		for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {
			cout << "0x" << std::hex << INS_Address(ins) << "\t" << INS_Disassemble(ins) << endl;
			//INS_InsertCall(ins, IPOINT_BEFORE,
			//	(AFUNPTR)SaveTransitions,
			//	IARG_INST_PTR,
			//	IARG_UINT32, BBL_NumIns(bbl), IARG_END);
		}
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
	
	std::string exe_path;
	for (int i = 1; i < (argc - 1); i++) {
		if (strcmp(argv[i], "--") == 0) {
			exe_path = argv[i + 1];
			break;
		}
	}
	WINDOWS::PathRemoveFileSpecA(&exe_path[0]);
	cout << "exe path" << exe_path << std::endl;
	processInfo.init("");
	traceLog.init("", false);
	//traceLog.init(KnobOutputFile.Value(), false);

	OutFile.open(KnobOutputFile.Value().c_str());

	// Register function to be called for every loaded module
	IMG_AddInstrumentFunction(ImageLoad, 0);
    
	// Register Instruction to be called to instrument instructions
	//INS_AddInstrumentFunction(Instruction, 0);
	
	
	TRACE_AddInstrumentFunction(Trace, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
