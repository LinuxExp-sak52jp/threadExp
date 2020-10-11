#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

void UsagePrint(char *execName)
{
    fprintf(stderr, "Usage:%s [options] addr [val|num]\n", execName);
    fprintf(stderr, "Args:\n");
    fprintf(stderr, "  addr: I/O Address(hex)\n");
    fprintf(stderr, "  val : Value to be wrote into addr(hex)\n");
    fprintf(stderr, "        (Only available in write mode. Default is 0)\n");
    fprintf(stderr, "  num : Num of datas read from addr(hex)\n");
    fprintf(stderr, "        (Only available in read mode. Default is 1)\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -r: Read mode(Default)\n");
    fprintf(stderr, "  -w: Write mode\n");
}

int AccessIO(uint32_t addr, int32_t mode, uint32_t val)
{
    // Initialize here
    // .......

    printf("addr=0x%08x,val=0x%08x,mode=%d\n", addr, val, mode);
    if (mode == 1) {
        // Write mode
        // Write to ...
    }
    else {
        // Read mode
        // Read ....
    }

    return 0;
}

int main(int ac, char *av[])
{
    int32_t mode = 0; // Read
    int32_t numOrVal = -1;
    uint32_t addr = 0;
    uint32_t val;

    if (ac < 2) {
        UsagePrint(av[0]);
        return -1;
    }
    for (int32_t i = 1; i < ac; i++) {
        if (strcmp(av[i], "-r") == 0) {
            // Read Mode
            mode = 0;
        }
        else if (strcmp(av[i], "-w") == 0) {
            // Write Mode
            mode = 1;
        }
        else if (addr == 0) {
            errno = 0;
            addr = strtoul(av[i], NULL, 16);
            if (errno != 0) {
                perror("main()");
                return -1;
            }
        }
        else if (numOrVal == -1) {
            errno = 0;
            numOrVal = strtol(av[i], NULL, 16);
            if (errno != 0) {
                perror("main()");
                return -1;
            }
        }
        else {
            UsagePrint(av[0]);
            return -1;
        }
    }

    if (mode == 0) {
        // Set Read Mode Default
        if (numOrVal == -1) {
            val = 1;
        }
        else if (numOrVal < 0) {
            fprintf(stderr, "Illegal num in read mode\n");
            return -1;
        }
        val = (uint32_t)numOrVal;
    }
    else {
        // Set Write Mode Default
        if (numOrVal == -1) {
            val = 0;
        }
        val = (uint32_t)numOrVal;
    }

    return AccessIO(addr, mode, val);
}
