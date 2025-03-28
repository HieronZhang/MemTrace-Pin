#include <iostream>
#include <fstream>
#include <unordered_set>
#include <queue>
#include <vector>
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
        printf("Usage: ./parse thread_num fortxt\n");
        printf("fortxt : 2\n");
        exit(0);
    }

    if (strcmp(argv[2], "2") == 0)
    {

        

        for (int i = 0; i < thread_num; i++)
        {
            std::string filename = "pinatrace.out." + std::to_string(i);
            std::string filename_txt = "pinatrace.out." + std::to_string(i) + ".txt";
            ifstream fin(filename);

            if(i==0){
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
            if(i>=28 && i<=69){
                TraceSample *sample = new TraceSample;
                char* byte_arr = (char*) sample;
                fin.read(byte_arr, sizeof(TraceSample));
                if (sample->ns>1)
                {
                    sample->ns = sample->ns - 1;
                    sample->addr = i;
                    sample->r = 'F';
                    sample->size = i;
                    data.push_back(sample);
                    std::cout << sample->ns << " " << sample->addr << " " << sample->r << " " << int(sample->size) << std::endl;
                }
            }

        }
        std::priority_queue<TraceSample*, std::vector<TraceSample*>, decltype(customLess)> overall_accesses(data.begin(), data.end(), customLess);
        ofstream outputFile;
        outputFile.open("new_pinatrace.out.0");
        for (; !overall_accesses.empty(); overall_accesses.pop())
        {
            TraceSample* sample1 = overall_accesses.top();
            //outputFile << sample1->ns << " " << sample1->addr << " " << sample1->r << " " << int(sample1->size) << std::endl;
            outputFile.write((char *)sample1, sizeof(TraceSample));
        }


        for (size_t i = 0; i < data.size(); i++)
        {
            delete data[i];
        }

        data.clear();




        for (int i = 69; i < thread_num; i++)
        {
            std::string filename = "pinatrace.out." + std::to_string(i);
            std::string filename_txt = "pinatrace.out." + std::to_string(i) + ".txt";
            ifstream fin(filename);

            if(i==69){
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
            if(i>=70 && i<=71){
                TraceSample *sample = new TraceSample;
                char* byte_arr = (char*) sample;
                fin.read(byte_arr, sizeof(TraceSample));
                if (sample->ns>1)
                {
                    sample->ns = sample->ns - 1;
                    sample->addr = i;
                    sample->r = 'F';
                    sample->size = i;
                    data.push_back(sample);
                    //std::cout << sample->ns << " " << sample->addr << " " << sample->r << " " << int(sample->size) << std::endl;
                }
            }

        }
        std::priority_queue<TraceSample*, std::vector<TraceSample*>, decltype(customLess)> overall_accesses(data.begin(), data.end(), customLess);
        ofstream outputFile;
        outputFile.open("new_pinatrace.out.69");
        for (; !overall_accesses.empty(); overall_accesses.pop())
        {
            TraceSample* sample1 = overall_accesses.top();
            //outputFile << sample1->ns << " " << sample1->addr << " " << sample1->r << " " << int(sample1->size) << std::endl;
            outputFile.write((char *)sample1, sizeof(TraceSample));
        }


        for (size_t i = 0; i < data.size(); i++)
        {
            delete data[i];
        }





        
        return 0;
    }

    
}
