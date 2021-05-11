#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h> 
#include <netdb.h> 

#define MAX_CHAR 100
#define MAX_LINE 1024
int main()
{
    int sockfd;
    struct sockaddr_in serv_addr;
    char hostname[MAX_CHAR], buffer[MAX_CHAR];
    int n;
    socklen_t len;

    if((sockfd = socket(AF_INET, SOCK_DGRAM,0))<0)
    {
        perror("[.....ERROR.....] Unable to create the socket\n");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(8181);

    //while(1)
    //{
        /*char s[4];
        printf("Do you have any query??\nEnter Yes if you have a query else Enter No\n");
        scanf("%s", s);

        if(strcmp(s, "NO")==0)
        {
            //sendto(sockfd, "\0", 1, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            break;
        }*/

        printf("Enter the hostname: ");
        scanf("%s", hostname);

        sendto(sockfd, (const char*)hostname, strlen(hostname)+1, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

        len = sizeof(serv_addr);
        n = recvfrom(sockfd, (char *)buffer, MAX_LINE, 0, (struct sockaddr *)&serv_addr, &len);
        buffer[n] = '\0';

        printf("[.....SUCCESS.....] IP addres for hostname %s : %s\n", hostname, buffer);
    //}
    return 0;
}