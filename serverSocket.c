/*
 * serverSocket.c
 *
 *  Created on: Jan 22, 2015
 *      Author: adminuser
 */

#include "webserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


int socketInit (int listenPort)
{
	struct sockaddr_in Address; /* Internet socket address struct */
	int addressSize=sizeof(struct sockaddr_in);
	int serverPort;
	int newServerSocket;

	serverPort = listenPort;

	printf("--> Init socket\n");
	 /* make a socket */
	newServerSocket=socket(AF_INET,SOCK_STREAM,0);

	if (newServerSocket == SOCKET_ERROR)
	{
		printf("*** Could not make a socket\n");
		return 0;
	}

	/* fill address struct */
	Address.sin_addr.s_addr=INADDR_ANY;
	Address.sin_port=htons(serverPort);
	Address.sin_family=AF_INET;

	printf("--> Binding to port %d\n",serverPort);

	/* bind to a port */
	if(bind(newServerSocket,(struct sockaddr*)&Address,sizeof(Address))== SOCKET_ERROR)
	{
		printf("*** Could not connect to host\n");
		return 0;
	}
	 /*  get port number */
	getsockname(newServerSocket, (struct sockaddr *) &Address,(socklen_t *)&addressSize);
	printf("--> Opened socket as socket [%d] on port (%d)\n",newServerSocket, ntohs(Address.sin_port) );

		printf("Server\n\
			  sin_family        = %d\n\
			  sin_addr.s_addr   = %d\n\
			  sin_port          = %d\n"
			  , Address.sin_family
			  , Address.sin_addr.s_addr
			  , ntohs(Address.sin_port)
			);


	printf("--> Making a listen queue of %d elements\n",QUEUE_SIZE);
	/* establish listen queue */
	if(listen(newServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
	{
		printf("*** Could not listen\n");
		//return 0;
	}
	return newServerSocket;

}


