#include <stdio.h>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"
#include "BPatch_function.h"

using namespace Dyninst;
using namespace std;

const bool printzero = false;

BPatch bpatch;
BPatch_image *appImage = NULL;
BPatch_process *appProc = NULL;
BPatch_function *incrementFunc = NULL;
struct funcStruct {
   BPatch_function *func;
   BPatch_variableExpr *counter;
   funcStruct(BPatch_function *f) : func(f), counter(NULL) {};
};
vector<funcStruct> allFuncs;


/* returns the first function to match a particular name */
BPatch_function* getFunction(const char* name)
{
   BPatch_Vector<BPatch_function*> funcs;
   appImage->findFunction(name, funcs,true,true,true);
   if (funcs.size() == 0) {
       return NULL;
   } else {
       return funcs[0];
   }
}

// gather all function usage data at the end of the run
static void printFuncCounts(BPatch_thread*, BPatch_exitType)
{
   for (auto iter = allFuncs.begin(); iter != allFuncs.end(); ++iter) {
      // Read the counter associated with this function
      	long val;
		iter->counter->readValue(&val);
      	printf("Function %20s was invoked %d times\n", iter->func->getName().c_str(), val);
		// and print its value
   }
}

// For each function:
//   1) Allocate a variable to hold the number of times invoked
//   2) Initialize the variable to 0
//   3) Insert a call to increment the variable
//   3a) Create arguments for the increment call
//   3b) Create a function call snippet
//   3c) Look up the entry point of the function
//   3d) Insert the call snippet at the entry point
void instrumentFuncs() {
    BPatch_constExpr zero(0);
    BPatch_constExpr one(1);
    for (auto iter = allFuncs.begin(); iter != allFuncs.end(); ++iter) {
//   1) Allocate a variable to hold the number of times invoked
		iter->counter = appProc->malloc(*appImage->findType("long") );

//   2) Initialize the variable to 0
//   3) Insert a call to increment the variable
		long init = 0;
		iter->counter->writeValue(&init);

//   3a) Create arguments for the increment call
		vector<BPatch_snippet *> increment_args;
		BPatch_constExpr var_addr(iter->counter->getBaseAddr());
		increment_args.push_back(&var_addr);
       	increment_args.push_back(&one);
//   3b) Create a function call snippet
		BPatch_funcCallExpr increment(*incrementFunc, increment_args);
//   3c) Look up the entry point of the function
		BPatch_Vector<BPatch_point*> * entryPoints = iter->func->findPoint(BPatch_entry);
       appProc->insertSnippet(increment, *entryPoints);
//   3d) Insert the call snippet at the entry point
    }

}

// Iterate over all modules in the executable and get
// all functions in each such module
void getExecutableFuncs ()
{
   vector<BPatch_module *> *mods = appImage->getModules();
   for (auto iter = mods->begin(); iter != mods->end(); ++iter) {
      // Skip module if it represents a shared library
      // Get all functions and add to allFuncs
      	vector<BPatch_function *> * hold = (*iter)->getProcedures();
		for(auto iter2 = hold->begin();iter2 != hold->end();iter2++){
			allFuncs.push_back(funcStruct(*iter2));
		}
      }
   }


BPatch_process *startMutateeProcess(int argc, char *argv[])
{
    // command line argument error message
    if (argc < 2) {
        fprintf(stderr, "Usage: %s prog_filename [args]\n",argv[0]);
        exit(1);
    } 
    // create the mutatee process 
    return bpatch.processCreate( argv[1] , (const char**)(argv + 1) );
}

int main(int argc, char *argv[])
{
    // process control
    appProc = startMutateeProcess(argc,argv);
    appImage = appProc->getImage();

    // Load the tool library
    appProc->loadLibrary("libincrement.so");

    // gather all functions in the executable, and their names
    getExecutableFuncs();

    // Identify the increment function
    incrementFunc = getFunction("increment");

    // instrument all function entries with count snippets
    instrumentFuncs();
    printf("Instrumented %d functions\n", allFuncs.size());

    // register printFuncCounts as exit callback function
    bpatch.registerExitCallback(printFuncCounts);
   
    // continue execution of the mutatee
    printf("\nCalling process continue\n");
    appProc->continueExecution();

    // wait for mutatee to terminate 
    while (!appProc->isTerminated()) {
        bpatch.waitForStatusChange();
    }
    return 0;
}

