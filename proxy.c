//Tyler Toth


#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>	/* system type defintions */
#include <sys/socket.h>	/* network system functions */
#include <netinet/in.h>	/* protocol & struct definitions */
#include <netdb.h>		//functions and symbols for accessing databases

#define BACKLOG	10
#define BUF_SIZE	1024*20
#define SMALL_BUF	100
#define LISTEN_PORT	8884
#define IP_ADD	129.120.151.98 //cse05
//URL used http://129.120.151.98:8888/www.cse.unt.edu

int threadCount = BACKLOG;
void *client_handler(void *arg);
int checkBList(char *);

int main(int argc, char *argv[])
{
	int status, *sock_tmp;
	pthread_t a_thread;
	void *thread_result;

	struct sockaddr_in addr_mine;
	struct sockaddr_in addr_remote;
	int sock_listen;
	int sock_aClient;
	int addr_size;
	int reuseaddr = 1;


	sock_listen = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_listen < 0) {
       	error("socket() failed");
	       exit(0);
	}

	memset(&addr_mine, 0, sizeof (addr_mine));
	addr_mine.sin_family = AF_INET;
	addr_mine.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_mine.sin_port = htons((unsigned short)LISTEN_PORT);

	status = bind(sock_listen, (struct sockaddr *) &addr_mine, sizeof (addr_mine));
	if (status < 0) 
	{	perror("bind() failed");
	       close(sock_listen);
       	exit(1);
    	}

	status = listen(sock_listen, 5);
	if (status < 0) {
       	perror("listen() failed");
	       close(sock_listen);
       	exit(1);
	}	

	addr_size = sizeof(struct sockaddr_in);
	printf("waiting for a client\n");
	while(1) 
    	{	if (threadCount < 1) 
		{
    			sleep(1);
	    	}

    		sock_aClient = accept(sock_listen, (struct sockaddr *) &addr_remote, &addr_size);
	    	if (sock_aClient == -1){
    			close(sock_listen);
	        	exit(1);
	    	}
	
	    	printf("Got a connection from %s on port %d\n",
	                    inet_ntoa(addr_remote.sin_addr),
	                    htons(addr_remote.sin_port));
	    	sock_tmp = malloc(1);
    		*sock_tmp = sock_aClient;
	    	printf("thread count = %d\n", threadCount);
		    	threadCount--;
				status = pthread_create(&a_thread, NULL, client_handler,
		            (void *) sock_tmp);
 		if (status != 0) {
 			perror("Thread creation failed");
 			close(sock_listen);
 			close(sock_aClient);
 			free(sock_tmp);
        	exit(1);
 		}
    }

    return 0;
}

void *client_handler(void *sock_desc) 
{	
	int sock_recv = *(int*)sock_desc;

	struct sockaddr_in	recv_addr,serv_addr;
	int i, bytes_received, sent, sock_out;
	fd_set readfds;
	struct timeval		timeout={0,0};
	int incoming_len;
	struct sockaddr	remote_addr;
	struct hostent 	*server;		//host server???
	struct sockaddr_storage	addr_info;
	socklen_t addr_len = sizeof(addr_info);
	int recv_msg_size, bl;
	int send_len,bytes_sent, flag;
	char buf[BUF_SIZE], code[80], servAdd[30], ipAdd[15], modBuf[BUF_SIZE];
	int select_ret;
            
		
	 while (1) 								//business area
	 {     bytes_received=recv(sock_recv,buf,BUF_SIZE,0);		//read in information from the browser (client)		
       	buf[bytes_received]=0;
        	printf("\nReceived: \n%s\n",buf);

		char* tmp = strdup(buf);			//duplicate the line for strtok
		const char* tok;				//declare the token variable
		tok = strtok(tmp, " :/");			//pull out the protocol code
		strcpy(code, tok);	


		if( strcmp("GET", code) == 0)		//8+ lines??
		{	strcpy(modBuf,code);
			strcat(modBuf," /");		//now - "GET / "
			tok = strtok(NULL, " /");		//pull out server address
			strcpy(servAdd, tok);
			if( strcmp("site", servAdd) == 0)
			{	tok = strtok(NULL, " ");		//pull out protocol label
				strcat(modBuf, servAdd);
				strcat(modBuf, "/");
				strcat(modBuf, tok);
				flag = 1;
			}
			//strcat(modBuf, servAdd);		
			tok = strtok(NULL, "\n");		//pull out protocol label
			strcat(modBuf, " ");
			strcat(modBuf, tok);			//now - "GET / HTTP/x.x"
			strcpy(code, tok);
			strcat(modBuf, "\n");		//end of line
			tok = strtok(NULL, " ");		//pull out host: label
			strcat(modBuf, tok);
			tok = strtok(NULL, "\n");		//pull out the proxy information
			strcat(modBuf, " ");
			
			if (flag != 1)
				strcat(modBuf, servAdd);		//insert the server address to host line
			
			while(tok != NULL)
			{	tok = strtok(NULL, ": \n");		//pull out next string
				if (strcmp("Referer", tok) == 0)	//if a token refers to another path/domain name
				{	//strcat(modBuf, tok);		
					//strcat(modBuf, "\n");
					tok = strtok(NULL, "/\n");	//pull out potential address
					tok = strtok(NULL, "/\n");
					tok = strtok(NULL, "\r");
					if ( strcmp("site", tok) != 0)
						strcpy(servAdd, tok);
					strcat(modBuf, servAdd);
				
				}
				tok = strtok(NULL, "\n");		//pull out full line

			}
			strcat(modBuf, ":80\r\n\r\n");		//concat the address with port 80

			strcpy(ipAdd, servAdd);
			char* tmp2 = strdup(servAdd);			//duplicate the line for strtok
			const char* token;				//declare the token variable
			token = strtok(tmp2, " .");			//pull out the protocol code
			free(tmp2);

			if (strcmp("favicon", ipAdd) == 0)
			{				//if a Favicon is requested
				char* tmp1 = strdup(code);			//duplicate the line for strtok
				const char* token;				//declare the token variable
				token = strtok(tmp1, "\r");			//pull out the protocol code
				strcpy(buf, token);				//copy to bufferr
				strcat(buf, " ");				
				strcat(buf, "204 No Content");
				//strcat(buf, "404");				//now "HTTP/x.x 404"
				strcat(buf, "\n\n");				//204 No Content
	
				printf("\nModified Request Message: \n%s\n",buf);
				free(tmp1);

			}else		//means we got a legit request
			{
			
			printf("\nSERVER ADDRESS: %s\n", servAdd);

								//function call for server address (servAdd)~~~~~~~~~~~~~~~~~~
			//to check up the blacklist
			//bl = checkBList(servAdd);
			if (bl == 1)					//they're trying to look at one of the blacklist sites!!
			{	printf("\n\nWE BLOCKING YOU, EVILDOERS\n\n");
				//send bad request protocol?????
				char* tmp3 = strdup(code);			//duplicate the line for strtok
				const char* token;				//declare the token variable
				token = strtok(tmp3, "\r");			//pull out the protocol code
				strcpy(buf, token);				//copy to bufferr
				strcat(buf, " ");				
				strcat(buf, "204 No Content");
				//strcat(buf, "404");				//now "HTTP/x.x 404"
				strcat(buf, "\n\n");				//204 No Content
	
				printf("\nModified Request Message: \n%s\n",buf);
				send_len=strlen(buf);
				bytes_sent=send(sock_recv,buf,send_len,0); //send reply to browser (client)
				free(tmp3);
				
				break;

			}
			

			server = gethostbyname(servAdd);		//trying to get IP address from host DB???
			if (server == NULL)
				printf("gethostbyname() failed\n");
			
			printf("\n%s\n", server->h_name);						//prints the name
			
			printf("\nModified Request Message: %s\n",modBuf);

			/* new socket for the request */
			sock_out = socket(AF_INET, SOCK_STREAM, 0);
			if (sock_out < 0) error("ERROR opening socket");

			memset(&serv_addr,0,sizeof(serv_addr));	/* fill in the structure */
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_port = htons(80);		//assume port is 80????
			memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

			/* connect the socket */
			if (connect(sock_out,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
				error("ERROR connecting");


		FILE *fp;
		char url[100];
		strcpy(url,"./");
		strcat(url, servAdd);
		if( access((const char *) url, F_OK) != -1)		//file exists
		{	fp = fopen((const char *)url, "r");		//need to read from cache
			printf("\n Read from cache \n");
			while(fgets(buf, BUF_SIZE, fp) != NULL)
			{	//printf("Received from Cache: %s", buf);
				send_len=strlen(buf);
				bytes_sent=send(sock_recv,buf,send_len,0); //send reply to browser (client)
			
			}fclose(fp);		
			//send_len=strlen(buf);
			//bytes_sent=send(sock_recv,buf,send_len,0);	//send reply to browser (client)

		}else								//file doesn't exist
		{

		fp = fopen(url, "a");
		sent = 0;
		send_len=strlen(modBuf);
		do {
			bytes_sent=send(sock_out,modBuf,send_len,0);			//send the modified request to server
			if (bytes_sent < 0)
				printf("\nError writing SERVER message\n");
			if (bytes_sent == 0)
				break;
			sent+= bytes_sent;
		} while (sent < send_len);


		while( (bytes_received=recv(sock_out,buf,BUF_SIZE,0)) > 0)		//continuously read from the server while it hasnt run into a newline char
		{				//read reply from server
       	buf[bytes_received]=0;
		printf("\nReceived: %s\n",buf);

		//filter for language!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		fprintf(fp, "%s", buf);
		sent = 0;
		send_len=strlen(buf);
		do {
			bytes_sent=send(sock_recv,buf,send_len,0);			//send reply to browser (client)
			if (bytes_sent < 0)
				printf("\nError writing BROWSER message\n");			
			if (bytes_sent == 0)
				break;
			sent+= bytes_sent;
		} while (sent < send_len);

		
		}
		fclose(fp);

	}}}	//end of else
		
		free(tmp);
		free(sock_desc);
		break;
	}
	close(sock_recv);
	close(sock_out);

	return 0;

}


int checkBList(char *address)
{	char line[SMALL_BUF], *ret;
	char *uRL = address;
	strcat(uRL, "\n");
	FILE *blackListFile = fopen("blackList.txt", "r");
	if (blackListFile == NULL)			//error opening the file
		return 0;
	if (!blackListFile)			//if file doesn't exist return 0
		return 0;	
	while(fgets(line, SMALL_BUF, blackListFile) != NULL)	//grab a line from Blacklist
	{	printf("Received from BLACK LIST %s", line);
		if (strcmp(line, uRL) == 0)
		{	printf("\n!!!found %s\n", line);				
			fclose(blackListFile);
			return 1;	
		}
		
	}
	fclose(blackListFile);
	return 0;

}
