#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
//#include <process.h>
#include <limits.h>
#define maxm 100




int send_subdirectory_name(char subdirectory_name[maxm], int sockfd)
{
    printf("Enter the Subdirectory name : ");
    scanf("%s", subdirectory_name);

    // Send the subdirectory_name 
    send(sockfd, subdirectory_name, strlen(subdirectory_name)+1,0);
    printf("[.....SUCCESS.....] Name of required file(subdirectory_name) send.\n");

    // Create the directory at client side or open the directory in it's already present.
    int check = mkdir(subdirectory_name, 0777);
    return check;
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


    
    //while(1)
    //{
        int count=0, abracadabra=0;

        /*char s[4];
        printf("Do you have any query??\nInput: YES or NO\n");
        scanf("%s", s);

        if(strcmp(s, "NO")==0)
            break;*/
        
        char subdirectory_name[maxm];
        for(int i=0; i<maxm; i+=1)
            subdirectory_name[i]='\0';
        int check=send_subdirectory_name(subdirectory_name, sockfd) ;
    
        
        // Unable to create the subdirectory at client side.
        if(check<0)
        {   
            DIR *dir ;
            dir=opendir(subdirectory_name);

            if(dir==NULL)
                perror("[.....ERROR.....] Unable to create or access directory at client side to receive images.\n");
        }
        
        printf("[.....SUCCESS.....] Subdirectory created at client side.\n");
        
        while(1)
        {
            //if(abracadabra>1)
              //  break;

            char name[maxm];
            check=recv(sockfd, name, maxm, 0);
            //printf("%d\n",check);
            if(check<=1)
            {
                //write(sockfd, "\0", 1);
                break;
            }
            else
            {
                count+=1;
                printf("[.....Receiving_Picture_in_Process.....] Picture received:%s\n", name);

                char name1 [maxm];
                memset(name1, '\0', sizeof(name1));
                int flag=0, j=0;
                for(int i=0; i<maxm; i+=1)
                {
                    if(flag==0)
                    {
                        if(name[i]=='/')
                            flag=1;
                    }
                    else
                    {
                        name1[j]=name[i];
                        j++;
                    }
                }

                //printf("[.....Receiving_Picture_in_Process.....] Picture received: %s.\n", name1);

                //sleep(1);

                /*printf("[.....Receiving_Picture_in_Process.....] Reading Picture Size send by server.\n");
                int size;
                read(sockfd, &size, sizeof(int));

                //Read Picture Byte Array
                printf("[.....Receiving_Picture_in_Process.....] Reading Picture as Byte Array over socket.\n");*/
                
                char p_array[maxm];
                int nb = read(sockfd, p_array, maxm);
                //printf("%d\n", nb);
                if(nb==1)
                {
                    printf("\n");
                    //abracadabra+=1;
                    continue; 
                }

                FILE *image = fopen(name1, "wb");

                while (nb >= 0) 
                {
                    fwrite(p_array, 1, nb, image);
                    nb = read(sockfd, p_array, maxm);
                    //printf("%d\n", nb);
                    if(nb==1 || nb==0)
                        break;
                }
                printf("[.....Receiving_Picture_in_Process.....] Picture totally received and saved: %s\n\n", name1);
            }
        }
        if(count>=2)
            count-=2;
        else
            count=0;
        printf("[.....SUCCESS.....] Number of Pictures received from server: %d.\n", count);
    //}
    close(sockfd);
    exit(0);
}