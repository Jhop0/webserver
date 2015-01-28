/*
 * stringOps.c
 *
 *  Created on: Jan 19, 2015
 *      Author: adminuser
 */

#include "webserver.h"
#include <string.h>
#include <stdio.h>




void removeSubstring(char *s,const char *toremove)
{
  while(s=strstr(s,toremove))
    memmove(s,s+strlen(toremove),1+strlen(s+strlen(toremove)));
}

char* trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

char* getFileReqParse(char* rBuffer, char** gFileCmd, char** gToken,char* fNameToken)
{
	char tmpRecvBuffer[RECV_BUFFER_SIZE];

	printf("--- enter getFileResParse\n");
	strcpy(tmpRecvBuffer,rBuffer);
	printf("tmpBuffer: %s\n", tmpRecvBuffer);
	*gFileCmd = strtok(tmpRecvBuffer, " ");
	printf( " %s\n", *gFileCmd );
	*gToken = strtok(NULL, " ");
	printf( " %s\n", *gToken );
	fNameToken = strtok(NULL, " ");
	printf( " %s\n", fNameToken );
	printf("--- exit getFileResParse\n");
	return fNameToken;

}
