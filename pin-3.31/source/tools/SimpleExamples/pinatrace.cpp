/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains an ISA-portable PIN tool for tracing memory accesses.
 */

 #include "pin.H"
 #include <iostream>
 #include <chrono>
 #include <fstream>
 #include <iomanip>
 #include <regex>
 #include <string>
 #include <vector>
 #include <unordered_map>
 #include <mutex>
 // #include "cache.hpp"
 
 using std::cerr;
 using std::dec;
 using std::endl;
 using std::hex;
 using std::ios;
 using std::setw;
 using std::string;
 
 using std::unordered_map;
 using std::vector;
 using std::ofstream;
 using std::mutex;
 
 
 
 /* ===================================================================== */
 /* Set Associative Cache Filter */
 /* ===================================================================== */
 //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 
 struct LRU_node{
     LRU_node* prev;
     LRU_node* next;
     int64_t cl_index;
 };
 
 class LRU_list{
     public:
         LRU_node* head;
         LRU_node* tail;
         LRU_list(){head = nullptr; tail = nullptr;};
 };
 
 
 struct cache_obj
 {
     bool isdirty;
     LRU_node* node;
 };
 
 
 struct eviction
 {
     // return 0 for no-eviction, return 1 for eviction-only, return 2 for flush
     int condition;
     int64_t index;
 };
 
 
 class fcache{
     public:
         std::unordered_map<int64_t, cache_obj> cachemem;
         LRU_list LRUlist;
         int64_t cache_size_CL;
         int64_t curr_size;
         fcache(int64_t size);
         void insert(int64_t index);
         void remove(int64_t index);
         bool is_hit(int64_t index);
         eviction miss_evict(int64_t index);
         void readhitCL(int64_t index);
         void writehitCL(int64_t index);
 };
 
 
 class sa_cache{
     public:
         int way;
         int64_t size_byte;
         int64_t num_sets;
         std::vector<fcache> sets;
         sa_cache(int64_t size_in_byte, int way);
         void insert(int64_t index);
         void remove(int64_t index);
         bool is_hit(int64_t index);
         eviction miss_evict(int64_t index);
         void readhitCL(int64_t index);
         void writehitCL(int64_t index);
 };
 
 
 
 fcache::fcache(int64_t size){
     cache_size_CL = size/64;
     curr_size = 0;
 }
 
 void fcache::insert(int64_t index){
     assert(curr_size < cache_size_CL);
     cache_obj new_obj;
     new_obj.isdirty = false;
     new_obj.node = new LRU_node;
     new_obj.node->prev = nullptr;
     new_obj.node->next = nullptr;
     new_obj.node->cl_index = index;
     if (LRUlist.tail==nullptr)
     {
         LRUlist.head = new_obj.node;
         LRUlist.tail = new_obj.node;
     }else
     {
         LRUlist.tail->next = new_obj.node;
         new_obj.node->prev = LRUlist.tail;
         LRUlist.tail = new_obj.node;
     }
     cachemem[index] = new_obj;
     curr_size++;
 }
 
 void fcache::remove(int64_t index){
     assert(curr_size>0);
     cachemem.erase(index);
     curr_size--;
 }
 
 bool fcache::is_hit(int64_t index){
     return (cachemem.find(index)!= cachemem.end());
 }
 
 eviction fcache::miss_evict(int64_t index){
     if (curr_size < cache_size_CL)
     {
         eviction ev;
         ev.condition = 0;
         ev.index = 0;
         return ev;
     }else
     {
         assert(LRUlist.head!=nullptr);
         LRU_node* evi = LRUlist.head;
         LRUlist.head->next->prev = nullptr;
         LRUlist.head = evi->next;
         bool is_dirty = cachemem[evi->cl_index].isdirty;
         remove(evi->cl_index);
         eviction eev;
         if (is_dirty)
         {
             eev.condition = 2;
         }
         else
         {
             eev.condition = 1;
         }
         eev.index = evi->cl_index;
         delete evi;
         return eev;
     }
 }
 
 
 void fcache::readhitCL(int64_t index){
     LRU_node* node = cachemem[index].node;
     if (node!=LRUlist.tail)
     {
         node->next->prev = node->prev;
         if (node->prev!=nullptr)
         {
             node->prev->next = node->next;
         } else
         {
             LRUlist.head = node->next;
         }
         
         LRUlist.tail->next = node;
         node->prev = LRUlist.tail;
         node->next = nullptr;
         LRUlist.tail = node;
     }
 }
 
 
 void fcache::writehitCL(int64_t index){
     readhitCL(index);
     cachemem[index].isdirty = true;
 }
 
 
 
 sa_cache::sa_cache(int64_t size_in_byte, int way){
     this->way = way;
     this->size_byte = size_in_byte;
     int64_t bank_size = size_byte / way;
     num_sets = bank_size / 64;
     for (int i = 0; i < num_sets; i++)
     {
         sets.emplace_back(way*64);
     }
 }
 
 void sa_cache::insert(int64_t index){
     int set_index = index % num_sets;
     sets[set_index].insert(index / num_sets);
 }
 
 void sa_cache::remove(int64_t index){
     int set_index = index % num_sets;
     sets[set_index].remove(index / num_sets);
 }
 
 bool sa_cache::is_hit(int64_t index){
     int set_index = index % num_sets;
     return sets[set_index].is_hit(index / num_sets);
 }
 
 eviction sa_cache::miss_evict(int64_t index){
     int set_index = index % num_sets;
     eviction evi = sets[set_index].miss_evict(index / num_sets);
     evi.index = evi.index * num_sets + set_index;
     return evi;
 }
 
 void sa_cache::readhitCL(int64_t index){
     int set_index = index % num_sets;
     sets[set_index].readhitCL(index / num_sets);
 }
 
 void sa_cache::writehitCL(int64_t index){
     int set_index = index % num_sets;
     sets[set_index].writehitCL(index / num_sets);
 }
 
 
 
 //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 
 
 /* ===================================================================== */
 /* Global Variables */
 /* ===================================================================== */
 static vector<ofstream> TraceFiles;
 static vector<sa_cache> Caches;
 
 static vector<VOID*> WriteAddr;
 static vector<INT32> WriteSize;
 // PIN_LOCK pinLock;
 UINT64 threadCount = 0; //total number of threads, including main thread
 
 double cpu_base_frequency() {
     std::regex re("model name\\s*:[^@]+@\\s*([0-9.]+)\\s*GHz");
     std::ifstream cpuinfo("/proc/cpuinfo");
     std::smatch m;
     for (std::string line; getline(cpuinfo, line);) {
         regex_match(line, m, re);
         if (m.size() == 2)
             return std::stod(m[1]);
     }
     return 1; // Couldn't determine the CPU base frequency. Just count TSC ticks.
 }
 double const CPU_GHZ_INV = 1 / cpu_base_frequency();
 
 struct TraceSample {
     uint64_t ns;
     uint64_t addr;
     uint8_t  r;
     uint8_t  size;
 } __attribute((__packed__));
 
 /* ===================================================================== */
 /* Commandline Switches */
 /* ===================================================================== */
 
 KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "pinatrace.out", "specify trace file name");
 KNOB< BOOL > KnobValues(KNOB_MODE_WRITEONCE, "pintool", "values", "1", "Output memory values reads and written");
 
 /* ===================================================================== */
 /* Print Help Message                                                    */
 /* ===================================================================== */
 
 VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v) {
     if (threadid == TraceFiles.size()) {
         string filename = KnobOutputFile.Value() + "." + decstr(threadid);
         TraceFiles.push_back(ofstream(filename));
         WriteAddr.push_back(0);
         WriteSize.push_back(0);
         Caches.emplace_back(2 * 1024 * 1024, 16);
     } else {
         printf("Thread %d duplicates\n", threadid);
         PIN_ExitProcess(1);
     }
 }
 
 VOID ThreadFini(THREADID threadid, const CONTEXT* ctxt, INT32 code, VOID* v) {
     for (unsigned int TraceFileID = 0; TraceFileID < TraceFiles.size(); TraceFileID++){
         TraceFiles[TraceFileID].close();
     }
 }
 
 static INT32 Usage()
 {
     cerr << "This tool produces a memory address trace.\n"
             "For each (dynamic) instruction reading or writing to memory the the ip and ea are recorded\n"
             "\n";
 
     cerr << KNOB_BASE::StringKnobSummary();
 
     cerr << endl;
 
     return -1;
 }
 
 static VOID RecordMem(VOID* ip, CHAR r, VOID* addr, INT32 size, THREADID threadid) {
     uint64_t ns;
     TraceSample sample[2];
     uint64_t memaddr = (uint64_t) addr;
     int64_t clindex = memaddr / 64;
 
     if (Caches[threadid].is_hit(clindex))
     {
         if (r == 'R')
         {
             Caches[threadid].readhitCL(clindex);
         }
         else if (r == 'W')
         {
             Caches[threadid].writehitCL(clindex);
         } 
         else
         {
             assert(0);
         }
     }
     else
     {
         eviction evi = Caches[threadid].miss_evict(clindex);
         if (evi.condition == 2)
         {
             ns = __builtin_ia32_rdtsc() * CPU_GHZ_INV;
             sample[0].ns = ns;
             sample[0].addr = evi.index * 64;
             sample[0].r = 'W';
             sample[0].size = 64; 
             TraceFiles[threadid].write((char *) &sample[0], sizeof(TraceSample));
         }
 
         Caches[threadid].insert(clindex);
         if (r == 'R')
         {
             // Caches[threadid].readhitCL(clindex);
         }
         else if (r == 'W')
         {
             Caches[threadid].writehitCL(clindex);
         } 
         else
         {
             assert(0);
         }
 
         // timestamp 0 addr 8 rw
         ns = __builtin_ia32_rdtsc() * CPU_GHZ_INV;
         sample[1].ns = ns;
         sample[1].addr = clindex * 64;
         sample[1].r = 'R';
         sample[1].size = 64;
         TraceFiles[threadid].write((char *) &sample[1], sizeof(TraceSample));
     }
 }
 
 static VOID RecordWriteAddrSize(VOID* addr, INT32 size, THREADID threadid) {
     WriteAddr[threadid] = addr;
     WriteSize[threadid] = size;
 }
 
 static VOID RecordMemWrite(VOID* ip, THREADID threadid) {
     RecordMem(ip, 'W', WriteAddr[threadid], WriteSize[threadid], threadid);
 }
 
 VOID Instruction(INS ins, VOID* v) {
     // instruments loads using a predicated call, i.e.
     // the call happens iff the load will be actually executed
 
     if (INS_IsMemoryRead(ins) && INS_IsStandardMemop(ins)) {
         INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMem, IARG_INST_PTR, IARG_UINT32, 'R', IARG_MEMORYREAD_EA,
                                  IARG_MEMORYREAD_SIZE,  IARG_THREAD_ID, IARG_END);
     }
 
     if (INS_HasMemoryRead2(ins) && INS_IsStandardMemop(ins)) {
         INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMem, IARG_INST_PTR, IARG_UINT32, 'R', IARG_MEMORYREAD2_EA,
                                  IARG_MEMORYREAD_SIZE,  IARG_THREAD_ID, IARG_END);
     }
 
     // instruments stores using a predicated call, i.e.
     // the call happens iff the store will be actually executed
     if (INS_IsMemoryWrite(ins) && INS_IsStandardMemop(ins)) {
         INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordWriteAddrSize, IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE, 
                                  IARG_THREAD_ID, IARG_END);
 
         if (INS_IsValidForIpointAfter(ins)) {
             INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)RecordMemWrite, IARG_INST_PTR, IARG_THREAD_ID, IARG_END);
         }
         if (INS_IsValidForIpointTakenBranch(ins)) {
             INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)RecordMemWrite, IARG_INST_PTR, IARG_THREAD_ID, IARG_END);
         }
     }
 }
 
 /* ===================================================================== */
 
 VOID Fini(INT32 code, VOID* v) {
 }
 
 /* ===================================================================== */
 /* Main                                                                  */
 /* ===================================================================== */
 
 int main(int argc, char* argv[]) {
     if (PIN_Init(argc, argv)) {
         return Usage();
     }
 
     TraceFiles.reserve(100);
     Caches.reserve(100);
     WriteAddr.reserve(100);
     WriteSize.reserve(100);
 
     PIN_AddThreadStartFunction(ThreadStart, NULL);
     PIN_AddThreadFiniFunction(ThreadFini, NULL);
     INS_AddInstrumentFunction(Instruction, 0);
     PIN_AddFiniFunction(Fini, 0);
 
     // Never returns
 
     PIN_StartProgram();
 
     RecordMemWrite(0,0);
     RecordWriteAddrSize(0, 0,0);
 
     return 0;
 }
 
 /* ===================================================================== */
 /* eof */
 /* ===================================================================== */
 