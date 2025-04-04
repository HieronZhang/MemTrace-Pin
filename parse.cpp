#include <iostream>
#include <fstream>
#include <unordered_set>
#include <queue>
#include <vector>
#include <stdint.h>
#include <string.h>

using std::ifstream;
using std::ofstream;
struct TraceSample
{
    uint64_t ns;
    uint64_t addr;
    uint8_t r;
    uint8_t size;
} __attribute((__packed__));

// Using a custom function object to compare elements.
struct
{
    bool operator()(const TraceSample* l, const TraceSample* r) const { return l->ns > r->ns; }
} customLess;

int main(int argc, char **argv)
{
    int thread_num;
    uint64_t cl_foot_print = 0;
    uint64_t page_foot_print = 0;
    std::unordered_set<uint64_t> access_addr;
    std::unordered_set<uint64_t> access_addr_page;
    std::vector<TraceSample*> data;
    
    if (argc == 3)
    {
        thread_num = std::stoi(argv[1]);
    }
    else
    {
        printf("Usage: ./parse core_num fortxt\n");
        printf("fortxt : 1 only for footprint, 0 for uncompressed txt file\n");
        exit(0);
    }

    if (strcmp(argv[2], "2") == 0)
    {

        

        for (int i = 0; i < thread_num; i++)
        {
            std::string filename = "pinatrace.out." + std::to_string(i);
            std::string filename_txt = "pinatrace.out." + std::to_string(i) + ".txt";
            ifstream fin(filename);

            while (!fin.eof())
            {
                TraceSample *sample = new TraceSample;
                char* byte_arr = (char*) sample;
                fin.read(byte_arr, sizeof(TraceSample));
                if (sample->ns!=0)
                {
                    data.push_back(sample);
                    //std::cout << sample->ns << " " << sample->addr << " " << sample->r << " " << int(sample->size) << std::endl;
                }
            }
        }
        // std::priority_queue<TraceSample*, std::vector<TraceSample*>, decltype(customLess)> overall_accesses(data.begin(), data.end(), customLess);
        // ofstream outputFile;
        // outputFile.open("overall.trace");
        // for (; !overall_accesses.empty(); overall_accesses.pop())
        // {
        //     TraceSample* sample1 = overall_accesses.top();
        //     //outputFile << sample1->ns << " " << sample1->addr << " " << sample1->r << " " << int(sample1->size) << std::endl;
        //     outputFile.write((char *)sample1, sizeof(TraceSample));
        // }

        for (size_t i = 0; i < data.size(); i++)
        {
            delete data[i];
        }
        
        return 0;
    }

    for (int i = 0; i < thread_num; i++)
    {
        std::string filename = "pinatrace.out." + std::to_string(i);
        std::string filename_txt = "pinatrace.out." + std::to_string(i) + ".txt";
        ifstream fin(filename);
        ofstream outputFile;
        if (strcmp(argv[2], "0") == 0)
            outputFile.open(filename_txt);

        char byte_arr[sizeof(TraceSample)];
        TraceSample *sample = (TraceSample *)byte_arr;

        while (!fin.eof())
        {
            fin.read(byte_arr, sizeof(TraceSample));
            if (access_addr.insert(sample->addr).second)
            {
                cl_foot_print += 64;
            }
            if (access_addr_page.insert(sample->addr / (64 * 64)).second)
            {
                page_foot_print += 4096;
            }

            if (strcmp(argv[2], "0") == 0)
                outputFile << (sample->r == (uint8_t)'R'? "LD" : "ST") << " " << sample->addr << std::endl;
            // outputFile.write()
            // char buffer [100];
            // snprintf ( buffer, 100, "The half of %d is %d", 60, 60/2 );
            // printf("ns: %20ld, addr: 0x%016lx, r: %c, size: %2d\n",
            //     sample->ns, sample->addr, sample->r, sample->size);
            // }
        }
    }
    printf("clfoot_print: %20ld \n", cl_foot_print);
    printf("page_foot_print: %20ld \n", page_foot_print);
}
