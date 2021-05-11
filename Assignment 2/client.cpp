#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include<bits/stdc++.h>
const int MAXN=1024;
using namespace std;
signed main() 
{ 
    int sockfd; 
    struct sockaddr_in servaddr; 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) 
    { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(8181); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    int n;
    socklen_t len; 
    char file_name[MAXN];
    printf("Please Enter the file you want to open in the server: ");
    scanf("%s",file_name);
    sendto(sockfd, (const char *)file_name, strlen(file_name), 0,(const struct sockaddr *) &servaddr, sizeof(servaddr)); 
    printf("Your entered file name has been sent to the server\n");   
    n = recvfrom(sockfd, (char *)file_name, MAXN, 0,(struct sockaddr *) &servaddr, &len);
    file_name[n] = '\0';
    printf("%s",file_name);
    if(file_name[0]=='H')
    {
        FILE *f=fopen("recieved.txt","w");
        int num=1;
        char* terminate="END\n";
        len=sizeof(servaddr);
        while(1)
        {
            file_name[0]='W';
            file_name[1]='O';
            file_name[2]='R';
            file_name[3]='D';
            sprintf(&file_name[4], "%d", num);
            num++;
            printf("Recieving %s\n",file_name);
            sendto(sockfd, (const char *)file_name,strlen(file_name),0,(const struct sockaddr *) &servaddr, sizeof(servaddr));
            n = recvfrom(sockfd, (char *)file_name,MAXN, 0,( struct sockaddr *) &servaddr, &len);
            file_name[n] = '\0'; 
            if(strcmp(file_name,terminate)==0) break;
            fputs(file_name,f);    
        }
        printf("%s\n",file_name);
        fclose(f); 
    }
    close(sockfd); 
    return 0; 
} 
