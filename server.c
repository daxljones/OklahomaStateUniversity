#include "assignment01_Jones_Dax_FunctionsFile.h"

// boobs
void customer(mqd_t, char, int *, int *, struct ItemInfo *);
void helper(mqd_t, struct ItemInfo *, char *, int, int *);
int* getMessage(mqd_t, char);
int getRandomOrder(int *, int *);


#define QUEUE_NAME "/mailbox"
#define PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MESSAGE_SIZE 256
#define MAX_BUFFER_SIZE MAX_MESSAGE_SIZE + 10

void server(struct ItemInfo *items)
{
    int amountOfProcesses;

    printf("Please enter a positive integer less than 45 for amount of costumers to make:\n");
    fflush(stdin);
    scanf("%d", &amountOfProcesses);

    if(amountOfProcesses < 1)
    {
        printf("No customers! Goodbye!");
        exit(1);
    }


    int i = 0;
    char letter = 64;

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MESSAGE_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t qd;
    if(((qd = mq_open(QUEUE_NAME, O_RDWR | O_CREAT, PERMISSIONS, &attr)) == -1))
    {
        printf("Error making msgid!");
        perror("mq_open");
        exit(1);
    }

    int *canContinue = mmap(NULL, sizeof(int), 
                        PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, 
                        -1, 0);
    
    *canContinue = 0;

    // titties

    char *order = malloc(sizeof(char) * amountOfProcesses + 2); //array for ordering

    printf("Please enter order:\n"); //Ask to enter order
    fflush(stdin);
    scanf("%s", order);
    

    pid_t pid = fork();

    if(pid == 0)
    {
        helper(qd, items, order, amountOfProcesses, canContinue);
    }


    int *ordersLeft = mmap(NULL, sizeof(int), 
                        PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, 
                        -1, 0);
    int *ordersToPick = mmap(NULL, sizeof(int) * 100, 
                        PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, 
                        -1, 0);
    *ordersLeft = 100;

    for(int j = 0; j < 100; j++)
    {
        ordersToPick[j] = j;
    }


    while(pid > 0 && i < amountOfProcesses)
    {
        letter++;
        pid = fork();

        if(pid > 0)
            wait(NULL);
        i++;
    }

    if(pid == 0)
    {
        customer(qd, letter, ordersLeft, ordersToPick, items);
    }
    else if(pid > 0)
    {
        *canContinue = 1;
    }

    while(wait(NULL) != -1 || errno != ECHILD){;}

    mq_close(qd);
    mq_unlink(QUEUE_NAME);

    printf("\n\nThank You!\n");
}


//==========================================
//          Customer Process
//==========================================

void customer(mqd_t msgid, char processID, int *ordersLeft, int *ordersToPick, struct ItemInfo *itemList)
{
    int nums[101];
    int temp;

    do{
    printf("\nHello! I'm Process %c!\nPlease enter the number of items you want me to purchas (There are %d items left):\n", processID, *ordersLeft); //Ask to enter num of orders
    fflush(stdin);
    scanf("%d", &temp); //Place in order

    nums[0] = temp;

    }while(nums[0] > *ordersLeft || nums[0] < 0);

    for(int i = 1; i < nums[0] + 1; i++)
    {
        nums[i] = getRandomOrder(ordersLeft, ordersToPick);
    }
    
    unsigned int senderID = processID;
    char test[101];

    for(int i = 0; i < nums[0] + 1; i++)
    {
        test[i] = nums[i];
    }

    if(mq_send(msgid, test, strlen(test) + 1, 0) == -1)
    {
        perror("msgsnd");
        exit(1);
    }

    exit(0);
}

int getRandomOrder(int *ordersLeft, int *ordersToPick)
{
    srand(time(0));

    int randomNum = rand() % *ordersLeft;

    int orderNum = ordersToPick[randomNum];

    int temp = ordersToPick[randomNum];
    ordersToPick[randomNum] = ordersToPick[*ordersLeft - 1];
    ordersToPick[*ordersLeft - 1] = temp;
    *ordersLeft -= 1;

    return orderNum;
}

//==========================================
//            HELPER Process
//==========================================

void helper(mqd_t msgid, struct ItemInfo *itemList, char *order, int numberOfProcesses, int *canContinue)
{
    int i = 0; //loop int
    int *selections;
    float total = 0;
    int itemNum = 1;
    char fileName[17] = "processReceipt";

    while(!*canContinue){;}

    printf("\n\nHi! I'm the helper! Here's the summary of all the orders:\n=========================================================\n");
    

    for(i = 0; i < numberOfProcesses; i++)
    {
        selections = getMessage(msgid, order[i]);

        fileName[14] = order[i];

        FILE *ptr;

        ptr = fopen(fileName, "w+");

        if(ptr == NULL)
        {
            printf("Receipt could not be printed!");
        }

        printf("\nProcess %c:\n", order[i]);
        printf("ItemNum\tSerial\tItem Name\tPrice\tLocation\n");
        fprintf(ptr,"ItemNum\tSerial\tItem Name\tPrice\tLocation\n");
        printf("==========================================================================================\n");
        fprintf(ptr, "==========================================================================================\n");

        for(int j = 1; j < selections[0] + 1; j++)
        {
            printf("%d.\t%d\t%s\t$%s\t%s\n", itemNum, itemList[selections[j]].serial, itemList[selections[j]].itemName, itemList[selections[j]].price, itemList[selections[j]].site);
            fprintf(ptr, "%d.\t%d\t%s\t$%s\t%s\n", itemNum, itemList[selections[j]].serial, itemList[selections[j]].itemName, itemList[selections[j]].price, itemList[selections[j]].site);
            total += atof(itemList[selections[j]].price);
            itemNum++;
        }

        printf("Final Price: $%.2f\n", total);
        fprintf(ptr, "Final Price: $%.2f\n", total);

        itemNum = 1;
        total = 0.0;
        fclose(ptr);
    }


    exit(0);
}

int* getMessage(mqd_t msgid, char sender)
{
    char in[MAX_BUFFER_SIZE];
    int *s = malloc(101 * sizeof(int));

    unsigned int senderID = sender;

    if(mq_receive(msgid, in, MAX_BUFFER_SIZE, &senderID) == -1) //receive message
    {
        perror("msgrcv");
        printf("Welp");
        exit(1);
    }

    s[0] = in[0];

    for(int i = 1; i < s[0] + 1; i++)
    {
        s[i] = in[i];
    }

    return s;
}


void printReciept()
{

}