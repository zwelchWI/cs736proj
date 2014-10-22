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
      long curCount;
      iter->counter->readValue(&curCount);
      printf("Function %20s was invoked %d times\n", iter->func->getName().c_str(), curCount);
   }
}

// 1. Find BPatch_point at entry of main for counter variable instrumentation initialization
/* FOREACH function: */
// 3. allocate counter variable
// 4. insert counter variable initialization at main entry point
// 5. insert increment snippet at function entry
void instrumentFuncs() {
    // 1. Find BPatch_point at entry of main for counter variable instrumentation initialization
    BPatch_function *main = getFunction("main");
    std::vector< BPatch_point * > *mainEntries = main->findPoint(BPatch_entry);
    BPatch_constExpr zero(0);
    BPatch_constExpr one(1);
    for (auto iter = allFuncs.begin(); iter != allFuncs.end(); ++iter) {
       // 2. allocate func counter variable
       iter->counter = appProc->malloc( *(appImage->findType("long")) );
       assert(iter->counter);

       // 3. initialize counter variable
       long initial = 0;
       iter->counter->writeValue(&initial);

       // 4. insert increment snippet at function entry
       vector<BPatch_snippet *> increment_args;
       BPatch_constExpr var_addr(iter->counter->getBaseAddr());
       increment_args.push_back(&var_addr);
       increment_args.push_back(&one);
       BPatch_funcCallExpr increment(*incrementFunc, increment_args);
       BPatch_Vector<BPatch_point*> * entryPoints = iter->func->findPoint(BPatch_entry);
       appProc->insertSnippet(increment, *entryPoints);
    }

}

void getExecutableFuncs ()
{
 	BPatch_module * b= appImage->findModule("libpthread",true);
   vector<BPatch_module *> *mods = appImage->getModules();
  /* for (auto iter = mods->begin(); iter != mods->end(); ++iter) {

      if ((*iter)->isSharedLib())  continue;
      vector<BPatch_function *> *funcs = (*iter)->getProcedures();
      for (auto iter = funcs->begin(); iter != funcs->end(); ++iter) {
         allFuncs.push_back(funcStruct(*iter));
         cerr << "Instrumenting function " << (*iter)->getName() << endl;
      }
   }
	if(b == NULL){
		printf("ACK\n");
	}*/
	char hold[15];
	vector<BPatch_function *> *funcs = b->getProcedures();
       for (auto iter = funcs->begin(); iter != funcs->end(); ++iter) {
			string h((*iter)->getName(hold,9),9);
			if(h == string("pthread_c")){
           allFuncs.push_back(funcStruct(*iter));
           cerr << "Instrumenting function " << (*iter)->getName() << endl;
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
    appProc->loadLibrary("./libincrement.so");
	void * b= appImage->findModule("libpthread",true);
    assert(b!=NULL);
// gather all functions in the executable, and their names
    getExecutableFuncs();

    // Identify the increment function
    incrementFunc = getFunction("increment");
	if(!incrementFunc)
		printf("ACK\n");
    // instrument all function entries with count snippets
    instrumentFuncs();
    printf("Instrumented %d functions\n", allFuncs.size());

    // register printFuncCounts as exit callback function
    bpatch.registerExitCallback(printFuncCounts);
   
    // continue execution of the mutatee
    printf("\nCalling process continue\n");
    appProc->continueExecution();
	printf("DID I MAKE IT HERE\n");
    // wait for mutatee to terminate 
    while (!appProc->isTerminated()) {
        bpatch.waitForStatusChange();
    }
    return 0;
}

