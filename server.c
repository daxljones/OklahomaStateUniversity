#include "assignment01_Jones_Dax_FunctionsFile.h"

void customer(mqd_t, int, int *, int *, char);
void helper(mqd_t, struct ItemInfo *, char *, int, int *);
int* getMessage(mqd_t, char);
int getRandomOrder(int *, int *);


#define QUEUE_NAME "/mailbox"
#define PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MESSAGE_SIZE 256
#define MAX_BUFFER_SIZE MAX_MESSAGE_SIZE + 10


//=========================================
//             Server Process
//=========================================

void server()
{
    struct ItemInfo *items = mmap(NULL, 100000, 
                        PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, 
                        0, 0);
    
    readInFile(items); //call to read in file to items

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
    char letter = 65;



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

    mq_close(qd);
    mq_unlink(QUEUE_NAME);

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

    
    
    
    char *order = malloc(sizeof(char) * amountOfProcesses + 2); //array for ordering

    char *orderUserReference = malloc((sizeof(char) * amountOfProcesses * 2) + 2); //array for ordering

    orderUserReference[0] = letter;

    for(int j = 1; j < amountOfProcesses * 2 - 1; j++)
    {
        if(j % 2 == 0)
        {
            orderUserReference[j] = ++letter;
        }
        else
        {
            orderUserReference[j] = ',';
        }
    }


    printf("Please enter order for: (%s)\n", orderUserReference); //Ask to enter order
    fflush(stdin);
    scanf("%s", order);

    free(orderUserReference);




    

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
        pid = fork();
        
        if(pid == 0)
        {
            customer(qd, amountOfProcesses - i, ordersLeft, ordersToPick, order[i]);
        }

        if(pid > 0)
            wait(NULL);
        i++;
    }

    
    if(pid > 0)
    {
        *canContinue = 1;
    }

    while(wait(NULL) != -1 || errno != ECHILD){;}
    while(!*canContinue){;}


    mq_close(qd);
    mq_unlink(QUEUE_NAME);

    munmap(canContinue, sizeof(int));
    munmap(ordersLeft, sizeof(int));
    munmap(ordersToPick, sizeof(int) * 100);
    free(order);

    printf("\n\nThank You!\n");
}


//==========================================
//          Customer Process
//==========================================

void customer(mqd_t msgid, int priority, int *ordersLeft, int *ordersToPick, char letter)
{
    int nums[101];
    int temp;

    char pid[20];
    sprintf(pid, "%d", getpid());
    //printf("Process id: %s", pid);

    if(mq_send(msgid, pid, strlen(pid) + 1, priority) == -1)
    {
        perror("msgsnd");
        exit(1);
    }

    do{
    printf("\nHello! I'm Process %c!\nPlease enter the number of items you want me to purchase (There are %d items left):\n", letter, *ordersLeft); //Ask to enter num of orders
    scanf("%d", &temp); //Place in order

    nums[0] = temp;

    }while(nums[0] > *ordersLeft || nums[0] < 0);

    for(int i = 1; i < nums[0] + 1; i++)
    {
        nums[i] = getRandomOrder(ordersLeft, ordersToPick);
    }

    char test[101];

    for(int i = 0; i < nums[0] + 1; i++)
    {
        test[i] = nums[i];
    }

    if(mq_send(msgid, test, strlen(test) + 1, priority) == -1)
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

    canContinue = 0;

    printf("\n\nHi! I'm the helper! Here's the summary of all the orders:\n=========================================================\n");
    

    for(i = 0; i < numberOfProcesses; i++)
    {
        fileName[14] = order[i];

        FILE *ptr;

        ptr = fopen(fileName, "w+");

        if(ptr == NULL)
        {
            printf("Receipt could not be printed!");
        }

        char in[MAX_BUFFER_SIZE];

        if(mq_receive(msgid, in, MAX_BUFFER_SIZE, 0) == -1) //receive message
        {
            perror("msgrcv");
            printf("Welp");
            exit(1);
        }

        char pid[20];

        for (int k = 0; k < 20; k++)
        {
            pid[k] = in[k];
        }

        selections = getMessage(msgid, order[i]);
        

        printf("\nProcess %c(PID: %s):\n", order[i], pid);
        printf("ItemNum\tSerial\tItem Name\t\t\tPrice\tLocation\n");
        fprintf(ptr,"ItemNum\tSerial\tItem Name\tPrice\tLocation\n");
        printf("==========================================================================================\n");
        fprintf(ptr, "==========================================================================================\n");

        for(int j = 1; j < selections[0] + 1; j++)
        {
            printf("%d.\t%d\t%s\t\t\t$%s\t%s\n", itemNum, itemList[selections[j]].serial, itemList[selections[j]].itemName, itemList[selections[j]].price, itemList[selections[j]].site);
            fprintf(ptr, "%d.\t%d\t%s\t\t\t$%s\t%s\n", itemNum, itemList[selections[j]].serial, itemList[selections[j]].itemName, itemList[selections[j]].price, itemList[selections[j]].site);
            fflush(stdout);
            total += atof(itemList[selections[j]].price);
            itemNum++;
        }

        printf("Final Price: $%.2f\n", total);
        fprintf(ptr, "Final Price: $%.2f\n", total);

        itemNum = 1;
        total = 0.0;
        fclose(ptr);
    }

    free(selections);

    *canContinue = 1;
    exit(0);
}

int* getMessage(mqd_t msgid, char sender)
{
    char in[MAX_BUFFER_SIZE];
    int *s = malloc(101 * sizeof(int));

    unsigned int senderID = sender;

    if(mq_receive(msgid, in, MAX_BUFFER_SIZE, 0) == -1) //receive message
    {
        perror("msgrcv");
        printf("Welp");
        exit(1);
    }

    s[0] = in[0];

    for(int i = 1; i < s[0] + 1; i++)
    {
        s[i] = (int)in[i];
    }

    return s;
}