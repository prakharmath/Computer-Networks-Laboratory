#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#define maxm 100
#define MAX_LINE 1024

//typedef char string[maxm + 1];
/*
Funtion to receive the string from the client
=========
Input:
sockfd : socket id 
buffer : character buffer for storing the received output
cliaddr: The address of the client
=========
 */


void udpreceive(int sockfd, char buffer[], struct sockaddr_in *cliaddr)
{
    int n;
    socklen_t len;
    len=sizeof(*cliaddr);
    n=recvfrom(sockfd, (char *)buffer, MAX_LINE, 0, (struct sockaddr *)cliaddr, &len);
    buffer[n]='\0';
}


//finding max of number
int max(int a, int b)
{
    if(a>b)
        return a;
    else 
        return b;
}


// get IP from host
int hostname_to_ip(char *hostname, char *ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ((he=gethostbyname(hostname)) == NULL)
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }

    addr_list=(struct in_addr **)he->h_addr_list;

    for (i=0; addr_list[i]!=NULL; i++)
    {
        //Return the first one;
        strcat(ip, inet_ntoa(*addr_list[i]));
        strcat(ip, ", ");
        // return 0;
    }

    return 1;
}



int main()
{
    int sockfd, newsockfd, sockfd2, nfds;
    socklen_t clilen, clilen2;
    struct sockaddr_in cli_add1, cli_add2, serv_addr;
    fd_set readSockSet;

    // struct timeval timeout;
    //creating socket 1
    if((sockfd=socket(AF_INET, SOCK_STREAM,0))<0)
    {
        perror("[.....ERROR.....] Unable to create the TCP socket\n");
        exit(1);
    }

    // creating socket 2
    if((sockfd2=socket(AF_INET,SOCK_DGRAM, 0))<0)
    {
        perror("[.....ERROR.....] Unable to create the UDP socket\n");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    memset(&cli_add1, 0, sizeof(cli_add1));
    memset(&cli_add2, 0, sizeof(cli_add2));

    // setting server
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(8181);

    // bind the socket 1 
    if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
    {
        perror("[.....ERROR.....] Unable to bind the TCP socket\n");
        exit(1);
    }
    // bind the socket 2
    if(bind(sockfd2,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
    {
        perror("[.....ERROR.....] Unable to bind the UDP socket\n");
        exit(1);
    }

    listen(sockfd, 5);

    while(1)
    {
        clilen = sizeof(cli_add1);
        clilen2 = sizeof(cli_add2);
        
        FD_ZERO(&readSockSet);
        FD_SET(sockfd, &readSockSet);
        FD_SET(sockfd2, &readSockSet);

        // timeout.tv_sec
        nfds = max(sockfd,sockfd2)+1;
        int ret = select(nfds, &readSockSet, 0, 0, 0); 

        //printf("%d, %d\n", nfds, ret);

        if (ret< 0)
        {
            perror("[.....ERROR.....] Unable to make select call\n");
            exit(1);
        }
        else
        {
            printf("\nSocket is selected for communication.\n");

            if(FD_ISSET(sockfd, &readSockSet))
            {
                pid_t child;
                printf("TCP socket is chosen.\n");

                if((child=fork())==0)
                {
                    newsockfd = accept(sockfd, (struct sockaddr*)&cli_add1, &clilen); // Accept the connection
                    if(newsockfd<0)
                    {
                        perror("[.....ERROR.....] Unable to connect to client.\n");
                        close(sockfd);
                        exit(0);
                    }
                    printf("[.....SUCCESS.....] Connected to Client(TCP).\n\n\n");

                    char subdirectory_name[maxm];
                    for(int i=0; i<maxm; i+=1)
                            subdirectory_name[i]='\0';

                    int size_name=recv(newsockfd, subdirectory_name, maxm, 0);

                    char name[maxm];
                    memset(name, '\0', sizeof(name));
                    strcat(name, "images/");
                    strcat(name, subdirectory_name);
                    strcat(name, "/");

                    printf("Name of directory asked by client: %s\n", name);

                    DIR *dir;
                    struct dirent *dp;
                    dir=opendir(name);
                    
                    if(dir==NULL)
                    {
                        printf("[.....ERROR.....] Unable to open directory\n"); 
                        close(sockfd);
                        exit(0); 
                    }
                    else
                    {
                        while((dp=readdir(dir))!=NULL)
                        {
                            char s[maxm];
                            memset(s, '\0', sizeof(s));
                            strcat(s, name);
                            strcat(s, dp->d_name);
                            //strcat(name, "/");

                            printf("[.....Sending_Picture_in_Process.....] Picture: %s\n", s);

                            sleep(1);
                            send(newsockfd, s, strlen(s)+1, 0);
                            sleep(1);

                            FILE *picture;
                            picture = fopen(s, "rb");
                            if(picture==NULL)
                            {
                                return -1;
                                break;
                            }

                            /*printf("[.....Sending_Image_in_Process.....] Getting Picture Size to send to client.\n");
                            int size;
                            fseek(picture, 0, SEEK_END);
                            size = ftell(picture);
                            fseek(picture, 0, SEEK_SET);

                            //Send Picture Size
                            printf("[.....Sending_Image_in_Process.....] Sending Picture Size to client.\n");
                            write(newsockfd, &size, sizeof(size));*/

                            //Send Picture as Byte Array
                            printf("[.....Sending_Picture_in_Process.....] Sending Picture as Byte Array to client.\n\n");
                            char send_buffer[maxm]; // no link between BUFSIZE and the file size
                            int nb;
                            while(!feof(picture))
                            {
                                nb = fread(send_buffer, 1, sizeof(send_buffer), picture);
                                write(newsockfd, send_buffer, nb);
                                //printf("%d\n", nb);
                                if(nb==0)
                                    break;
                                // no need to bzero
                            }
                            sleep(1);
                            write(newsockfd, "\0", 1);
                        }
                    }
                    printf("[.....SUCCESS.....] Pictures in subdirectory sent to client.\n\n\n");
                    sleep(1);
                    write(newsockfd, "\0", 1);
                    return 0;
                }
            }

            if(FD_ISSET(sockfd2, &readSockSet))
            {
                printf("UDP socket is chosen.\n");
                char hostname[MAX_LINE];    //Buffer to store the output
                udpreceive(sockfd2, hostname, &cli_add2);
                char ip[1000];
                for(int i=0; i<1000; i++)
                ip[i]='\0';
                hostname_to_ip(hostname, ip);
                sendto(sockfd2, (const char*)ip, strlen(ip)+1, 0, (const struct sockaddr*)&cli_add2, clilen2);
                printf("[.....SUCCESS.....] IP address sent.\n\n\n");
            }


        }
        
    }
    return 0;
}