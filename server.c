#include "assignment01_Jones_Dax_FunctionsFile.h"

void customer(mqd_t, int, int *, int *, char);
void helper(mqd_t, struct ItemInfo *, char *, int, int *);
int* getMessage(mqd_t);
int getRandomOrder(int *, int *);


#define QUEUE_NAME "/mailbox" //define queue name  
#define PERMISSIONS 0660 //define permissions
#define MAX_MESSAGES 10 //define max messages
#define MAX_MESSAGE_SIZE 1024 //define max message size
#define MAX_BUFFER_SIZE MAX_MESSAGE_SIZE + 10


//=========================================
//             Server Process
//=========================================


//Desc: Main server function, it'll create the items struct and send it to be read, 
//  ask the user for a number of processes, ask the user for an order, and create 
//  N processes + 1 Helper and send them to do their jobs. The process waits for the
//  helper to signal all processes are complete before exiting.


void server()
{
    struct ItemInfo *items = mmap(NULL, 100000, // Creates struct array to hold all items from file
                        PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, 
                        0, 0);
    
    readInFile(items); //call to read in file to items

    int amountOfProcesses; //int to hold user assigned number of processes

    printf("Please enter a positive integer less than 45 for amount of costumers to make:\n");
    fflush(stdin);
    scanf("%d", &amountOfProcesses); //stores user given number in var

    if(amountOfProcesses < 1) //If the user put 0 or a neg number, exit
    {
        printf("No customers! Goodbye!");
        exit(1);
    }


    int i = 0; // var for counting processes made
    char letter = 65; // letter variable used to give processes a pseudo name



    struct mq_attr attr; //attr struct for mq_open

    attr.mq_flags = 0; //Designates blocking
    attr.mq_maxmsg = MAX_MESSAGES; 
    attr.mq_msgsize = MAX_MESSAGE_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t qd; //create queue desc var
    if(((qd = mq_open(QUEUE_NAME, O_RDWR | O_CREAT, PERMISSIONS, &attr)) == -1)) //opens message queue checking for errors
    {
        printf("Error making msgid!");
        perror("mq_open");
        exit(1);
    }

    mq_close(qd); // Close name and queue to ensure queue is clear
    mq_unlink(QUEUE_NAME);

    if(((qd = mq_open(QUEUE_NAME, O_RDWR | O_CREAT, PERMISSIONS, &attr)) == -1)) //make queue again
    {
        printf("Error making msgid!");
        perror("mq_open");
        exit(1);
    }

    int *canContinue = mmap(NULL, sizeof(int), //create shared psuedo boolean to signal continuations between helper and main server
                        PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, 
                        -1, 0);
    
    *canContinue = 0; //set to false

    
    
    
    char *order = malloc(sizeof(char) * amountOfProcesses + 2); //array for ordering

    char *orderUserReference = malloc((sizeof(char) * amountOfProcesses * 2) + 2); //array to display what processes user needs to order (temporary)

    orderUserReference[0] = letter; //Set first order to 'A'

    for(int j = 1; j < amountOfProcesses * 2 - 1; j++) // continue to build array in the case of N > 1
    {
        if(j % 2 == 0) //correct position for a letter
        {
            orderUserReference[j] = ++letter; //add letter
        }
        else
        {
            orderUserReference[j] = ','; //places ',' in correct spot
        }
    }


    printf("Please enter order for: (%s)\n", orderUserReference); //Ask to enter order
    fflush(stdin);
    scanf("%s", order); //take in user given order

    free(orderUserReference); //free temp order array




    

    pid_t pid = fork(); //fork creating helper

    if(pid == 0) //child sent to be helper
    {
        helper(qd, items, order, amountOfProcesses, canContinue);
    }


    int *ordersLeft = mmap(NULL, sizeof(int), // shared var to keep teack of remaining orders
                        PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, 
                        -1, 0);
    int *ordersToPick = mmap(NULL, sizeof(int) * 100, //shared int array of available items customers can pick
                        PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, 
                        -1, 0);
    *ordersLeft = 100; //sets to beginning amount of items

    for(int j = 0; j < 100; j++) //intializes array with item indexes in order
    {
        ordersToPick[j] = j;
    }


    while(pid > 0 && i < amountOfProcesses) //loop so parent creates N processes
    {
        pid = fork(); 
        
        if(pid == 0) //child gets sent to act as customer
        {
            customer(qd, amountOfProcesses - i, ordersLeft, ordersToPick, order[i]);
        }

        if(pid > 0) // wait until customer process is done to prevent multiple customers asking at once
            wait(NULL);
        i++; 
    }

    
    if(pid > 0) //if parent, signal helper to begin printing
    {
        *canContinue = 1; //turns to "true"
    }

    while(wait(NULL) != -1 || errno != ECHILD){;} // ensures all children are done
    
    while(!*canContinue){;} //waits until signal from helper to continue


    mq_close(qd);               //Closes all queues and mapped vars
    mq_unlink(QUEUE_NAME);

    munmap(canContinue, sizeof(int));
    munmap(ordersLeft, sizeof(int));
    munmap(ordersToPick, sizeof(int) * 100);
    free(order);

    printf("\n\nThank You!\n"); // last statement
}
















//==========================================
//          Customer Process
//==========================================



//Desc: customer process ask user how many items to pick, then picks that many items at random
//  and send that list to helper function through message queue
//Parameters: 
//  msgid: id for message queue
//  priority: given priority so message can be recieved in order
//  ordersLeft: num of orders left to pick from
//  ordersToPick: array holding available orders to pick from
//  letter: Psuedo name for process

void customer(mqd_t msgid, int priority, int *ordersLeft, int *ordersToPick, char letter)
{
    int nums[101]; // array of numbers + 1; element 0 is how many items user picked
    int temp; // var to hold user given N

    char pid[20]; // string to hold process id
    sprintf(pid, "%d", getpid()); //places pid in string var

    if(mq_send(msgid, pid, strlen(pid) + 1, priority) == -1) //send pid first
    {
        perror("msgsnd");
        exit(1);
    }

    do{ 
    printf("\nHello! I'm Process %c!\nPlease enter the number of items you want me to purchase (There are %d items left):\n", letter, *ordersLeft); //Ask to enter num of orders
    scanf("%d", &temp); //store user given N

    nums[0] = temp; //set first element to N

    }while(nums[0] > *ordersLeft || nums[0] < 0); //Ask user to enter N items to pick and loops if N < 0 & N > items left

    for(int i = 1; i < nums[0] + 1; i++) //Store randomly picked items into nums array
    {
        nums[i] = getRandomOrder(ordersLeft, ordersToPick); //call to get random number
    }

    char send[101]; //creates char array of 101 to equal nums and send it

    for(int i = 0; i < nums[0] + 1; i++) // match every element in both arrays
    {
        send[i] = nums[i];
    }

    if(mq_send(msgid, send, strlen(send) + 1, priority) == -1) //sends array
    {
        perror("msgsnd");
        exit(1);
    }

    exit(0); //exit process
}


//Desc: gets random number based on system time and swaps ordersToPick array in a method that ensures
//  the same order can't be picked twice
//Parameters:
//  ordersLeft: num of remaining orders to pick from, also used to create upper bounds for random number
//  ordersToPick: array of available indexes for items, arranged in a way to prevent duplicate picks
//Return:
//  orderNum: int of randomly picked index

int getRandomOrder(int *ordersLeft, int *ordersToPick)
{
    srand(time(0)); //set seed with system time

    int randomNum = rand() % *ordersLeft; //picks random number where [0, ordersLeft)

    int orderNum = ordersToPick[randomNum]; //order num = to value at ordersToPickIndex

    int temp = ordersToPick[randomNum]; // temp var to hold value for swap
    ordersToPick[randomNum] = ordersToPick[*ordersLeft - 1]; //puts last unpicked element into picked index
    ordersToPick[*ordersLeft - 1] = temp; //puts picked index into array pos so it cannot be picked again
    *ordersLeft -= 1; //decrement orders left

    return orderNum;// return randomly picked index
}


















//==========================================
//            HELPER Process
//==========================================


//Desc: Helper function that recieves all messages sent from customer processes and prints them to
//  both console and .txt receipt for each
//Parameters:
//  msgid: var to have access to message queue
//  itemList: Struct array that holds all items, used to retrieve which items customers picked
//  oder: char array of given order from user
//  numberOfProcess: number of customer processes made
//  canContinue: shared memory "boolean" to signal continuation between parent


void helper(mqd_t msgid, struct ItemInfo *itemList, char *order, int numberOfProcesses, int *canContinue)
{
    int i = 0; //loop int
    int *selections; //array to hold recieved selections from customers
    float total = 0; // float to hold total value
    int itemNum = 1;    //int to display sequence of picks
    char fileName[17] = "processReceipt"; //starting string for each receipt .txt name

    while(!*canContinue){;} // waits until server confirms it can continue  

    canContinue = 0; //set to 0 to make parent wait

    printf("\n\nHi! I'm the helper! Here's the summary of all the orders:\n=========================================================\n");
    

    for(i = 0; i < numberOfProcesses; i++) //loop through each customer process
    {
        fileName[14] = order[i]; //appends correct customer name to file

        FILE *ptr; //create file pointer

        ptr = fopen(fileName, "w+"); //open file with correct name

        if(ptr == NULL)
        {
            printf("Receipt could not be printed!");
        }

        char in[MAX_BUFFER_SIZE]; //char array to receive message

        if(mq_receive(msgid, in, MAX_BUFFER_SIZE, 0) == -1) //receive first message that contains pid
        {
            perror("msgrcv");
            printf("Welp");
            exit(1);
        }

        char pid[20]; // create array to store pid

        for (int k = 0; k < 20; k++) //place contents of in to pid
        {
            pid[k] = in[k];
        }

        selections = getMessage(msgid); // receive second message from customer and store array of choices
        

        printf("\nProcess %c(PID: %s):\n", order[i], pid);
        printf("ItemNum\tSerial\tItem Name\t\t\tPrice\tLocation\n");
        fprintf(ptr,"ItemNum\tSerial\tItem Name\tPrice\tLocation\n");
        printf("==========================================================================================\n");
        fprintf(ptr, "==========================================================================================\n");

        for(int j = 1; j < selections[0] + 1; j++) //go through every item selection and print the contents to console and file
        {
            printf("%d.\t%d\t%s\t\t\t$%s\t%s\n", itemNum, itemList[selections[j]].serial, itemList[selections[j]].itemName, itemList[selections[j]].price, itemList[selections[j]].site);
            fprintf(ptr, "%d.\t%d\t%s\t\t\t$%s\t%s\n", itemNum, itemList[selections[j]].serial, itemList[selections[j]].itemName, itemList[selections[j]].price, itemList[selections[j]].site);
            fflush(stdout);
            total += atof(itemList[selections[j]].price); //add to total
            itemNum++; //increment item num
        }

        printf("Final Price: $%.2f\n", total);
        fprintf(ptr, "Final Price: $%.2f\n", total);

        itemNum = 1; //reset item num
        total = 0.0; //reset total
        fclose(ptr); //close customer's file
    }

    free(selections); //free selections array

    *canContinue = 1; //signal to server that everything has finished
    exit(0);
}

//Desc: Function that receives message for helper process and returns array of customer selections
//Parameters:
//  msgid: message queue id to access message queue


int* getMessage(mqd_t msgid)
{
    char in[MAX_BUFFER_SIZE]; //array to store message
    int *s = malloc(101 * sizeof(int)); // array 


    if(mq_receive(msgid, in, MAX_BUFFER_SIZE, 0) == -1) //receive message
    {
        perror("msgrcv");
        printf("Welp");
        exit(1);
    }

    s[0] = in[0]; // set first elements equal so we know how many items to loop through
 
    for(int i = 1; i < s[0] + 1; i++) //set elements equal for as many items
    {
        s[i] = (int)in[i];
    }

    return s; //return array of ints
}