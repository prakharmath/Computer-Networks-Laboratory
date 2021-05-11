#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#define maxm 100

int main()
{
    int sockfd, newsockfd; // Creating socket ids.
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    // Creating Scoket.
    if((sockfd=socket(AF_INET, SOCK_STREAM, 0))<0)
    {
        perror("[.....ERROR.....] Unable to create socket.\n");
        exit(0);

    }
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_port=htons(8181);
    
    // Binding socket.
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        perror("[.....ERROR.....] Unable to bind the socket.\n");
        close(sockfd);
        exit(0);
    }
    
    listen(sockfd, 5);
   
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen); // Accept the connection
    if(newsockfd<0)
    {
        perror("[.....ERROR.....] Unable to connect to client.\n");
        close(sockfd);
        exit(0);
    }
    printf("[.....SUCCESS.....] Connected to Client.\n");


    char filename[maxm], name_buff[maxm];
    for(int i=0; i<maxm; i++)
        filename[i]=name_buff[i]='\0';

    int flag;
    flag=recv(newsockfd, filename, maxm,0);

    while ((flag=recv(newsockfd, name_buff, maxm, MSG_DONTWAIT))>-1)
        strcat(filename, name_buff);


    int fd=open(filename, O_RDONLY);
    if(fd==-1)
    {
        perror("[.....ERROR.....] Unable to open file.\n");
        close(newsockfd);
        close(sockfd);
        exit(0);
    }
    printf("[.....SUCCESS.....] File open - name: %s.\n",filename);


    int nbyte_read=0;
    do{
        char server_buff[maxm];
        nbyte_read = read(fd, server_buff, maxm-1);
        server_buff[nbyte_read]='\0';
        send(newsockfd, server_buff, strlen(server_buff)+1, 0);
    }while(nbyte_read==maxm-1);


    printf("[.....SUCCESS.....] File is sent to client.\n");
    close(sockfd);
    close(newsockfd);
    close(fd);
    return 0;
    
}