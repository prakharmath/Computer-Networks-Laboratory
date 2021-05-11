/*Created By-
Prakhar Bindal(17CS10036)
Gaurav Goyal(17CS30013)
Group 18*/
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h> 
#include <fcntl.h>
#include <signal.h>
#define PORT 8080
#define BUFF_SIZE 1024
struct sockaddr_in proxyaddr, inpaddr, cliaddr;
socklen_t len, len_cli;
int sock_in, maxfd, nready;
int opt = 1;
char buff[BUFF_SIZE];
struct timeval timeout;
int n=1000;
int infd[1000];
int outfd[1000];
int num_connections = 0;
fd_set read_set;
fd_set write_set;
char buff[1024];
char buff1[1024];
int a, b,a1,b1;
int arg1,arg3;
int max(int a, int b) 
{
    if(a>=b)
        return a;
    else
        return b;
}
int min(int a,int b)
{
       if(a<=b)
        return a;
       else
        return b;
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
    connect(outfd[num_connections], (struct sockaddr *)&proxyaddr, sizeof(proxyaddr));
    num_connections++;
    return;
}
void read_call()
{
     memset(buff1, 0, sizeof(buff1));
     a1 = read(0, buff1, sizeof(1024));
     printf("\n");
     printf("typed: ");
     printf("%s",buff);
     printf("\n");
     if (strcmp(buff1, "exit") == 0) 
     {
        int i=0;
        while(i<num_connections) 
        {
            close(infd[i]);
            close(outfd[i]);
            i++;
        }
        printf("The user has asked to exit the proxy server");
        printf("\n");
        close(sock_in);
        exit(0);
    } 
}
void read_write_call()
{
    int i=0;
    while(i<num_connections) 
    {
        char buff[1024];
        int a, b;
        if(FD_ISSET(infd[i], &read_set) && FD_ISSET(outfd[i], &write_set)) 
        {
            memset(buff, 0, sizeof(buff));
            a = read(infd[i], buff, sizeof(1024));
            b = send(outfd[i], buff, a, 0);
            if (errno == EPIPE) 
            {
                i++;
                continue;
            }
        }
        if(FD_ISSET(outfd[i], &read_set) && FD_ISSET(infd[i], &write_set)) 
        {
            memset(buff, 0, sizeof(buff));
            a = read(outfd[i], buff, sizeof(1024));
            b = send(infd[i], buff, a, 0);
            if (errno == EPIPE)
            {
                i++; 
                continue;
            }
        }
        i++;
    }
    return;
}
void initialise_sockets()
{
    bzero(&inpaddr, sizeof(inpaddr));
    bzero(&proxyaddr, sizeof(proxyaddr));
    bzero(&buff, sizeof(buff));
    timeout.tv_sec  = 0;
    timeout.tv_usec = 1;   
}
void set_sockets()
{
    inpaddr.sin_family = AF_INET;
    inpaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    inpaddr.sin_port = htons(arg1);
    proxyaddr.sin_family = AF_INET;
    proxyaddr.sin_port = htons(arg3);
}
int main(int argc, char *argv[]) 
{
    signal(SIGPIPE, SIG_IGN);
    if (argc != 4)
    {
        printf("Incorrect command line arguments passed.");
        printf("\n");
        printf("Correct usage:");
        printf("\n");
        printf("./SimProxy <listen port> <institute_proxy_IP> <institute_proxy_port");
        printf("\n");
        return 0;
    }
    initialise_sockets();
    if ((sock_in = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        socket_creation_fail();
    if (fcntl(sock_in, F_SETFL, O_NONBLOCK) == -1) 
        non_blocking_fail();
    if (setsockopt(sock_in, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        setsockopt_fail();
    arg1=atoi(argv[1]);
    arg3=atoi(argv[3]);
    set_sockets();
    if (inet_pton(AF_INET, argv[2], &proxyaddr.sin_addr) <= 0)
        proxy_fail();
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
            memset(buff1,0,sizeof(buff1));
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