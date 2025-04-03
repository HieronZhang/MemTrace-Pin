# Modified Pin Tool for Memory Trace Generation
This repo includes a modified Pin-3.31 tool (the "pinatrace") for per-core memory trace generation for any linux executable. The pin tool can support tracing the DRAM-level memory request generated by the CPU caches in a multi-thread (up to 16 threads) application. We achieve this by integrating a cache simulator into the Intel Pin tool and added support for multi-thread traces (See `pin-3.31/source/tools/SimpleExamples/pinatrace.cpp`).
The default setting assumes the LLC cache is partitioned to 2MB pieces, each piece work for one core (line 296).

## Usage
1. Directly trace a binary:
```
cd pin-3.31/source/tools/SimpleExamples 
make obj-intel64/pinatrace.so
cd ../../../..
./pin-3.31/pin -t pin-3.31/source/tools/SimpleExamples/obj-intel64/pinatrace.so -- ${YOUR EXECUTABLE}
```

2. Attach the pin tool to a running process:

```
./pin-3.31/pin -pid {PID} -t pin-3.31/source/tools/SimpleExamples/obj-intel64/pinatrace.so
```

The traces will be generated as a set of compressed files `pinatrace.out.{X}`, where X is the core number.


## Trace Format and Processing
You need to use the parser to parse the raw data (using the `parse.c`). It can analyze the footprint of the trace or uncompress it and generate .txt files.

```
make
./parse {thread_num} {1,0}        # 1: only generate footprint, 0: also output uncompressed trace files
```

The `parse.py` can be used to scale the timestamp of the memory trace, since the PIN instrumentation will slowdown the program by a lot. For example, the program `bfs`  normally takes 1 second to finish. But when tracing using the Pin tool, it takes 20 seconds. You can use the `parse.py` to repair the timestamp of the trace files:

```
python parse.py -t {thread_num} -s SCALE
// python parse.py -t 1 -s 20     (For example)
```


The format of trace is:
```
struct TraceSample
{
    uint64_t ns;
    uint64_t addr;
    uint8_t r;    //read or write
    uint8_t size;
} __attribute((__packed__));
```

The uncompressed file can look like this:

```
641030369676438 140732991872448 R 64
641030372692752 140732991872384 R 64
641030372694474 140732991872256 R 64
641030372694894 140732991872320 R 64
641030372695230 138683091111424 R 64
641030372704554 138683091110528 R 64
641030372710812 138683091106368 R 64
641030372711106 138683091107392 R 64
641030372711610 138683091110592 R 64
641030374151370 138683091110720 R 64
641030374244652 138683091110656 R 64
641030375323212 138683091111296 R 64
641030375324178 138683091107456 R 64
```
