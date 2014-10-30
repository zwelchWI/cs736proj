#include <stdio.h>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"
#include "BPatch_function.h"
using namespace Dyninst;
using namespace std;
using namespace SymtabAPI;
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


// 1. Find BPatch_point at entry of main for counter variable instrumentation initialization
/* FOREACH function: */
// 3. allocate counter variable
// 4. insert counter variable initialization at main entry point
// 5. insert increment snippet at function entry
void instrumentFuncs() {
    // 1. Find BPatch_point at entry of main for counter variable instrumentation initialization
    //BPatch_function *main = getFunction("pthread_create");
    //std::vector< BPatch_point * > *mainEntries = main->findPoint(BPatch_entry);
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
	vector<BPatch_function *> *funcs = b->getProcedures();
       	char hold[15];
	for (auto iter = funcs->begin(); iter != funcs->end(); ++iter) {
			string h((*iter)->getName(hold,14),14);
			if(h == string("pthread_create")){
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
    appProc->loadLibrary("./libcreateFunc.so");
    void * b= appImage->findModule("libpthread",true);
    assert(b!=NULL);
    // gather all functions in the executable, and their names
    getExecutableFuncs();

    // Identify the increment function
    BPatch_function *main = getFunction("pthread_create");
      if(!main)
                printf("ACK\n");

    incrementFunc = getFunction("my_pthread_create");
	if(!incrementFunc)
		printf("ACK\n");

    //had to load the file again, no idea why
    std::string file = "libcreateFunc.so";  
    Symtab *obj = NULL;
    vector<Symbol *> syms;

    bool rtn = Symtab::openFile(obj, file); 
    if(!rtn) printf("Problem opening file"); 

    rtn = obj->findSymbol(syms, "orig_pthread_create", Symbol::ST_UNKNOWN,mangledName, false, false, true);
    if(!rtn) cout << SymtabAPI::Symtab::printError(SymtabAPI::Symtab::getLastSymtabError()) << endl; 
    
    // instrument all function entries with count snippets
    rtn = appProc->wrapFunction(main,incrementFunc,syms[0]);
    if(!rtn) cout << SymtabAPI::Symtab::printError(SymtabAPI::Symtab::getLastSymtabError()) << endl; 
    
    cout << std::string("Instrumented ") + std::to_string(allFuncs.size()) + std::string(" functions") <<endl; 

    // continue execution of the mutatee
    printf("\nCalling process continue\n");
    appProc->continueExecution();

    // wait for mutatee to terminate 
    while (!appProc->isTerminated()) {
        bpatch.waitForStatusChange();
    }
    return 0;
}

