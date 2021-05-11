#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#define maxm 100




void parse_file(char client_buf[maxm], int sockfd, int fd, int* rec_byte, int* counter, int* words, int* bytes)
{
    int delim=1;
    while(((*rec_byte)=recv(sockfd, client_buf, maxm, 0))>0)
    {
        (*counter)++;
        // writing data to the file
        write(fd,client_buf,strlen(client_buf));
        int j=0;
        while((j<(*rec_byte)) && (client_buf[j]!='\0'))
        {
            (*bytes)++;
            // count the number of bytes and words
            if( (client_buf[j]==' ') || (client_buf[j]==':') || (client_buf[j]=='.') ||
                                        (client_buf[j]==';') || (client_buf[j]==',') || 
                                        (client_buf[j]=='\t') || (client_buf[j]=='\n') )
                delim=1;
            
            else
            {
                if(delim==1)
                {
                    (*words)++;
                    delim=0;
                }
            }
            j++;
        }
        for (int i=0; i<maxm; i++)
            client_buf[i]='\0';
    }
}




int send_filename(char filename[maxm], int sockfd)
{
    printf("Enter the filename : ");
    scanf("%s",filename);

    // Send the filename 
    send(sockfd, filename, strlen(filename)+1,0);
    printf("[.....SUCCESS.....] Name of required file(filename) send.\n");

    // Create the file at client side or open the file in it's already present.
    int fd = open("required_file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    return fd;
}




int main()
{
    int sockfd;
    struct sockaddr_in serv_addr;

    // Creating sockets
    if((sockfd=socket(AF_INET, SOCK_STREAM, 0))<0)
    {
        perror("[.....ERROR.....] Unable to create the socket.\n");
        exit(0);
    }

    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_port=htons(8181);

    // Making connection to server.
    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))<0)
    {
        perror("[.....ERROR.....] Unable to connect to server.\n");
        close(sockfd);
        exit(0);
    }
    printf("[.....SUCCESS.....] Connected to Server.\n");


    // Taking filename from user and sending it to server.
    char filename[maxm];
    for(int i=0; i<maxm; i+=1)
        filename[i]='\0';
    int fd=send_filename(filename, sockfd) ;

    // Unable to create the file at client side.
    if(fd<0)
    {
        perror("[.....ERROR.....] Unable to create a file.\n");
        close(sockfd);
        exit(1);
    }


    // Read the file data from the server
    int rec_byte=0, counter=0, bytes=0, words=0;
    char client_buf[maxm];
    for (int i=0; i<maxm; i++)
        client_buf[i]='\0';

    parse_file(client_buf, sockfd, fd, &rec_byte, &counter, &words, &bytes) ;
    
    // File doesn't exist at server side.
    if (counter==0 && rec_byte==0)
    {
        printf("[.....ERROR.....] File Not Found at Server.\n");
        remove("required_file.txt");
        close(sockfd);
        close(fd);
        exit(0);
    }

    // File received and parsed.
    printf("[.....SUCCESS.....] File received in %d calls.\n", counter);
    printf("Number of words: %d.\nNumber of bytes: %d.\n", words, bytes);
    close(sockfd);
    close(fd);
    return 0;
}