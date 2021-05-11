#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
const int MAXN=1024;
#include<bits/stdc++.h>
using namespace std;
signed main()
{ 
    int sockfd; 
    struct sockaddr_in servaddr, cliaddr; 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 )
    { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(8181); 
    if (bind(sockfd,(const struct sockaddr *)&servaddr,sizeof(servaddr))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    printf("\nServer Running....\n");
    int n; 
    socklen_t len;
    char file_name[MAXN]; 
    char temp[MAXN];
    len = sizeof(cliaddr);
    n = recvfrom(sockfd, (char *)file_name,MAXN,0,( struct sockaddr *)&cliaddr,&len); 
    file_name[n] = '\0';
    file_name[n+1]='\0';
    FILE *F=fopen(file_name,"r");
    char *end="END\n";
    if(F==NULL)
    {
        sprintf(temp,"%s","NOTFOUND ");
        sprintf(&temp[9],"%s",file_name);
        sendto(sockfd, (const char *)temp, strlen(temp),0,(const struct sockaddr *) &cliaddr, sizeof(cliaddr));
        return 0;
    } 
    else
    {
        fgets(file_name,MAXN,F);
        sendto(sockfd,(const char *)file_name, strlen(file_name),0,(const struct sockaddr *) &cliaddr, sizeof(cliaddr));
        while(1)
        {
            n = recvfrom(sockfd, (char *)file_name, MAXN, 0,( struct sockaddr *) &cliaddr, &len);
            file_name[n] = '\0';
            printf("Sending %s\n",file_name);
            fgets(file_name,MAXN,F);
            sendto(sockfd, (const char *)file_name, strlen(file_name),0,(const struct sockaddr *) &cliaddr, sizeof(cliaddr));
            if(strcmp(file_name,end)==0) break;
        }
        fgets(file_name,MAXN,F);
        fclose(F);
    }
    return 0; 
} 
