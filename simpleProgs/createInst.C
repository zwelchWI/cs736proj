#include <stdio.h>
#include <string>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"
#include "BPatch_function.h"
#include "defs.h"
using namespace Dyninst;
using namespace std;
using namespace SymtabAPI;
const bool printzero = false;
extern int* prios;
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

static void show_usage(string name)
{
    cerr << "Usage: " << name << " <option(s)> -d SOURCES\n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-t,--type SCHEDULE\tSpecify the schedule policy\n"
              << "\t-s,--size INtEGER\tNumber of threads\n"
              << "\t-d,--dyninst SOURCES\tSpecify what to give dyninst\n"
              << endl;
}


int schedule(string sched, int numThreads){
    FILE *file = fopen("createPrios.txt","w");
    int ret = 0;
    if(sched == "RAND"){
	fprintf(file,"%d\n",numThreads);
        int ndx;
        for(ndx = 0;ndx < numThreads;ndx++){
	    fprintf(file,"%d\n",rand()%(sched_get_priority_max(SCHED_FIFO)-
				sched_get_priority_min(SCHED_FIFO)));
        }
    }
    else{
        ret = -1;
    }
    
    

    fclose(file);
    return ret;

}


int handleArgs(int argc,char **argv){
    vector <string> sources;
    string sched;
    int size = -1;
    bool both = false;
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) {
            show_usage(argv[0]);
            exit( 1);
        } else if ((arg == "-t") || (arg == "--type")) {
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                sched = argv[++i]; // Increment 'i' so we don't get the argument as the next argv[i].
		cout <<"SCHEDULE "<<sched<<endl;
                if(both){	
	            if(schedule(sched,size) < 0){
		        cerr << "ERROR: INVALID SCHEDULE" << endl;
		        show_usage(argv[0]);
   		    } 

                }
		else{
                    both=true;
                }  
	    }else { // Uh-oh, there was no argument to the destination option.
                cerr << "--type option requires one argument." << endl;
                 exit(1);
            }
        }else if ((arg == "-s") || (arg == "--size")) {
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                size = atoi(argv[++i]); // Increment 'i' so we don't get the argument as the next argv[i].
		cout <<size<< " THREADS"<<endl;
                if(both){
                    
                    if(schedule(sched,size) < 0){
                        cerr << "ERROR: INVALID SCHEDULE" << endl;
                        show_usage(argv[0]);
                    }
                } 
                else{
                    both=true;
                }
            }else { // Uh-oh, there was no argument to the destination option.
                cerr << "--size option requires one argument." << endl;
                exit( 1);
            }
	}else if ((arg == "-d") || (arg == "--dyninst")) {
            appProc = startMutateeProcess(argc-i,argv+i);
            return 0;
	}else {
            sources.push_back(argv[i]);
        }
    }
}


int main(int argc, char *argv[])
{
    // process control


    handleArgs(argc,argv);

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

