#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/wait.h>
#include <mqueue.h>

struct ItemInfo{ 
        int serial;
        char itemName[100];
        char price[50];
        char site[20];
};

void readInFile();
void server(struct ItemInfo *);