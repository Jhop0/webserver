/*
 * webserver.c
 *
 *  Created on: Jan 15, 2015
 *      Author: adminuser
 */

#include "webserver.h"
#include <sys/socket.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void usage(void)
{
	printf("Usage:\n");
	printf("\twebserver [options]\n");
	printf("options:\n");
	printf("\t-p port (Default: 8888)\n");
	printf("\t-t number of worker threads (Default: 1, Range: 1-1000\n");
	printf("\t-f path to static files (Default: .)\n");
	printf("\t-h some help message\n");
	exit (8);
}


int main(int argc, char* argv[])
{
    int serverSocket;  /* handle to socket */
    /* Set default arg values */
    pthread_t tid[WORKER_THREADS_MAX];
    int i=0;
    int newSocket;
    struct sockaddr_in cliAddress;
    socklen_t cliAddrLen;


    /* Init the mutex */
    pthread_mutex_t mtex = PTHREAD_MUTEX_INITIALIZER;  	/* mutex lock for buffer */
    //pthread_cond_t mtex_cond = PTHREAD_COND_INITIALIZER; /* producer waits on this cond var */
    pthread_cond_init(&mtex_cond, NULL);

    /* Init the globals */
    serverPort = DEF_SERVER_PORT;
    filePath = DEF_FILE_PATH;
    int workerThreads = DEF_WORKER_THREADS;
    threadsActive = 0;
    queueCount = 0;
    int opt=0;

    memset(&cliAddress, 0, sizeof(cliAddress)); /* zero-fill cli_addr structure*/



    while ((opt = getopt(argc, argv, "p:t:f:h")) != -1)
    {
        switch(opt)
        {
			case 'p':
				serverPort = atoi(optarg);
				printf("\nOption value=%d", serverPort);
				break;
			case 't':
				workerThreads = atoi(optarg);;
				printf("\nOption value=%d", workerThreads);
				break;
			case 'f':
				filePath = optarg;
				trimwhitespace(filePath);
				printf("\nOption value=%s\n", filePath);
				break;
			case 'h':
				usage();
				break;
			case '?':
				printf("\nMissing argument(s)");
				break;
		 }
     }

    printf("\n--> Starting server ...\n");

    /* Bind and listen */
    if ((serverSocket = socketInit(serverPort)) == 0)
    {
    	error("Error creating server socket");
    }

    /* Create request Q */
    createQ();

    /* Create worker thread pool */
    printf("--> Creating [%d] worker threads .... \n",workerThreads);
    /* create/fork threads */
    for (i = 0; i < workerThreads; i++)
    {
    	pthread_create(&tid[i], NULL, handleCliReq, NULL);
    }

    threadsWaiting = workerThreads;
    while(1)
    {
    	cliAddrLen = sizeof(cliAddress);
    	if ((newSocket = accept(serverSocket, (struct sockaddr*) &cliAddress, &cliAddrLen)) == SOCKET_ERROR)
    	{
    			error("*** Could not establish client socket\n");

		}
    	/* Mutex local & queue the connection */
		pthread_mutex_lock (&mtex);
		printf("\n*****************************\n");
		printf("--> New conn on sock 	[%03d]\n", newSocket);
		printf("--> Threads active:  	[%03d]\n", threadsActive);
		printf("--> Threads waiting:    [%03d]\n", threadsWaiting);
		printf("--> Sent file count:    [%03d]\n", sentFileCount);

		enQ(newSocket);
		//DEBUG: displayQ();
		queueCount++;

		printf("--> Current Q count:    [%03d]\n", queueCount);
		printf("\n*****************************\n");
		pthread_cond_broadcast (&mtex_cond);
		pthread_mutex_unlock (&mtex);
     } /* End while */

    for (i = 0; i < workerThreads; i++)
    { /* wait/join threads */
    		pthread_join(tid[i], NULL);
    }
	printf("\nClosing Master socket");
	/* Close socket */
	if(close(serverSocket) == SOCKET_ERROR)
	{
	 printf("\nCould not close socket\n");
	 return 0;
	}

}
