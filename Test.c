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

void printProcess(int, char);
void helper(int);

struct Item{
    char name;
    int price;
};

struct message_buffer{
    long msg_type;
    struct Item *ptr;
};



int main()
{
//    int *n = mmap(NULL, 10 * sizeof(int), 
//                             PROT_READ | PROT_WRITE, 
//                             MAP_SHARED | MAP_ANONYMOUS, 
//                             0, 0);
   
   
    int i = 0;
    char letter = 64;


    int msgid;
    if((msgid = msgget(IPC_PRIVATE, 0600 | IPC_CREAT)) == -1)
    {
        printf("Error making msgid!");
        exit(1);
    }

    pid_t pid = fork();

    if(pid == 0)
    {
        helper(msgid);
    }

    while(pid > 0 && i < 4)
    {
        letter += 1;
        pid = fork();
        i++;
    }

    if(pid == 0)
    {
        printProcess(msgid, letter);
    }


    while(wait(NULL) != -1 || errno != ECHILD){;}

    return 0;
}

void printProcess(int key, char letter)
{
    struct message_buffer message;
    struct Item *send = malloc(sizeof(char) + sizeof(int));

    send->name = 'B';
    send->price = letter + 100;
    
    //printf("Child: %c\n", letter);
    message.msg_type = (int)letter;
    message.ptr = send;
    
    if((msgsnd(key, &message, sizeof(message), IPC_NOWAIT)) == -1)
    {
        perror("msgsnd");
        exit(1);
    }

    //printf("Message Sent!\n");

    exit(1);
}

void helper(int msgid)
{ 
    struct message_buffer recieve;
    
    char *order = malloc(sizeof(char) * 4);

    order[0] = 'A';
    order[1] = 'B';
    order[2] = 'D';
    order[3] = 'C';

    int i = 0, temp = 0;

    for(i = 0; i < 4; i++)
    {
        printf("Looking for: %d at spot: %d\n", order[i], i);

        if(msgrcv(msgid, &recieve, sizeof(recieve), order[i], 0) == -1)
        {
            perror("msgrcv");
            exit(1);
        }

        printf("I got %d\n", recieve.ptr->price);
    }


    if(msgctl(msgid, IPC_RMID, NULL) == -1)
    {
        perror("msgctl");
    }
    exit(1);
}