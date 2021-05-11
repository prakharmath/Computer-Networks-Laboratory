#include "rsocket.h"

int main() 
{ 
    int sockfd; 
    struct sockaddr_in servaddr, ownconfig; 
    sockfd = r_socket(AF_INET, SOCK_MRP, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&ownconfig, 0, sizeof(ownconfig)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(50000 + (2 * 10036) + 1); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    ownconfig.sin_family    = AF_INET; 
    ownconfig.sin_addr.s_addr = INADDR_ANY; 
    ownconfig.sin_port = htons(50000 + (2 * 10036)); 
    if (r_bind(sockfd, (const struct sockaddr *)&ownconfig,  sizeof(ownconfig)) < 0)
    {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }
    printf("Binding complete\n");
    int n;
    socklen_t len;
    char buf[] = "A";
    int i;
    for (i = 0; i < strlen(buf); i++)
    {
        char buf_here[2];
        sprintf(buf_here, "%c", buf[i]);
        buf_here[1] = '\0';
        if (r_sendto(sockfd, (const char *)buf_here, 2, 0,
                (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        {
            perror("Error in sending");
            exit(EXIT_FAILURE);
        }
    }
    int clilen = sizeof(servaddr);
    r_recvfrom(sockfd, buf, 2, 0, (const struct sockaddr *) &servaddr, &servaddr);
    printf("Just now received: %s\n", buf);
    r_close(sockfd); 
    return 0; 
} 