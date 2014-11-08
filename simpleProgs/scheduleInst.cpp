#include <stdio.h>
#include <string>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"
#include "BPatch_function.h"
using namespace Dyninst;
using namespace std;
using namespace SymtabAPI;
BPatch bpatch;
BPatch_image *appImage = NULL;
BPatch_process *appProc = NULL;
vector<string> options;


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
    <<"\n\nSOURCES\n"
	     <<"\tRAND    - random weights\n"
             <<"\tEQUAL   - equal weights\n"
             <<"\tREVERSE - Earlier threads have higher priority\n"
    <<"\n\nINSTS example: create,dMem,sync\n"
             <<"\tcreate  - instrument pthread_create\n"
             <<"\tsync    - instrument pthread syncronization\n"
             <<"\tdMem    - instrument dynamic memory allocations and accesses\n"
              << endl;
}


int schedule(string sched, int numThreads){
    FILE *file = fopen("createPrios.txt","w");
    int ret = 0;
    if(sched == "RAND"){
	fprintf(file,"%d\n",numThreads);
        int ndx;
        for(ndx = 0;ndx < numThreads;ndx++){
	    fprintf(file,"%d\n",rand()%(sched_get_priority_max(SCHED_RR)-
				sched_get_priority_min(SCHED_RR)));
        }
    }
    else if(sched == "EQUAL"){
        fprintf(file,"%d\n",numThreads);
        int ndx;
        for(ndx = 0;ndx < numThreads;ndx++){
            fprintf(file,"%d\n",sched_get_priority_min(SCHED_RR));
        }
    }
    else if(sched == "REVERSE"){
        fprintf(file,"%d\n",numThreads);
        int ndx;
        for(ndx = 0;ndx < numThreads;ndx++){
	    int val = (sched_get_priority_min(SCHED_RR)+numThreads-(ndx+1));
            val = val > sched_get_priority_min(SCHED_RR)?val:sched_get_priority_min(SCHED_RR);
            fprintf(file,"%d\n",val);
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
    string createFile = "libcreateFunc.so";
    Symtab *obj = NULL;
    vector<Symbol *> syms;

    bool rtn = Symtab::openFile(obj, createFile);
    if(!rtn) printf("Problem opening file");

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
}

void memInst(){

}

void instrument(){
     for (vector<string>::iterator it = options.begin();it != options.end(); it++){
         cout << *it;
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

int main(int argc, char *argv[]){
    // process control

    handleArgs(argc,argv);

    appImage = appProc->getImage();

    // Load the tool library
    appProc->loadLibrary("./libcreateFunc.so");
    
    //for running as root
    //appProc->loadLibrary("/home/robert/cs736proj/simpleProgs/libcreateFunc.so"); 
    instrument();

    // continue execution of the mutatee
    printf("\nCalling process continue\n");
    appProc->continueExecution();

    // wait for mutatee to terminate 
    while (!appProc->isTerminated()) {
        bpatch.waitForStatusChange();
    }
    return 0;
}

