#include "assignment01_Jones_Dax_FunctionsFile.h"


void readInFile(struct ItemInfo *items)
{
    //Desc: Reads in file state.txt and places information into given struct array
    //Parameters: 
    // items: struct array to being filled with info from the file


    FILE *fp = fopen("items.txt", "r"); //open file state.txt


    char line[100]; //create buffer line for raw line
    int pos = 0, linePos = 0, i = 0; // pos: to keep array index of array desired to store, linePos: to keep array index through buffer line, i: keeps index of struct array: 


    char serial[15]; // array to store current serial number
    char itemName[100]; // array to store current item name
    char price[50]; // array to store current price
    char site[20]; // array to store current site

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


}

