/*Code by Dmitry Khovratovich, 2016
CC0 license
*/
#include "pow.h"

#include <inttypes.h>
#include <ctime>
#include "string.h"
#include <cstdlib>

using namespace _POW;


void TestEquihash(unsigned n, unsigned k, Seed seed)
{
   Equihash equihash(n,k,seed);
	Proof p = equihash.FindProof();
    p.Test();
}

static void fatal(const char *error) {
    fprintf(stderr, "Error: %s\n", error);
    exit(1);
}

static void usage(const char *cmd) {
    printf("Usage: %s  [-n N] [-k K] "
        "[-s S]\n",
        cmd);
    printf("Parameters:\n");
    printf("\t-n N \t\tSets the tuple length of iterations to N\n");
    printf("\t-k K\t\tSets the number of steps to K \n");
    printf("\t-s S\t\tSets seed  to S\n");
}


int main(int argc, char *argv[]) {
    uint32_t n = 0, k=0;
    Seed seed;
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    /* parse options */
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        unsigned long input = 0;
        if (!strcmp(a, "-n")) {
            if (i < argc - 1) {
                i++;
                input = strtoul(argv[i], NULL, 10);
                if (input == 0 ||
                    input > 255) {
                    fatal("bad numeric input for -n");
                }
                n = input;
                continue;
            }
            else {
                fatal("missing -n argument");
            }
        }
        else if (!strcmp(a, "-k")) {
            if (i < argc - 1) {
                i++;
                input = strtoul(argv[i], NULL, 10);
                if (input == 0 ||
                    input > 20) {
                    fatal("bad numeric input for -k");
                }
                k = input;
                continue;
            }
            else {
                fatal("missing -k argument");
            }
        }
        if (!strcmp(a, "-s")) {
            if (i < argc - 1) {
                i++;
                input = strtoul(argv[i], NULL, 10);
                if (input == 0 ||
                    input > 0xFFFFFF) {
                    fatal("bad numeric input for -s");
                }
                seed = Seed(input);
                continue;
            }
            else {
                fatal("missing -s argument");
            }
        }
    }
    printf("N:\t%" PRIu32 " \n", n);
    printf("K:\t%" PRIu32 " \n", k);
    printf("SEED: ");
    for (unsigned i = 0; i < SEED_LENGTH; ++i) {
        printf(" \t%" PRIu32 " ", seed[i]);
    }
    printf("\n");
    printf("Memory:\t\t%" PRIu64 "KiB\n", ((((uint32_t)1) << (n / (k + 1)))*LIST_LENGTH*k*sizeof(uint32_t)) / (1UL << 10));
    TestEquihash(n,k,seed);

    return 0;
}
