#include <stdio.h>
#include <string>
#include <stdbool.h>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"
#include "BPatch_function.h"
using namespace Dyninst;
using namespace std;
using namespace SymtabAPI;
#include "BPatch_process.h"
/*
enum BPatch_exitType { NoExit, ExitedNormally,
	ExitedViaSignal };
*/
BPatch bpatch;
BPatch_image *appImage = NULL;
BPatch_process *appProc = NULL;
BPatch_function *traceEntryFunc;
BPatch_function *traceExitFunc;
BPatch_type *intType;

vector<string> options;
vector<string> incs;
bool tracing = true;
bool detach = false; 
bool inclusive = false;
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
              << "\t-i,--instrument INSTS\tComma separated list of operations to instrument\n"
              << "\t-r --rNum INT\t random number seed\n"
	      << "\t--noTrace\t disable function tracing\n"
	      << "\t--detach\t detach after process creation\n"
              << "\t-i,--inclusive INSTS\tComma separated list of modules to instrument\n"
    <<"\n\nSOURCES\n"
	     <<"\tRAND    - random weights\n"
             <<"\tEQUAL   - equal weights\n"
             <<"\tREVERSE - Earlier threads have higher priority\n"
             <<"\tNORMAL  - Uses linux default scheduler\n"
    <<"\n\nINSTS example: create,dMem,sync\n"
             <<"\tcreate  - instrument pthread_create\n"
             <<"\tsync    - instrument pthread syncronization\n"
             <<"\tdMem    - instrument dynamic memory allocations and accesses\n"
             <<"\tfork    - instrument dynamic memory allocations and accesses\n"
 << endl;
}


int schedule(string sched, int numThreads){
    FILE *file = fopen("/tmp/createPrios.txt","w");
    int ret = 0;
    if(sched == "RAND"){
	fprintf(file,"%d,%d\n",SCHED_RR, numThreads);
        int ndx;
        for(ndx = 0;ndx < numThreads;ndx++){
	    fprintf(file,"%d\n",rand()%(sched_get_priority_max(SCHED_RR)-
				sched_get_priority_min(SCHED_RR)));
        }
    }
    else if(sched == "EQUAL"){
        fprintf(file,"%d,%d\n", SCHED_RR, numThreads);
        int ndx;
        for(ndx = 0;ndx < numThreads;ndx++){
            fprintf(file,"%d\n",sched_get_priority_min(SCHED_RR));
        }
    }
    else if(sched == "REVERSE"){
        fprintf(file,"%d,%d\n", SCHED_RR, numThreads);
        int ndx;
        for(ndx = 0;ndx < numThreads;ndx++){
	    int val = (sched_get_priority_min(SCHED_RR)+numThreads-(ndx+1));
            val = val > sched_get_priority_min(SCHED_RR)?val:sched_get_priority_min(SCHED_RR);
            fprintf(file,"%d\n",val);
        }
    }
    else if(sched == "NORMAL"){
	fprintf(file, "%d,%d\n", SCHED_OTHER, numThreads);
	//not needed, but let's keep the code/input files consistent
        int ndx;
        for(ndx = 0;ndx < numThreads;ndx++){
            fprintf(file,"%d\n", 0);
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
	}else if ((arg == "-a") || (arg == "--attach")) {
		//don't need path as linux does this I guess..
		//char *path = argv[++i]; 
		int pid = atoi(argv[++i]);
		appProc = bpatch.processAttach(NULL, pid);
		return 0;
	} else if((arg == "-i")||(arg == "--instrument")){
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                string hold = argv[++i];
                replace( hold.begin(), hold.end(), ',', ' ');
    		string buf = ""; // Have a buffer string
    		stringstream ss(hold); // Insert the string into a stream

    		while (ss >> buf){
		    cout <<"Found option "<<buf<<endl;
        	    options.push_back(buf);
                }
           }
        }else if((arg == "-r")||(arg == "--rNum")){
           srand(atoi(argv[++i]));
        }else if(arg == "--noTrace"){
           tracing = false;
           cerr << "Tracing disabled" << endl; 
        }else if(arg == "--detach"){
           detach = true;
           cerr << "Detaching after process creation" << endl; 
       }else if(arg == "--inclusive"){
           inclusive = true;

            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                string hold = argv[++i];
                replace( hold.begin(), hold.end(), ',', ' ');
                string buf = ""; // Have a buffer string
                stringstream ss(hold); // Insert the string into a stream

                while (ss >> buf){
                    cout <<"Found option "<<buf<<endl;
                    incs.push_back(buf);
                }
           }
 


        }else {
            sources.push_back(argv[i]);
        
      }
}
return 0;
}



void createInst(){
    BPatch_function *origCreate = getFunction("pthread_create");
    if(!origCreate)
        printf("ACK\n");

    BPatch_function *newCreate = getFunction("my_pthread_create");
    if(!newCreate)
        printf("ACK\n");

    //had to load the file again, no idea why
    char* createFile; 
    createFile = getenv("SCHEDULEINST_LIB"); 

    Symtab *obj = NULL;
    vector<Symbol *> syms;

    bool rtn = Symtab::openFile(obj, createFile);
    if(!rtn || obj == NULL){
	fprintf(stderr, "Problem opening tool library\n");
 	exit(1); 
    }

    rtn = obj->findSymbol(syms, "orig_pthread_create", Symbol::ST_UNKNOWN,mangledName, false, false, true);
    if(!rtn) cout << SymtabAPI::Symtab::printError(SymtabAPI::Symtab::getLastSymtabError()) << endl;

    // instrument all function entries with count snippets
    rtn = appProc->wrapFunction(origCreate,newCreate,syms[0]);
    if(!rtn) cout << SymtabAPI::Symtab::printError(SymtabAPI::Symtab::getLastSymtabError()) << endl;

}

void forkInst(){
    BPatch_function *origCreate = getFunction("fork");
    if(!origCreate)
        printf("ACK\n");

    BPatch_function *newCreate = getFunction("my_fork");
    if(!newCreate)
        printf("ACK\n");

    //had to load the file again, no idea why
    string createFile = "libcreateFunc.so";
    Symtab *obj = NULL;
    vector<Symbol *> syms;

    bool rtn = Symtab::openFile(obj, createFile);
    if(!rtn) printf("Problem opening file");

    rtn = obj->findSymbol(syms, "orig_fork", Symbol::ST_UNKNOWN,mangledName, false, false, true);
    if(!rtn) cout << SymtabAPI::Symtab::printError(SymtabAPI::Symtab::getLastSymtabError()) << endl;

    // instrument all function entries with count snippets
    rtn = appProc->wrapFunction(origCreate,newCreate,syms[0]);
    if(!rtn) cout << SymtabAPI::Symtab::printError(SymtabAPI::Symtab::getLastSymtabError()) << endl;

}

void syncInst(){
    vector<string> syncFuncs;
    syncFuncs.push_back("sem_wait");
    syncFuncs.push_back("sem_post");
    syncFuncs.push_back("pthread_mutex_unlock");
    syncFuncs.push_back("pthread_mutex_lock");
    BPatch_function *inst = getFunction("randPrio");

       // 1. Find BPatch_point at entry of main for counter variable instrumentation initialization
    for (auto iter = syncFuncs.begin(); iter != syncFuncs.end(); ++iter) {
       // 4. insert increment snippet at function entry
       BPatch_function *syncFunc = getFunction((*iter).c_str());
       vector<BPatch_snippet *> args;
       BPatch_funcCallExpr syncF(*inst, args);
       BPatch_Vector<BPatch_point*> * entryPoints = syncFunc->findPoint(BPatch_exit);
       if(entryPoints != NULL) appProc->insertSnippet(syncF, *entryPoints);
    }

}


void memInst(){

}

void instrument(){
     for (vector<string>::iterator it = options.begin();it != options.end(); it++){
         cout << *it<<endl;
         if(*it == "create"){
             createInst();
	 }
         else if(*it == "sync"){
             syncInst();
         }
         else if(*it == "dMem"){
	     memInst();
         }
	 else if(*it == "fork"){
	     forkInst(); 
	 }
         else{
             cerr <<"Invalid instrumentation type "<<*it<<endl;
             exit(1);
         }
    }
}



//ALTERED FROM http://www.paradyn.org/tracetool.html#Download
enum arg_type { tr_unknown = 0, tr_int = 1 };

// End the sequence with a NULL
// should_instrument_module is expecting these to all be in lowercase
char *excluded_modules[] = {
   "default_module", "libstdc++", "libm", "libc", "ld-linux",
   "libdyninstapi_rt", "libdl", "tracelib", "kernel", ".so.",
   "global_linkage", // AIX modules
   NULL
};

void mystrlwr(char *str) {
   for(char *c=str; (*c)!=0; c++) {
      *c = tolower(*c);
   }
}

bool should_instrument_module(char *mod_input) {

   int i=0;
   char modname[1000];
   strcpy(modname, mod_input);
   mystrlwr(modname);
   while(i<1000) {
      char *cur_mod = excluded_modules[i];
      
      if(cur_mod == NULL) break;
      else if(strstr(modname, cur_mod)){
          printf("NOT INSTRUMENTING %s\n",cur_mod);
          return false;
      }
      i++;
   }
   if(inclusive){

      for(int n = 0;n<incs.size();n++){

	if(strstr(mod_input,incs[n].c_str()))
		return true;
      }
      return false;

   }



   return true;
}

void instrument_entry(BPatch_function *func, char *funcname) {
   BPatch_Vector<BPatch_localVar *> *params = func->getParams();
   int num_args = (*params).size();
   enum arg_type argOneType = tr_unknown;
   
   if(num_args>0) {
      BPatch_localVar *firstParam = (*params)[0];
      BPatch_type *first_arg_type = firstParam->getType();
      if(first_arg_type && first_arg_type->getID() == intType->getID())
         argOneType = tr_int;
   }
   
   BPatch_Vector<BPatch_snippet *> traceFuncArgs;
   BPatch_constExpr funcName(funcname);
   BPatch_constExpr descArg("...desc...");
   BPatch_constExpr numFuncArgs(num_args);
   BPatch_paramExpr argOne(0);
   BPatch_constExpr nullArgOne((const void *)NULL);
   BPatch_constExpr argType(argOneType);

   traceFuncArgs.push_back(&funcName);
   traceFuncArgs.push_back(&descArg);
   traceFuncArgs.push_back(&numFuncArgs);
   if(num_args > 0)
      traceFuncArgs.push_back(&argOne);
   else
      traceFuncArgs.push_back(&nullArgOne);
   traceFuncArgs.push_back(&argType);

   BPatch_Vector<BPatch_point *> *entryPointBuf = 
      func->findPoint(BPatch_entry);
   if(entryPointBuf == NULL || (*entryPointBuf).size() != 1) {
      cerr << "couldn't find entry point for func " << funcname << endl;
      return;
   }
   BPatch_point *entryPoint = (*entryPointBuf)[0];

   BPatch_funcCallExpr traceEntryCall(*traceEntryFunc, traceFuncArgs);
   appProc->insertSnippet(traceEntryCall, *entryPoint, BPatch_callBefore,
                            BPatch_firstSnippet);
}

void instrument_exit(BPatch_function *func, char *funcname) {
   cerr << "calling instrument_exit\n";
   BPatch_type *retType = func->getReturnType();

   enum arg_type argOneType = tr_unknown;
   if(retType && retType->getID() == intType->getID())
      argOneType = tr_int;
   
   BPatch_Vector<BPatch_snippet *> traceFuncArgs;
   BPatch_constExpr funcName(funcname);
   BPatch_constExpr descArg("...desc...");
   BPatch_retExpr   retVal;
   BPatch_constExpr retValType(argOneType);

   traceFuncArgs.push_back(&funcName);
   traceFuncArgs.push_back(&descArg);
   traceFuncArgs.push_back(&retVal);
   traceFuncArgs.push_back(&retValType);

   BPatch_Vector<BPatch_point *> *exitPointBuf = func->findPoint(BPatch_exit);

   // dyninst might not be able to find exit point
   if(exitPointBuf == NULL || (*exitPointBuf).size() == 0) {
      cerr << "   couldn't find exit point, so returning\n";
      return;
   }

   for(int i=0; i<(*exitPointBuf).size(); i++) {
      cerr <<"   inserting an exit pt instrumentation\n";
      BPatch_point *curExitPt = (*exitPointBuf)[i];

      BPatch_funcCallExpr traceExitCall(*traceExitFunc, traceFuncArgs);
      appProc->insertSnippet(traceExitCall, *curExitPt, BPatch_callAfter,
                               BPatch_firstSnippet);
   }
}


void instrument_funcs_in_module(BPatch_module *mod) {
   BPatch_Vector<BPatch_function *> *allprocs = mod->getProcedures();

   char name[1000];
   unsigned int i=0;
   for( i=0; i<(*allprocs).size(); i++) {
      BPatch_function *func = (*allprocs)[i];
      func->getName(name, 999);

      if(strstr(name,"Handle"))continue;
      //if(strstr(name,"<"))continue;
     if(strstr(name,"__"))continue;
     // if(strstr(name,"::"))continue;

     //ffmpeg hack
     if(strstr(name,"ff_thread_await_progress")) continue;
     if(strstr(name,"ff_h263_encode_motion")) continue;  
      cout << "  instrumenting function #" << i+1 << ":  " << name << endl;
      instrument_entry(func, name);
      instrument_exit(func, name);
   }
}

void initTracing(){
   traceEntryFunc = getFunction("trace_entry_func");
   traceExitFunc = getFunction("trace_exit_func");
    intType = appImage->findType("int");

//BROKEN


   const BPatch_Vector<BPatch_module *> *mbuf = appImage->getModules();

   for(unsigned n=0;n<(*mbuf).size(); n++) {
      BPatch_module *mod = (*mbuf)[n];
      char modname[1000];
      mod->getName(modname, 999);
      cout << "Program Module " << modname << " ------------------" << endl;
      if(should_instrument_module(modname)) {
         instrument_funcs_in_module(mod);
      }
   }

//NOT BROKEN





   char name[1000];
   unsigned int i=0;

    vector<string> syncFuncs;
    syncFuncs.push_back("sem_wait");
    syncFuncs.push_back("sem_post");
    syncFuncs.push_back("pthread_mutex_unlock");
    syncFuncs.push_back("pthread_mutex_lock");
    syncFuncs.push_back("my_pthread_create");
    syncFuncs.push_back("pthread_spin_lock");
    syncFuncs.push_back("pthread_spin_unlock");

       // 1. Find BPatch_point at entry of main for counter variable instrumentation initialization
    for (auto iter = syncFuncs.begin(); iter != syncFuncs.end(); ++iter) {
       // 4. insert increment snippet at function entry
       BPatch_function *syncFunc = getFunction((*iter).c_str());
      syncFunc->getName(name, 999);
      if(strstr(name,"__"))continue;

      cout << "  instrumenting function #" << i+1 << ":  " << name << endl;
      i++;
      instrument_entry(syncFunc, name);
      instrument_exit(syncFunc, name);
   }


}





int main(int argc, char *argv[]){

    /*int major, minor, subminor; 
    bpatch.getBPatchVersion(major, minor, subminor); 
    printf("Version: Dyninst %d.%d.%d\n", major, minor, subminor); 
    */

    // process control
    char* lib_path = NULL;
    lib_path = getenv("SCHEDULEINST_LIB"); 
    if(lib_path == NULL){
	fprintf(stderr, "SCHEDULEINST_LIB not defined\n"); 
	exit(1); 
    }
    handleArgs(argc,argv);
    appImage = appProc->getImage();

    // Load the tool library
    if(!appProc->loadLibrary(lib_path)){
	fprintf(stderr, "Couldn't load tool library.\n"); 
    }
    if(tracing) initTracing();
    instrument();

    // continue execution of the mutatee
    printf("\nCalling process continue\n");
    bool rtn = appProc->continueExecution();
    if(!rtn){
	printf("error starting proc\n"); 
	exit(1); 
    }
    if(detach) {
	appProc->detach(true); 
	exit(0); 
    }
    // wait for mutatee to terminate 
    while (!appProc->isTerminated()) {
        bpatch.waitForStatusChange();
    }

    int exit_val; 
    if((exit_val = appProc->getExitSignal()) != -1){
    	printf("Calling process done: caught signal %d\n", exit_val); 
    }
    else if((exit_val = appProc->getExitCode()) != 0){
	printf("Calling process done: exit code %d\n", exit_val); 
    }
    else{
	printf("Calling process done, exited normally\n"); 
    }
    return exit_val;
}

