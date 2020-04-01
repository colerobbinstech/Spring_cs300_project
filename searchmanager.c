#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "longest_word_search.h"
#include "queue_ids.h"
#include <sys/unistd.h>
#include <signal.h>
#include <pthread.h>
#ifndef mac
size_t                  /* O - Length of string */
strlcpy(char       *dst,        /* O - Destination string */
        const char *src,      /* I - Source string */
        size_t      size)     /* I - Size of destination string buffer */
{
    size_t    srclen;         /* Length of source string */


    /*
     * Figure out how much room is needed...
     */

    size --;

    srclen = strlen(src);

    /*
     * Copy the appropriate amount...
     */

    if (srclen > size)
        srclen = size;

    memcpy(dst, src, srclen);
    dst[srclen] = '\0';

    return (srclen);
}
#endif

//Struct to keep track of copmletion status on each prefix
typedef struct status{
    int index;
    int count;
} status;

//Store prefixes, statuses, and count of prefixes
char **prefixArray;
status* statusArray;
int prefixCount = 0;

pthread_mutex_t lock;

//Handles Ctrl+C 
void sigintHandler(int sig_num) {
    //Print status
    printf(" ");
    for(int i = 0; i < prefixCount; i++) {
        if(statusArray[i].count == -1 || statusArray[i].index == -1) { //No responses received
            printf("%s - pending\n", prefixArray[i]);
        }
        else if(statusArray[i].count == -2 || statusArray[i].index == -2) { //Invalid prefix
            printf("%s - invalid\n", prefixArray[i]);
        }
        else if(statusArray[i].count == statusArray[i].index) { //All Responses Received
            printf("%s - completed\n", prefixArray[i]);
        }
        else { //Midway through
            printf("%s - %d of %d\n", prefixArray[i], statusArray[i].index, statusArray[i].count);
        }
    }
    //Sigint prints weren't flushing
    fflush(stdout);

    //Clear handler
    signal(SIGINT, sigintHandler);
}

void send(char* prefix, int id)
{
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    prefix_buf sbuf;
    size_t buf_length;

    key = ftok(CRIMSON_ID,QUEUE_NUMBER);
    if ((msqid = msgget(key, msgflg)) < 0) {
        int errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("(msgget)");
        fprintf(stderr, "Error msgget: %s\n", strerror( errnum ));
    }

    // We'll send message type 1
    sbuf.mtype = 1;
    strlcpy(sbuf.prefix,prefix,WORD_LENGTH);
    sbuf.id=id;
    buf_length = strlen(sbuf.prefix) + sizeof(int)+1;//struct size without long int type

    // Send a message.
    if((msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT)) < 0) {
        int errnum = errno;
        fprintf(stderr,"%d, %ld, %s, %d\n", msqid, sbuf.mtype, sbuf.prefix, (int)buf_length);
        perror("(msgsnd)");
        fprintf(stderr, "Error sending msg: %s\n", strerror( errnum ));
        exit(1);
    }
    else
        fprintf(stderr,"\nMessage(%d): \"%s\" Sent (%d bytes)\n\n", sbuf.id, sbuf.prefix,(int)buf_length);
}

response_buf receive()
{
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    response_buf rbuf;
    size_t buf_length;

    key = ftok(CRIMSON_ID,QUEUE_NUMBER);
    if ((msqid = msgget(key, msgflg)) < 0) {
        int errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("(msgget)");
        fprintf(stderr, "Error msgget: %s\n", strerror( errnum ));
    }
    // else
    //     fprintf(stderr, "msgget: msgget succeeded: msgqid = %d\n", msqid);


    // msgrcv to receive message
    int ret;
    do {
      ret = msgrcv(msqid, &rbuf, sizeof(response_buf), 2, 0);//receive type 2 message
      int errnum = errno;
      if (ret < 0 && errno !=EINTR){
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("Error printed by perror");
        fprintf(stderr, "Error receiving msg: %s\n", strerror( errnum ));
      }
    } while ((ret < 0 ) && (errno == 4));
    //fprintf(stderr,"msgrcv error return code --%d:$d--",ret,errno);
    return rbuf;
}

/** Takes time to wait and prefixes as input
 * Initializes mutex and sigint
 * 
 * 
 */
int main(int argc, char** argv) {

    if(argc < 3) {
        printf("Incorrect input. Please enter an int followed by prefixes");
    }

    int waitTime = atoi(argv[1]);
    prefixCount = argc - 2;
    //We now know the size of prefix and status arrays
    prefixArray = (char**) malloc(sizeof(char**) * prefixCount);
    statusArray = (status*) malloc(sizeof(status) * prefixCount);
    response_buf response;

    //Initialize mutex to protect global status and prefix arrays
    pthread_mutex_init(&lock, NULL);

    //Declare signal handler
    signal(SIGINT, sigintHandler);

    //Populate prefix arrays using mutex and strlcpy
    for(int i = 0; i < prefixCount; i++) {
        prefixArray[i] = (char*) malloc(sizeof(char) * WORD_LENGTH);
        
        pthread_mutex_lock(&lock);
        strlcpy(prefixArray[i], argv[i + 2], WORD_LENGTH);
        statusArray[i].count = -1; //Default starting values checked for by handler
        statusArray[i].index = -1;
        pthread_mutex_unlock(&lock);
    }

    //Loop through each prefix
    //Starts at 1 since example output shows msgsnds as 1-indexed
    for(int i = 1; i <= prefixCount; i++) {
        char* word = argv[i+1]; //prefix
        int length = strlen(word);
        int passageCount = -1;

        //Check if prefix is valid
        if(length < 3 || length > 20) {
            fprintf(stderr, "Invalid length of prefix: %s\n", word);
            pthread_mutex_lock(&lock);
            statusArray[i].count = -2; //Default ERROR values checked for by handler
            statusArray[i].index = -2;
            pthread_mutex_unlock(&lock);
            continue;
        }

        //Send the message
        send(word, i);

        //Receive one message so we can see total amount of passages
        response = receive();
        //Use this total amount to create an array of response_buf
        response_buf responseArray[response.count];
        responseArray[response.index] = response;
        passageCount = response.count;

        pthread_mutex_lock(&lock);
        statusArray[i-1].index = 1; //First response received
        statusArray[i-1].count = response.count;
        pthread_mutex_unlock(&lock);


        //Now we must loop count-1 times since we've received one
        for(int j = 1; j < response.count; j++) {
            #ifdef DEBUG
                //Slow down receives so I can properly test sigint
                sleep(1);
            #endif 
            response = receive();
            responseArray[response.index] = response;
            pthread_mutex_lock(&lock);
            statusArray[i-1].index++; //Increment how far along we are in status
            pthread_mutex_unlock(&lock);
        }

        //Print report, looping for each passage
        printf("Report \"%s\"\n", word);
        for(int j = 0; j < passageCount; j++) {
            //Not found
            if(responseArray[j].present == 0) {
                printf("Passage %d - %s - no word found\n", j, responseArray[j].location_description);
            }
            else { //Found
                printf("Passage %d - %s - %s\n", j, responseArray[j].location_description, responseArray[j].longest_word);
            }
        }
        sleep(waitTime);
    }
    send("   ", 0);
    printf("Exiting ...\n");
}