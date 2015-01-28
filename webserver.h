/*
 * webserver.h
 *
 *  Created on: Jan 23, 2015
 *      Author: adminuser
 */

#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


#define SOCKET_ERROR       		 -1

/* GetFile protocol & buffers */
//#define BUFFER_SIZE        		 100
#define SEND_BUFFER_SIZE		4096
#define RECV_BUFFER_SIZE		4096
#define QUEUE_SIZE        	 	 10
#define GETFILE_CMD_PREFIX		"GetFile GET "
//#define GETFILE_CMD_1			"GetFile "
//#define GETFILE_CMD_2			"GET "
//#define GETFILE_CMD_PREFIX_SIZE	 12
#define GETFILE_PATH_MAX			255
#define GETFILE_MAX_FNAME_SIZE		200
#define GETFILE_STATUS_ERR		"GetFile FILE_NOT_FOUND 0 "
#define GETFILE_STATUS_OK		"GetFile OK "
#define WORKER_THREADS_MAX		1000

/* API Defaults */
#define DEF_SERVER_PORT				8888	/* -p port Server listening port */
#define DEF_WORKER_THREADS			10 		/* -t number of worker threads */
#define DEF_FILE_PATH			"." 	/* -f path to static files */

/* Global vars */
int serverPort;
char *filePath;
int queueCount;
int threadsActive;
int threadsWaiting;
int sentFileCount;

/* Create mutex for queue */
pthread_mutex_t mtex;  	/* mutex lock for buffer */
pthread_cond_t mtex_cond;


typedef struct
{
	int socket;
	struct sockaddr address;
	int addr_len;
	char *fPath;
	int tid;
} clientConn_t;


/* webserver */
void error(const char*);
void * process(void *);
void usage(void);

/* serversocket */
int socketInit(int);

/* serverOps */
void * handleCliReq(void *);
int sendFile(int, char*);
void sigChld(int);

/* stringOps */
void removeSubstring(char*, const char*);
char* trimwhitespace(char*);

/* queueOps */
int frontelement();
void enQ(int);
int deQ();
int emptyQ();
void displayQ();
void createQ();
int sizeQ();


#endif /* WEBSERVER_H_ */
