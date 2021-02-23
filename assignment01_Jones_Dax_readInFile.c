#include "assignment01_Jones_Dax_FunctionsFile.h"

void clear();

void readInFile()
{
    FILE *fp = fopen("items.txt", "r");


    char line[100];
    int pos = 0, linePos = 0, i = 0, j = 0;

    struct ItemInfo *items = mmap(NULL, 100000, 
                        PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, 
                        0, 0);
    char serial[15];
    char itemName[100];
    char price[50];
    char site[20];

    while(fgets(line, 100, fp) != NULL) 
    {
        while(isdigit(line[linePos]))
        {
            serial[pos++] = line[linePos];
            linePos++;
        }

        items[i].serial = atoi(serial);

        pos = 0;
        linePos++;

        while(line[linePos] != '$')
        {
            
            if(line[linePos] != ' ' && line[linePos] != '\t')
            {
                itemName[pos] = line[linePos];
                pos++;
            }
            else if(line[linePos] == ' ' && line[linePos + 1] != ' ')
            {
                itemName[pos] = line[linePos];
                pos++;
            }

            linePos++;
        }
        
        strcpy(items[i].itemName, itemName);
        pos = 0;
        linePos++;

        while(line[linePos] != ' ' && line[linePos] != '\t')
        {
            price[pos] = line[linePos]; 
            linePos++;
            pos++;
        }
        
        strcpy(items[i].price, price);

        linePos += 3;
        pos = 0;

        while(line[linePos] != '\n')
        {
            site[pos] = line[linePos];
            pos++;
            linePos++;
        }

        strcpy(items[i].site, site);

        memset(itemName, 0, 100);
        memset(price, 0, 50);
        memset(site, 0, 20);
        pos = 0;
        linePos = 0;
        i++;
    }

    server(items);
}

