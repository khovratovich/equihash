/*Code by Dmitry Khovratovich, 2016
CC0 license
*/

#include "pow.h"
#include "blake/blake2.h"
#include <algorithm>


static uint64_t rdtsc(void) {
#ifdef _MSC_VER
    return __rdtsc();
#else
#if defined(__amd64__) || defined(__x86_64__)
    uint64_t rax, rdx;
    __asm__ __volatile__("rdtsc" : "=a"(rax), "=d"(rdx) : : );
    return (rdx << 32) | rax;
#elif defined(__i386__) || defined(__i386) || defined(__X86__)
    uint64_t rax;
    __asm__ __volatile__("rdtsc" : "=A"(rax) : : );
    return rax;
#else
#error "Not implemented!"
#endif
#endif
}


using namespace _POW;
using namespace std;

void Equihash::InitializeMemory()
{
    uint32_t  tuple_n = ((uint32_t)1) << (n / (k + 1));
    Tuple default_tuple(k); // k blocks to store (one left for index)
    std::vector<Tuple> def_tuples(LIST_LENGTH, default_tuple);
    tupleList = std::vector<std::vector<Tuple>>(tuple_n, def_tuples);
    filledList= std::vector<unsigned>(tuple_n, 0);
    solutions.resize(0);
    forks.resize(0);
}

void Equihash::PrintTuples(FILE* fp) {
    unsigned count = 0;
    for (unsigned i = 0; i < tupleList.size(); ++i) {
        for (unsigned m = 0; m < filledList[i]; ++m) {
            fprintf(fp, "[%d][%d]:", i,m);
            for (unsigned j = 0; j < tupleList[i][m].blocks.size(); ++j)
                fprintf(fp, " %x ", tupleList[i][m].blocks[j]);
            fprintf(fp, " || %x", tupleList[i][m].reference);
            fprintf(fp, " |||| ");
        }
        count += filledList[i];
        fprintf(fp, "\n");
    }
    fprintf(fp, "TOTAL: %d elements printed", count);
}

void Equihash::FillMemory(uint32_t length) //works for k<=7
{
    uint32_t input[SEED_LENGTH + 2];
    for (unsigned i = 0; i < SEED_LENGTH; ++i)
        input[i] = seed[i];
    input[SEED_LENGTH] = nonce;
    input[SEED_LENGTH + 1] = 0;
    uint32_t buf[MAX_N / 4];
    for (unsigned i = 0; i < length; ++i, ++input[SEED_LENGTH + 1]) {
        blake2b((uint8_t*)buf, &input, NULL, sizeof(buf), sizeof(input), 0);
        uint32_t index = buf[0] >> (32 - n / (k + 1));
        unsigned count = filledList[index];
        if (count < LIST_LENGTH) {
            for (unsigned j = 1; j < (k + 1); ++j) {
                //select j-th block of n/(k+1) bits
                tupleList[index][count].blocks[j - 1] = buf[j] >> (32 - n / (k + 1));
            }
            tupleList[index][count].reference = i;
            filledList[index]++;
        }
    }
}

std::vector<Input> Equihash::ResolveTreeByLevel(Fork fork, unsigned level) {
    if (level == 0)
        return std::vector<Input>{fork.ref1, fork.ref2};
    auto v1 = ResolveTreeByLevel(forks[level - 1][fork.ref1], level - 1);
    auto v2 = ResolveTreeByLevel(forks[level - 1][fork.ref2], level - 1);
    v1.insert(v1.end(), v2.begin(), v2.end());
    return v1;
}

std::vector<Input> Equihash::ResolveTree(Fork fork) {
    return ResolveTreeByLevel(fork, forks.size());
}


void Equihash::ResolveCollisions(bool store) {
    const unsigned tableLength = tupleList.size();  //number of rows in the hashtable 
    const unsigned maxNewCollisions = tupleList.size()*FORK_MULTIPLIER;  //max number of collisions to be found
    const unsigned newBlocks = tupleList[0][0].blocks.size() - 1;// number of blocks in the future collisions
    std::vector<Fork> newForks(maxNewCollisions); //list of forks created at this step
    auto tableRow = vector<Tuple>(LIST_LENGTH, Tuple(newBlocks)); //Row in the hash table
    vector<vector<Tuple>> collisionList(tableLength,tableRow);
    std::vector<unsigned> newFilledList(tableLength,0);  //number of entries in rows
    uint32_t newColls = 0; //collision counter
    for (unsigned i = 0; i < tableLength; ++i) {        
        for (unsigned j = 0; j < filledList[i]; ++j)        {
            for (unsigned m = j + 1; m < filledList[i]; ++m) {   //Collision
                //New index
                uint32_t newIndex = tupleList[i][j].blocks[0] ^ tupleList[i][m].blocks[0];
                Fork newFork = Fork(tupleList[i][j].reference, tupleList[i][m].reference);
                //Check if we get a solution
                if (store) {  //last step
                    if (newIndex == 0) {//Solution
                        std::vector<Input> solution_inputs = ResolveTree(newFork);
                        solutions.push_back(Proof(n, k, seed, nonce, solution_inputs));
                    }
                }
                else {         //Resolve
                    if (newFilledList[newIndex] < LIST_LENGTH && newColls < maxNewCollisions) {
                        for (unsigned l = 0; l < newBlocks; ++l) {
                            collisionList[newIndex][newFilledList[newIndex]].blocks[l] 
                                = tupleList[i][j].blocks[l+1] ^ tupleList[i][m].blocks[l+1];
                        }
                        newForks[newColls] = newFork;
                        collisionList[newIndex][newFilledList[newIndex]].reference = newColls;
                        newFilledList[newIndex]++;
                        newColls++;
                    }//end of adding collision
                }
            }
        }//end of collision for i
    }
    forks.push_back(newForks);
    std::swap(tupleList, collisionList);
    std::swap(filledList, newFilledList);
}

Proof Equihash::FindProof(){
    FILE* fp = fopen("proof.log", "w+");
    fclose(fp);
    this->nonce = 1;
    while (nonce < MAX_NONCE) {
        nonce++;
        printf("Testing nonce %d\n", nonce);
        uint64_t start_cycles = rdtsc();
        InitializeMemory(); //allocate
        FillMemory(4UL << (n / (k + 1)-1));   //fill with hashes
        uint64_t fill_end = rdtsc();
        printf("Filling %2.2f  Mcycles \n", (double)(fill_end - start_cycles) / (1UL << 20));
        /*fp = fopen("proof.log", "a+");
        fprintf(fp, "\n===MEMORY FILLED:\n");
        PrintTuples(fp);
        fclose(fp);*/
        for (unsigned i = 1; i <= k; ++i) {
            uint64_t resolve_start = rdtsc();
            bool to_store = (i == k);
            ResolveCollisions(to_store); //XOR collisions, concatenate indices and shift
            uint64_t resolve_end = rdtsc();
            printf("Resolving %2.2f  Mcycles \n", (double)(resolve_end - resolve_start) / (1UL << 20));
           /* fp = fopen("proof.log", "a+");
            fprintf(fp, "\n===RESOLVED AFTER STEP %d:\n", i);
            PrintTuples(fp);
            fclose(fp);*/
        }
        uint64_t stop_cycles = rdtsc();

        double  mcycles_d = (double)(stop_cycles - start_cycles) / (1UL << 20);
        uint32_t kbytes = (tupleList.size()*LIST_LENGTH*k*sizeof(uint32_t)) / (1UL << 10);
        printf("Time spent for n=%d k=%d  %d KiB: %2.2f  Mcycles \n",
            n, k, kbytes,
            mcycles_d);

        //Duplicate check
        for (unsigned i = 0; i < solutions.size(); ++i) {
            auto vec = solutions[i].inputs;
            std::sort(vec.begin(), vec.end());
            bool dup = false;
            for (unsigned k = 0; k < vec.size() - 1; ++k) {
                if (vec[k] == vec[k + 1])
                    dup = true;
            }
            if (!dup)
                return solutions[i];
        }
    }
    return Proof(n, k, seed, nonce, std::vector<uint32_t>());
}

bool Proof::Test()
{
    uint32_t input[SEED_LENGTH + 2];
    for (unsigned i = 0; i < SEED_LENGTH; ++i)
        input[i] = seed[i];
    input[SEED_LENGTH] = nonce;
    input[SEED_LENGTH + 1] = 0;
    uint32_t buf[MAX_N / 4];
    std::vector<uint32_t> blocks(k+1,0);
    for (unsigned i = 0; i < inputs.size(); ++i) {
        input[SEED_LENGTH + 1] = inputs[i];
        blake2b((uint8_t*)buf, &input, NULL, sizeof(buf), sizeof(input), 0);
        for (unsigned j = 0; j < (k + 1); ++j) {
            //select j-th block of n/(k+1) bits
            blocks[j] ^= buf[j] >> (32 - n / (k + 1));
        }
    }
    bool b = true;
    for (unsigned j = 0; j < (k + 1); ++j) {
        b &= (blocks[j] == 0);
    }
    if (b && inputs.size()!=0)    {
        printf("Solution found:\n");
        for (unsigned i = 0; i < inputs.size(); ++i) {
            printf(" %x ", inputs[i]);
        }
        printf("\n");
    }
    return b;
}
