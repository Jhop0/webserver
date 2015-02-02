/*
 * serverFunc.c
 *
 *  Created on: Jan 21, 2015
 *      Author: adminuser
 */
#include "webserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h> /* open, O_RDONLY */


/* 	=====================================================
 * 	handleCliReq
 * 	=====================================================
 */

void *handleCliReq(void *arg)
{
	char sendBuffer[SEND_BUFFER_SIZE];
	char recvBuffer[RECV_BUFFER_SIZE];
	char tmpRecvBuffer[RECV_BUFFER_SIZE];
	char *fileNameToken;
	char *getFileCmdToken;
	char *getToken;
	char *tmpFilePath;
	char *sendFilePath;
	ssize_t recvBytes;
	ssize_t sentBytes;
	int clientSocket;

	//printf ("DEBUG: *** <Enter handleClientReq>\n");
	for (;;)
	{
		tmpFilePath = (char *) malloc(sizeof(char) * GETFILE_PATH_MAX);
		sendFilePath = (char *) malloc(sizeof(char) * GETFILE_PATH_MAX);

		sprintf(tmpFilePath,"%s", filePath);
		//memcpy(tmpFilePath, filePath, sizeof(filePath));


		pthread_mutex_lock (&mtex);
		while (emptyQ() == 1)
		{  /* block if buffer empty */
			//printf("DEBUG: --> Thread [%010u] waiting ...\n",pthread_self());
			pthread_cond_wait (&mtex_cond, &mtex);
		}

		/* Get the next request from queue */
		clientSocket = deQ();
		queueCount--;
		threadsActive++;
		threadsWaiting--;
		printf("INFO:[%010u] deQ for conn socketfd [%d]\n", pthread_self(), clientSocket);
		pthread_mutex_unlock (&mtex);

		bzero((char *) &sendBuffer, sizeof(sendBuffer)+1);
		bzero((char *) &recvBuffer, sizeof(recvBuffer)+1);

		/* Read GetFile request from socket into buffer */
		recvBytes = recv(clientSocket,recvBuffer,RECV_BUFFER_SIZE,0);
		trimwhitespace(recvBuffer);
		printf("INFO:[%010u] Recd message:[%s] len:[%d]\n", pthread_self(),recvBuffer, strlen(recvBuffer));

		/* Check GetFile request syntax */
		strcpy(tmpRecvBuffer,recvBuffer);
		//printf("DEBUG: tmp:[%d] recv:[%d]\n", strlen(tmpRecvBuffer), strlen(recvBuffer));
		if ((strncmp(tmpRecvBuffer, GETFILE_CMD_PREFIX, strlen(GETFILE_CMD_PREFIX))) != 0)
		{
			sprintf(sendBuffer, "%s%d", GETFILE_STATUS_ERR, 0);
			//printf("DEBUG: \n[%s]: [%s]\n",tmpRecvBuffer, sendBuffer);
			if((sentBytes = send(clientSocket, sendBuffer, SEND_BUFFER_SIZE, 0)) < 0 )
			{
				error("ERROR: sending to client");
				return NULL;
			}
		}
		else
		{
			/* Parse GetFile request */
			//printf("DEBUG: tmp:[%s] recv:[%s]\n", tmpRecvBuffer, recvBuffer);
			getFileCmdToken = strtok(tmpRecvBuffer, " ");
			getToken = strtok(NULL, " ");
			fileNameToken = strtok(NULL, " ");
			trimwhitespace(fileNameToken);
			printf( "INFO:[%010u] parsed message: [%s][%s][%s]\n", pthread_self(), getFileCmdToken, getToken, fileNameToken );

			/* Format the target file path */
			//printf("DEBUG: conn fname:[%s] pointer:[%p]\n", tmpFilePath, &tmpFilePath);
			if (strcmp(tmpFilePath, ".") == 0)
			{
				sprintf(tmpFilePath, "./");
			}
			sprintf(sendFilePath,"%s%s", tmpFilePath, fileNameToken);
			//printf("DEBUG: filePath:[%s]\n", sendFilePath);

			/* Send the requested file */
			if (sendFile(clientSocket, sendFilePath) == 0)
				error ("ERROR: sending file to client");

		} /* If-else */

		if(close(clientSocket) == SOCKET_ERROR)
		{
			error("ERROR: Could not close CLIENT socket\n");
		}

		free(tmpFilePath);
		free(sendFilePath);

		pthread_mutex_lock (&mtex);
		threadsActive--;
		threadsWaiting++;
		pthread_mutex_unlock (&mtex);


	} /* End for */
	//pthread_exit(0);
	return NULL;
}


int sendFile(int sock, char *fName)
{
	int sendMsgCount; 				/* how many sending chunks, for debugging */
	ssize_t readFileBytes; 			/* bytes read from local file */
	ssize_t sentBytes; 				/* bytes sent to connected socket */
	ssize_t sentFileSize;
	char sendBuffer[SEND_BUFFER_SIZE];
	int fp;
	struct stat fStat;
	sendMsgCount = 0;
	sentFileSize = 0;

	bzero((char *) &sendBuffer, sizeof(sendBuffer)+1);

	/* Try to open file */
	//printf("File name [%s]\n",fName);
	if( (fp = open(fName, O_RDONLY)) < 0) /* can't open requested file */
	{
		sprintf(sendBuffer, "%s%d", GETFILE_STATUS_ERR, 0);
		printf("DEBUG:[%010u]: [%s]: [%s]\n", pthread_self(), fName, sendBuffer);
		if( (sentFileSize = send(sock, sendBuffer, SEND_BUFFER_SIZE, 0)) < 0 )
		{
			perror("Send file error");
			return 0;
		}
		sendMsgCount++;
	}
	else /* Open file successful */
	{
		if(fstat(fp,&fStat) < 0) return 0;
		sprintf(sendBuffer,"%s%d",GETFILE_STATUS_OK, fStat.st_size);
		printf("INFO:[%010u] Send response:[%s]\n", pthread_self(), sendBuffer);
		send(sock, sendBuffer, SEND_BUFFER_SIZE,0);
		/* Read file and stream to client */
		printf("INFO:[%010u] Send file:[%s] size:[%d]\n", pthread_self(), fName, fStat.st_size);
		while( (readFileBytes = read(fp, sendBuffer, RECV_BUFFER_SIZE)) > 0 )
		{
			if( (sentBytes = send(sock, sendBuffer, readFileBytes, 0)) < readFileBytes )
			{
				perror("Send file error");
				return 0;
			}
			sendMsgCount++;
			sentFileSize += sentBytes;
			//printf("DEBUG [send_file]: Sent bytes:[%d] read file bytes:[%d] bytes remaining:[%d]\n",sentBytes, readFileBytes, (fStat.st_size - sentFileSize));
		}
		close(fp);
		printf("####:[%010u] File Sent %d bytes in %d msgs\n", pthread_self(), sentFileSize, sendMsgCount);
	} /* End else */

	/* Update global send counter */
	pthread_mutex_lock (&mtex);
	sentFileCount++;
	pthread_mutex_unlock (&mtex);
	return sendMsgCount;
}


/* 	=====================================================
 * 	sigChld
 * 	=====================================================
 */

void sigChld(int signo)
{
	pid_t pid;
	int stat;
	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
	printf("child %d terminated\n", pid);
	/* for debugging only, i/o not recommended here */
	return;
}
