#include "assignment01_Jones_Dax_FunctionsFile.h"

//Desc: Reads in file state.txt and places information into given struct array
//Parameters: 
//  items: struct array to being filled with info from the file

void readInFile(struct ItemInfo *items)
{
    FILE *fp = fopen("items.txt", "r"); //open file state.txt


    char line[100]; //create buffer line for raw line
    int pos = 0, linePos = 0, i = 0; // pos: to keep array index of array desired to store, linePos: to keep array index through buffer line, i: keeps index of struct array: 


    char serial[15]; // array to store current serial number
    char itemName[100]; // array to store current item name
    char price[50]; // array to store current price
    char site[20]; // array to store current site

    while(fgets(line, 100, fp) != NULL) //fgets pull next line every loop
    {
        while(isdigit(line[linePos])) //Counts until hitting '.' or space
        {
            serial[pos++] = line[linePos]; //places char into serial array and increments both
            linePos++;
        }

        items[i].serial = atoi(serial); //converts serial array into int then places in struct

        pos = 0; //resets pos to 0 for next buffer array
        linePos++; //increments to move past '.' in line buffer

        while(line[linePos] != '$') //increments through name until hitting '$'
        {
            
            if(line[linePos] != ' ' && line[linePos] != '\t') //Ignores spaces and tabes
            {
                itemName[pos] = line[linePos]; //places current line index into itemName array
                pos++; //move to next pos in itemName array
            }
            else if(line[linePos] == ' ' && line[linePos + 1] != ' ') // Checks to see if the space is part of the name by checking the next char to see if it's a letter
            {
                itemName[pos] = line[linePos]; //places current line index into itemName array
                pos++; //move to next pos in itemName array
            }

            linePos++; //increment line postion no matter what character
        }
        
        strcpy(items[i].itemName, itemName); //copy itemName array buffer into struct
        pos = 0; //reset pos
        linePos++; //move past '$'

        while(line[linePos] != ' ' && line[linePos] != '\t') //Gathers chars until hitting a space or tab
        {
            price[pos] = line[linePos]; //place char into price array
            linePos++; //increment positions
            pos++;
        }
        
        strcpy(items[i].price, price); // copy price array into struct

        linePos += 3; //increment by 3 to get past "on" and "at"
        pos = 0; //reset pos to 0

        while(line[linePos] != '\n') //read until end of line
        {
            site[pos] = line[linePos]; //Copy line char into site array
            pos++; //increment positions
            linePos++;
        }

        strcpy(items[i].site, site); //copy site into struct

        memset(itemName, 0, 100); // 3 statements that erase structs to be used again
        memset(price, 0, 50);
        memset(site, 0, 20);
        pos = 0; // reset all positions
        linePos = 0;
        i++; // increment struct array count
    }


}

