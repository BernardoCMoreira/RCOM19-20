	#include <string.h>
	#include <stdio.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <signal.h>
	#include <netdb.h>
	#include <strings.h>
	#include <ctype.h>

	#define DEFAULT_SERVER_PORT 21



	void createFile(int sockfd, char* filename)
	{

		char answer = 'n';
		int num = access( filename, F_OK );
		if( num!= -1 ) {

			printf("File already exists, do you wish to overwrite it? (Y/N) \n");
			scanf("%c", &answer);
			printf("\n");			
		}
	
		 if(answer == 'y' || answer == 'Y' || num==-1){

			printf("Starting donwload of file:\n");

			FILE *file = fopen((char *)filename, "wb+");

			char* sockBuffer = (char*)malloc(sizeof(char));
			memset(sockBuffer,0,sizeof(char));

			int sockBufferRes = 1;

			while (read(sockfd, sockBuffer, 1) > 0) {

				sockBuffer = (char*)malloc(sizeof(char) * (sockBufferRes + 1));
				memset(sockBuffer,0,sizeof(char) * (sockBufferRes + 1));

				fwrite(sockBuffer, 1, 1, file);
			}

			fclose(file);

			printf("-File Downloaded Succesfully\n");

		}
		else if(answer == 'n'|| answer == 'N')
		{
			printf("-Operation aborted!\n");
		}
		
	}


	void parseUrl(char *url, char *username, char *password, char *hostname, char *path)
	{
	int state = 0;
	char header[] = "ftp://";
	int cur_field_index = 0;
	int i = 0;

	while (i < strlen(url))
	{
	switch (state)
	{

	case 0:
		if (url[i] == header[i] && i == 5)
			state = 1;
		else if(url[i] != header[i])
			printf("ERROR: Unrecognized header.");
		break;

	case 1:
		if (url[i] != ':')
		{
			username[cur_field_index] = url[i];
			cur_field_index++;
		}
		else
		{
			state = 2;
			cur_field_index = 0;
		}
		break;

	case 2:
		if (url[i] != '@')
		{
			password[cur_field_index] = url[i];
			cur_field_index++;
		}
		else
		{
			state = 3;
			cur_field_index = 0;
		}
		break;

	case 3:
		if (url[i] != '/')
		{

			hostname[cur_field_index] = url[i];
			cur_field_index++;
		}
		else
		{
			state = 4;
			cur_field_index = 0;
		}
		break;

		case 4:
			path[cur_field_index] = url[i];
			cur_field_index++;
			break;

		}

		i++;

	}
	printf("-Username: %s\n-Password: %s\n-Hostname: %s\n-Path :%s\n", username, password, hostname, path);
	}


	void parseFilename(char *path, char *filename){

	int file_index;
	int i = strlen(path);
	int my_bool = 0;
	while(i > 0) {
		if(path[i] == '/') {
			file_index = i+1;
			my_bool = 1;
		}
		i--;
	}
	if(my_bool == 0) {
		strcpy(filename,path);
	}

	else{
		memcpy(filename, &path[file_index], strlen(path) - file_index);
		filename[strlen(path) - file_index] = '\0';
	}
	printf("-Filename: %s\n", filename);
	}

	struct hostent * getIp(char* hostname) {

	struct hostent *h;

	if ((h = gethostbyname(hostname)) == NULL) {

		herror("gethostbyname");
		exit(1);
	}

	printf("-IP Address : %s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));

	return h;
	}



	char* getReply(int sockfd)
	{

	int state = 0;
	int i = 0;
	char char_received;
	char* replyCode = (char *)malloc(sizeof(char)*3);
	memset(replyCode, 0, sizeof(char)*3);

	while (state != 3)
	{

		read(sockfd, &char_received, 1);
		//printf("%c", char_received );

		switch (state)
		{
		case 0:
			if (char_received == ' ')
			{
				if (i != 3)
				{
					printf("ERROR: Reply code not recognized.\n");
					return "-1";
				}
				i = 0;
				state = 1;
			}
			else if (char_received == '-') {
				state = 2;
				i=0;
			}
			else if(isdigit(char_received)) {
				replyCode[i] = char_received;
				i++;
			}
			break;

		case 1:
			if (char_received == '\n')
			{
				//printf("-Code: %s\n", replyCode);
				return replyCode;
			}
			break;

		case 2:
			if (char_received == replyCode[i])
				i++;

			else if(i==3) {

				if (char_received  == ' ')
					state = 1;

				else if(char_received =='-')
					i=0;

			}
			break;
		}
	}
	return NULL;
	}

	char * buildCmd(char* cmd, char* cmdContent){

	char* res = (char *)malloc(strlen(cmd) + strlen(cmdContent) + 2);
	memset(res,0,strlen(cmd) + strlen(cmdContent) + 2);
	strcat(res, cmd);
	strcat(res, " ");
	strcat(res, cmdContent);
	strcat(res, "\n");

	return res;

	}

	void sendCmd(int sockfd, char cmd[]){

	write(sockfd, cmd, strlen(cmd));

	}


	int interpretReply(int sockfd, char* filename, int listenPortSockfd, char* backupCmd)
	{


	char *code;
	int first_digit = 0;

		//printf("Command sent: %s", backupCmd);

	while (1)
	{
		code = getReply(sockfd);
		first_digit = code[0] - '0';

		switch (first_digit)
		{
		case 1:
			if(strcmp(backupCmd, "retr") == 0) {
				createFile(listenPortSockfd, filename);
				break;
			}
			code = getReply(sockfd);
			break;
		case 2:
			return 0;
		case 3:
			return 1;
		case 4:
			//sendCmd(sockfd, backupCmd);
			break;
		case 5:
			printf("The command was not accepted and the requested action did not take place.\n");
			close(sockfd);
			exit(-1);
		}
	}
	}

	int getListenPort(int socketfd)
	{
	int state = 0;
	int i = 0;
	char* p1 = (char *)malloc(sizeof(char)*2);
	char* p2 = (char *)malloc(sizeof(char)*2);
	memset(p1,0,sizeof(char)*2);
	memset(p2,0,sizeof(char)*2);

	char char_received;

	while (state != 7)
	{
		read(socketfd, &char_received, 1);
		//printf("%c", char_received);

		switch (state)
		{
		case 0:
			if (char_received != ' ')
				i++;
			else
			{
				if (i != 3)
				{
					printf("ERROR: Reply code not recognized.\n");
					return -1;
				}
				i = 0;
				state = 1;
			}
			break;
		case 5:
			if (char_received != ',')
			{
				p1[i] = char_received;
				i++;
			}
			else
			{
				i = 0;
				state++;
			}
			break;
		case 6:
			if (char_received != ')')
			{
				p2[i] = char_received;
				i++;
			}
			else
				state++;
			break;
		default:
			if (char_received == ',')
											state++;
			break;

		}
	}

	return (atoi(p1)* 256 + atoi(p2));
	}



	int sendCommandInterpretResponse(int sockfd, char* cmd, char* filename, int listenPortSockfd)
	{
	char *code;
	int first_digit = 0;

	//printf("Command sent: %s", backupCmd);

	while (1)
	{
		code = getReply(sockfd);
		first_digit = code[0] - '0';

		switch (first_digit)
		{
		case 1:
			if(strcmp(cmd, "retr") == 0) {
				createFile(listenPortSockfd, filename);
				break;
			}
			code = getReply(sockfd);
			break;
		case 2:
			return 0;
		case 3:
			return 1;
		case 4:
			//sendCmd(sockfd, backupCmd);
			break;
		case 5:
			printf("The command was not accepted and the requested action did not take place.\n");
			close(sockfd);
			exit(-1);
		}
	}
	}


	int main(int argc, char *argv[])
	{
	//parse arguments
	char username[50];
	char path[50];
	char password[50];
	char hostname[50];
	char filename[50];
	memset(username,0,50);
	memset(path,0,50);
	memset(password,0,50);
	memset(hostname,0,50);
	memset(filename,0,50);
	printf("\nHere's the parsed info:\n");



	parseUrl(argv[1], username, password, hostname, path);

	parseFilename(path, filename);

	struct hostent *h = getIp(hostname);

	//establishing connection through the socket

	int sockfd;
	struct sockaddr_in server_addr;
	int bytes;

	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr))); /*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(DEFAULT_SERVER_PORT); /*server TCP port must be network byte ordered */

	printf("\nAttempting connection with server:\n");

	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("socket()");
		exit(0);
	}

	/*connect to the server*/
	if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("connect()");
		exit(0);
	}

	char *replyCode;
	replyCode = getReply(sockfd);

	/*reply code 220 means:
		*  2 - that the requested action has been completed
		*  2 - that it was pertaining to a connection
		*  0 - the server is ready for a new user
		*/
	if (strcmp(replyCode,"220") == 0)
	{
		printf("-Connection successful\n");
	}

	printf("\nAttempting log-in procedure:\n");

	printf("-Sending Username\n");

	int listenPortSockfd =-1;


	sendCmd(sockfd, buildCmd("user", username));

	int res = interpretReply(sockfd, filename, listenPortSockfd, buildCmd("user", username));

	if (res == 1)
	{		
		printf("-Sending Password\n");
		sendCmd(sockfd, buildCmd("pass", password));
		res = interpretReply(sockfd, filename, listenPortSockfd, buildCmd("pass", password));
	}


	if (res == 0)
	{
		printf("-Login successful\n");
	}


	printf("\nObtaining server listening port:\n");
	/*enables passive mode, server will now "listen" from
		* non-default port indicated in the next reply
		*/
	sendCmd(sockfd, buildCmd("pasv", ""));

	printf("-Entered passive mode\n");

	int listenPort = getListenPort(sockfd);

	printf("-Server listening from port: %d\n", listenPort);


	printf("\nConnecting server port:\n");

	struct sockaddr_in server_listen_addr;

	/*server address handling*/
	bzero((char*)&server_listen_addr,sizeof(server_listen_addr));
	server_listen_addr.sin_family = AF_INET;
	server_listen_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr))); /*32 bit Internet address network byte ordered*/
	server_listen_addr.sin_port = htons(listenPort); /*server TCP port must be network byte ordered */

	/*open an TCP socket*/
	if ((listenPortSockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("socket()");
		exit(0);
	}

	/*connect to the server*/
	if(connect(listenPortSockfd, (struct sockaddr *)&server_listen_addr, sizeof(server_listen_addr)) < 0) {
		perror("connect()");
		exit(0);
	}
	else
		printf("-Connection successful\n");


	printf("-Sending retrieve command:\n");

	sendCmd(sockfd, buildCmd("retr", path));

	if(interpretReply(sockfd, filename, listenPortSockfd, "retr") == 0) {

		close(listenPortSockfd);
		close(sockfd);
		exit(0);
	}
	else printf(" > ERROR in RETR response\n");

	close(listenPortSockfd);
	close(sockfd);
	exit(1);



	}
