
#include <iostream>
#include <fstream>
#include "pin.H"

std::ofstream TraceFile;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
	"o", "trace.out", "specify trace file name");

INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

VOID Syscall_entry(THREADID thread_id, CONTEXT *ctx, SYSCALL_STANDARD std, void *v)
{
	ADDRINT sysNum = PIN_GetSyscallNumber(ctx, std);
	if (sysNum == 3)
	{
		printf("NtReadFile\n");
		string msg = "NtReadFile found!\n";
		TraceFile.write(msg.c_str(), msg.size());
	}
}

VOID Fini(INT32 code, VOID *v)
{
	TraceFile << "# eof" << endl;

	TraceFile.close();

}
int main(int argc, char * argv[])
{
	/* Init Pin arguments */
	if (PIN_Init(argc, argv)){
		return Usage();
	}

	TraceFile.open(KnobOutputFile.Value().c_str());

	/* Add the syscall handler */
	PIN_AddSyscallEntryFunction(Syscall_entry, 0);
	PIN_AddFiniFunction(Fini, 0);

	/* Start the program */
	PIN_StartProgram();
    
    return 0;
}
