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

void send(char* prefix, int id)
{
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    prefix_buf sbuf;
    size_t buf_length;

    // if (argc <= 1 || strlen(argv[1]) <3) {
    //     printf("Error: please provide prefix of at least three characters for search\n");
    //     printf("Usage: %s <prefix>\n",argv[0]);
    //     exit(-1);
    // }

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
    // else
    //     fprintf(stderr,"Message(%d): \"%s\" Sent (%d bytes)\n", sbuf.id, sbuf.prefix,(int)buf_length);
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

}

int main(int argc, char** argv) {

    if(argc < 3) {
        printf("Incorrect input. Please enter an int followed by prefixes");
    }

    int waitTime = atoi(argv[1]);
    int prefixCount = argc - 2;
    response_buf response;
    response_buf* responseArray;
    int buffer_length=sizeof(response_buf)-sizeof(long);

    //Loop through each prefix
    //Starts at 1 since example output shows msgsnds as 1-indexed
    for(int i = 1; i <= prefixCount; i++) {
        char* word = argv[i+1];
        int length = strlen(word);

        //Check if prefix is valid
        if(length < 3 || length > 20) {
            printf("Invalid length of prefix");
            continue;
        }

        //Send the message
        send(word, i);
        printf("Message(%d): \"%s\" Sent (%d bytes)",i, word,buffer_length);
        sleep(waitTime);

        //Receive one message so we can see total amount of passages
        response = receive();
        //Use this total amount to create an array of response_buf
        responseArray = malloc(response.count * sizeof(response_buf));
        responseArray[response.index] = response;

        //Now we must loop count-1 times since we've received one
        for(int j = 1; j < response.count; j++) {
            response = receive();
            responseArray[response.index] = response;
        }

        //Print report, looping for each passage
        printf("Report \"%s\"\n", word);
        for(int j = 0; j < response.count; j++) {
            //Not found
            if(responseArray[j].present == 0) {
                printf("Passage %d - %s - no word found\n", j, responseArray[j].location_description);
            }
            //Found
            else {
                printf("Passage %d - %s - %s\n", j, responseArray[j].location_description, responseArray[j].longest_word);
            }
        }
    }
}