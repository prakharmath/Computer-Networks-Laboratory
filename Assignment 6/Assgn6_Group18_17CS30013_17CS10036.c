/*
Gaurav Goyal(17CS30013)
Prakhar Bindal(17CS10036)
Group 18
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <syslog.h>
#include <err.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/ftp.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>

#define PORT 80
#define BUFF_SIZE 65535



struct sockaddr_in proxyaddr, inpaddr, cliaddr;
socklen_t len, len_cli;
int sock_in, maxfd, nready;
int opt = 1;
char buff[BUFF_SIZE];
int n = 1000;
fd_set read_set;
fd_set write_set;
int a, b;
int infd[1000];
int outfd[1000];
int num_connections = 0;



int max(int a, int b)
{
    return a>b? a : b;
}



int parser(int sockfd)
{
	char *method, *uri, *query, *protocol;

	int recved, sockin, sockout;
	char *buffer;

	sockin=sockfd;

	buffer=malloc(65535);

	recved=recv(sockin, buffer, 65535, 0);

	if(recved<0)
	{
        printf("Error in recv() call.\n");
        return -1;
    }
    else if(recved==0)
    {   
        printf("Client closed connection upexpectedly.\n");
        return -1;
    }
    else
    {
    	buffer[recved]='\0';

    	char *temp_string;
    	temp_string=calloc(strlen(buffer)+1, sizeof(char));

    	strcpy(temp_string, buffer);

    	method = strtok(buffer,  " \t\r\n");
        uri = strtok(NULL, " \t");
        protocol = strtok(NULL, " \t\r\n"); 

        printf("The method is %s\n", method);

        int flag=0;
        if(!strcmp(method, "GET") || !strcmp(method, "POST"))
            flag=1;

        if (flag==0)
        {
        	printf("Request is neither GET nor POST, so not parsed.\n");
            return -1;
        }

        printf("The uri is %s\n", uri);
        printf("The protocol is %s\n", protocol);

        query = strchr(uri, '?');
        if(query)
            *query++ = '\0';
        else 
            query = uri - 1;

        char *hhost, *hhostname;
        hhost="Host";

        int i;
        for(i=0; i<18; i+=1)
        {
        	char *hname, *hvalue, *t;

        	hname = strtok(NULL, "\r\n: \t"); 
            if(!hname) 
                break;
            hvalue = strtok(NULL, "\r\n");     
            while(*hvalue && *hvalue==' ') 
                hvalue++;

            if(strcmp(hname, hhost)==0)
                hhostname = hvalue;

            printf("[Header_Parsed]=> %s: %s\n", hname, hvalue);

            t=hvalue+strlen(hvalue)+1;
            if((t[1] == '\r') && (t[2] == '\n')) 
            	break;
        }

		struct hostent *hhh = gethostbyname(hhostname);
		const char *ip_address = (const char *) inet_ntoa( (struct in_addr) *((struct in_addr *) hhh->h_addr_list[0]));

		printf("Hostname is: %s\n", hhostname);
		printf("IP address is: %s\n", ip_address);

		struct sockaddr_in destaddr;

        destaddr.sin_family = AF_INET;
        destaddr.sin_port = htons(80);
        if (inet_pton(AF_INET, ip_address, &destaddr.sin_addr) < 0)
        {
            printf("Invalid proxy address.\n");
            return -1;
            exit(4);
        }

        if ((sockout = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            fprintf(stderr, "socket failed: %s\n", strerror(errno));
            return -1;
            exit(5);
        }

        int nb = fcntl(sockout, F_GETFL, 0);
	    if(nb < 0) 
	    {
	        printf("fcntl F_GETFL: FD %d: %s", sockout, strerror(errno));
	        exit(2);
	    }
	    if(fcntl(sockout, F_SETFL, nb | O_NONBLOCK) < 0) 
	    {
	        printf("fcntl F_SETFL: FD %d: %s", sockout, strerror(errno));
	        exit(3);
	    }

		connect(sockout, (struct sockaddr *)&destaddr, sizeof(destaddr));


		int bytes_sent, devered, bytes_left;

		bytes_sent=0;
		bytes_left=recved;

		char *send_buff;

		send_buff=temp_string;

		while (bytes_left > 0)
		{
            devered = write(sockout, send_buff, bytes_left);

            if(devered<=0)
            	continue;

		    if (devered != bytes_left)
		        memmove(send_buff, send_buff+devered, bytes_left-devered);
		    
		    bytes_left-= devered;
            bytes_sent += devered;
        }
            
        free(buffer);
        free(temp_string);
        return sockout;
    }
}



void socket_creation_fail()
{
      perror("Socket creation failed");
      perror("\n");
      exit(EXIT_FAILURE);
}
void non_blocking_fail()
{
      perror("The creation of non blocking socket filed");
      perror("\n");
      exit(EXIT_FAILURE);
}
void setsockopt_fail()
{
      perror("Setsockopt failure for socket in");
      perror("\n");
      exit(EXIT_FAILURE);
}
void bind_fail()
{
      perror("Socket bind failed");
      perror("\n");
      exit(EXIT_FAILURE);
}
void proxy_fail()
{
      perror("Invalid proxy server address");
      perror("\n");
      exit(EXIT_FAILURE);
}
void listen_fail()
{
      perror("Socket is Listen failed");
      perror("\n");
      exit(EXIT_FAILURE);
}
void set_read_write_set()
{
    FD_ZERO(&write_set);
    FD_ZERO(&read_set);
}
void default_set_all_parameters()
{
        set_read_write_set();
        FD_SET(0,&read_set);
        maxfd=1;
        FD_SET(sock_in, &read_set);
        maxfd=sock_in;
        int i=0;
        while(i<num_connections)
        {
            FD_SET(outfd[i],&read_set);
            FD_SET(outfd[i],&write_set);
            FD_SET(infd[i],&write_set);
            FD_SET(infd[i],&read_set);
            maxfd=max(maxfd,infd[i]);
            maxfd=max(maxfd,outfd[i]);
            i++;
        }
        maxfd=maxfd+1;
}
void accept_call()
{
    char str[100]; 
    inet_ntop(AF_INET, &(cliaddr.sin_addr), str, 100);
    int clientServerPort = (int) ntohs(cliaddr.sin_port);        
	printf("Connection accepted from ");
    printf("%s",str);
    printf(":");
    printf("%d",clientServerPort);
    printf("\n");

    if ((outfd[num_connections] = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {
        printf("socket() failed: ");
        printf("%s",strerror(errno));
        printf("\n");
        exit(0);
    }

    if (fcntl(outfd[num_connections], F_SETFL, O_NONBLOCK) == -1) 
    {
        printf("fcntl failed : ");
        printf("%s",strerror(errno));
        printf("\n");
        exit(EXIT_FAILURE);
    }

    outfd[num_connections]=parser(infd[num_connections]);

    num_connections++;
}
void read_call()
{
    memset(buff, 0, sizeof(buff));
    a = read(0, buff, sizeof(65535));
    printf("\n");
    printf("Keyboard Input: %s", buff);
    printf("%s",buff);
		printf("\n");
    if (strcmp(buff, "exit") == 0) 
    {

        for (int i = 0; i < num_connections; i++) 
        {
            close(infd[i]);
            close(outfd[i]);
        }
        close(sock_in);
        printf("The user has asked to exit the proxy server");
		printf("\n");
        printf("Everything closed and exiting. Bye!!");
        printf("\n");
        exit(0);
    } 
}
void read_write_call()
{
	for (int i = 0; i < num_connections; i++) 
    {

        char buff[65535];
        int a, b;

        if(FD_ISSET(infd[i], &read_set) && FD_ISSET(outfd[i], &write_set)) 
        {
            memset(buff, 0, sizeof(buff));

            outfd[i]=parser(infd[i]);

            if (errno == EPIPE) 
            {
                continue;
            }
        }

        if(FD_ISSET(outfd[i], &read_set) && FD_ISSET(infd[i], &write_set)) 
        {
            memset(buff, 0, sizeof(buff));
            a = read(outfd[i], buff, sizeof(65535));
            b = send(infd[i], buff, a, 0);
            if (errno == EPIPE) 
            {
                continue;
            }
        }

    }
}



int main(int argc, char *argv[]) {

    signal(SIGPIPE, SIG_IGN);

    if (argc != 2) {
        printf("Incorrect command line arguments passed.");
        printf("\n");
        printf("Correct usage:");
        printf("\n");
        printf("./SimProxy <listen port> <institute_proxy_IP> <institute_proxy_port>");
        printf("\n");
        return 0;
    }

    bzero(&inpaddr, sizeof(inpaddr));
    bzero(&proxyaddr, sizeof(proxyaddr));

    bzero(&buff, sizeof(buff));

    struct timeval timeout;
    timeout.tv_sec  = 0;
    timeout.tv_usec = 1;

    if ((sock_in = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    	socket_creation_fail();

    if (fcntl(sock_in, F_SETFL, O_NONBLOCK) == -1)
    	non_blocking_fail();

    if (setsockopt(sock_in, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    	setsockopt_fail();

    inpaddr.sin_family = AF_INET;
    inpaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    inpaddr.sin_port = htons(atoi(argv[1]));

    if (bind(sock_in, (struct sockaddr*)&inpaddr, sizeof(inpaddr)))
        bind_fail();

    if (listen(sock_in, n) < 0)
    	listen_fail();

    set_read_write_set();

    while (1) 
    {
       	default_set_all_parameters();
        
        nready = select(maxfd, &read_set, &write_set, NULL, &timeout);
        if (nready > 0) 
        {
            if(FD_ISSET(sock_in, &read_set))
            {
                if (num_connections < n && (infd[num_connections] = accept(sock_in, (struct sockaddr *)&cliaddr, &len_cli)) >= 0)
                	accept_call();
            }

            else if (FD_ISSET(0, &read_set)) 
            	read_call();
            
            read_write_call();
        }

    }

    return 0;
}