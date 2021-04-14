/*
Author: Dax Jones
Email: dax.jones@okstate.edu
Date: 2/23/2021
Program Desc:

This program allows the user to create a set number of processes, each process
ask the user how many items it should buy, and then it buys that amount in random.
Each customer process then sends what it picked to a helper function that reads the messages
from a message queue and prints a recipet to both the console and a .txt
*/


#include "assignment01_Jones_Dax_FunctionsFile.h"

int main()
{
    server(); //kick off to start program
}