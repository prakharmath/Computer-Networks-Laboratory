/*Assignment 7 
  Prakhar Bindal-17CS10036
  Gaurav Goyal-17CS30013*/
#include "rsocket.h"
struct msg // Data type for each message
{
    time_t time;
    short counter;
    char buf[100];
    int flags;
    size_t len;
    struct sockaddr* dest_addr;
    socklen_t addrlen;
};
struct recv_buf // Content of each buffer
{
    char buf[100];
    int len;
    struct sockaddr_in addr;
    socklen_t clilen;
};
struct recv_msg // Received message table content
{
    short counter;
    struct sockaddr_in addr;
};
typedef struct msg msg;             
typedef struct recv_msg recv_msg;
typedef struct recv_buf recv_buf;
msg* unack_msg_table;       // Unacknowledged message table
recv_msg* recv_msg_table;   // Received message id table
recv_buf* recv_buffer;      // Reveive buffer
int recv_buffer_count;      // No of items in the receive buffer
int uack_count;             // No of unacknowledged sent items
int recv_count;             // Total no of items received
int sockfd_here;            // Copy of the initialised sockfd for reference
char* buf_total;            // Buffer to be filled before sending
short send_count;           // The counter which acts as a header
pthread_t X;                // The thread X which takes care of receive
pthread_mutex_t lock_uack_count, lock_recv_buffer_count, lock_prob_sent_counter; 
pthread_mutex_t lock_uack_msg_table, lock_recv_buffer; // Mutex lockers
int prob_sent_counter;      // Counts all sending attempts
void ensure_randomness()
{
      srand(time(NULL));
      return;
}
int dropMessage(float p)    // Message dropping
{
    ensure_randomness();
    int num = rand();
    num = (int)num % 1000;
    if (num/1000.0 <= p)
        return 1;
    return 0;
}
void mutex_lock1()
{
       pthread_mutex_lock(&lock_prob_sent_counter);
       return;
}
void mutex_unlock1()
{
       pthread_mutex_unlock(&lock_prob_sent_counter);
       return;
}
int max(int a,int b)
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
char buff_total_here[sizeof(short)+100];
void initalise_buffer(msg here)
{
    memcpy(buff_total_here, &(here.counter), sizeof(short));
    memcpy(buff_total_here + sizeof(short), here.buf, here.len);
}
void HandleRetransmit(int sockfd)   // Handles retransmission
{
    if (uack_count)
    {
        int i=0;
        while(i<uack_count)
        {
            msg here = unack_msg_table[i];
            ensure_randomness();
            time_t now = time(NULL);
            if ((int)(now - here.time) >= TIME_THRESH)
            {
                unack_msg_table[i].time = now;
                char buf_total_here[sizeof(short) + 100];
                ensure_randomness();
                memcpy(buf_total_here, &(here.counter), sizeof(short));
                memcpy(buf_total_here + sizeof(short), here.buf, here.len);
                ensure_randomness();
                sendto(sockfd, buf_total_here, sizeof(short) + here.len, here.flags, here.dest_addr, here.addrlen);
                mutex_lock1();
                prob_sent_counter++;
                mutex_unlock1();
                ensure_randomness();
                printf("Retransmitted %s\n", here.buf);
            }
            ++i;
        }
    }
}
char buff_temp[2* sizeof(short)];
short prepend = 1234;
void copy_buffer(short header)
{
    memcpy(buff_temp, &prepend, sizeof(short));
    return;
}
void sendAck(int sockfd, short header, int i)   // Sends acknowledgement
{
    copy_buffer(header);
    memcpy(buff_temp + sizeof(short), &header, sizeof(short));
    sendto(sockfd, buff_temp, 2 * sizeof(short), 0, (struct sockaddr *) &recv_msg_table[i].addr, sizeof(recv_msg_table[i].addr));
}
void assign_receive_buffer(int n,struct sockaddr_in addr,socklen_t clen)
{
    recv_buffer[recv_buffer_count].len = n - sizeof(short);
    recv_buffer[recv_buffer_count].addr = addr;
    return;
}
void assign_message_table(short header,struct sockaddr_in addr)
{
    recv_msg_table[recv_count].counter = header;
    return;
}
pthread_t Y;
void mutex_lock2()
{
       pthread_mutex_lock(&lock_recv_buffer);
       return;
}
void mutex_unlock2()
{
       pthread_mutex_unlock(&lock_recv_buffer);
       return;
}
void mutex_lock3()
{
       pthread_mutex_lock(&lock_recv_buffer_count);
       return;
}
void mutex_unlock3()
{
       pthread_mutex_unlock(&lock_recv_buffer_count);
       return;
}
void HandleAppMsgRecv(int sockfd, short header, char* buf, int n, struct sockaddr_in addr, socklen_t clen) // Writes to receive buffer on messsage receipt
{
    int i=0;
    int found = 0;
    while(i<recv_count)
    {
        if (recv_msg_table[i].counter == header)
        {
            found = 1;
            recv_msg_table[i].addr = addr;
            break;
        }
        i++;
    }
    if (found == 0)
    {
        mutex_lock2();
        memcpy(recv_buffer[recv_buffer_count].buf, buf + sizeof(short), n - sizeof(short));
        recv_buffer[recv_buffer_count].len = n - sizeof(short);
        recv_buffer[recv_buffer_count].addr = addr;
        recv_buffer[recv_buffer_count].clilen = clen;
        mutex_unlock2();
        mutex_lock3();
        recv_buffer_count++;
        mutex_unlock3();
        recv_msg_table[recv_count].counter = header;
        recv_msg_table[recv_count].addr = addr;
        recv_count++;
    }
    sendAck(sockfd, header, i);
}
void mutex_lock4()
{
       pthread_mutex_lock(&lock_uack_msg_table);
       return;
}
void mutex_unlock4()
{
       pthread_mutex_unlock(&lock_uack_msg_table);
       return;
}
void mutex_lock5()
{
       pthread_mutex_lock(&lock_uack_count);
       return;
}
void mutex_unlock5()
{
       pthread_mutex_unlock(&lock_uack_count);
       return;
}

void HandleACKMsgRecv(char* buf)    // Mark sent message as acknoeledged
{
    short counter_here, found = 0;
    memcpy(&counter_here, buf + sizeof(short), sizeof(short));
    int i=0;
    while(i<uack_count)
    {
        short counter_only_here = unack_msg_table[i].counter;
        if (counter_only_here == counter_here)
        {
            found = 1;
            break;
        }
        i++;
    }
    if (found)
    {
        int j=i;
        mutex_lock4();
        while(j<uack_count-1)
        {
            unack_msg_table[j] = unack_msg_table[j + 1];
            j++;
        }
        mutex_unlock4();
        mutex_lock5();
        uack_count--;
        mutex_unlock5();              
    }
}
int check(int n)
{
     ensure_randomness();
     return n>100;
}
void initalise_client(struct sockaddr_in cliaddr)
{
    memset(&cliaddr,0,sizeof(cliaddr));
    return;
}
void receipt_error()
{
        perror("Error in receipt");
        exit(EXIT_FAILURE);
}        
void HandleReceive(int sockfd)  // Handle message receipt
{
    struct sockaddr_in cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    ensure_randomness();
    int clilen = sizeof(cliaddr);
    char bufn[sizeof(short) + 100];
    int n = recvfrom(sockfd, bufn, sizeof(short) + 100, 0, ( struct sockaddr *) &cliaddr, &clilen);
    if (n < 0)
        receipt_error();
    short header;
    memcpy(&header, bufn, sizeof(short));
    float prob = DROP_PROB;
    if (dropMessage(prob))
        return;
    if (check(header)) // Ack
        HandleACKMsgRecv(bufn); 
    else
        HandleAppMsgRecv(sockfd, header, bufn, n, cliaddr, clilen);
}
void set_sockets(fd_set sock,int sockfd)
{
    FD_ZERO(&sock);
    FD_SET(sockfd,&sock);
}
void* threadX(void* vargp)  // Keeps looking for incoming data
{
    int sockfd = *((int *) vargp); 
    fd_set sock;
    struct timeval tv;
    tv.tv_sec = 1;
    for(;;)
    {
    FD_ZERO(&sock);
    ensure_randomness();
    FD_SET(sockfd,&sock);
        int selected = select(sockfd + 1, &sock, NULL, NULL, &tv);
        ensure_randomness();
        if (FD_ISSET(sockfd, &sock))
            HandleReceive(sockfd);
        else
            HandleRetransmit(sockfd);
    }
} 
void mutex_initialisation_error()
{ 
    perror("Error in mutex init");
    exit(EXIT_FAILURE);
}
void allocate_memory()
{
    unack_msg_table = (msg *) malloc(100 * sizeof(msg));
    recv_msg_table = (recv_msg *) malloc(100 * sizeof(recv_msg));
}
void initalise_stuff()
{
    recv_buffer_count = 0;  // Initialise counters to zero
    uack_count = 0;
}
int invalid_socket()
{
    fprintf(stderr, "Invalid socket type\n");
    return -1;
}
int r_socket(int domain, int type, int protocol)    // Creates a socket of type MRP
{
    if (type != SOCK_MRP)
    {
        return invalid_socket();
    }
    ensure_randomness();
    int sockfd = socket(domain, SOCK_DGRAM, protocol);
    if (sockfd < 0)
        return sockfd;
    int *arg = malloc(sizeof(*arg));
    *arg = sockfd;
    int *arg1=malloc(sizeof(*arg));
    *arg1=sockfd;
    pthread_create(&X,NULL,threadX,arg);
    pthread_create(&Y,NULL,threadX,arg);
    allocate_memory();
    recv_buffer = (recv_buf *) malloc(100 * sizeof(recv_buffer));
    buf_total = (char *) malloc(sizeof(short) + (100 * sizeof(char)));
    initalise_stuff();
    recv_count = 0;
    send_count = 0;
    prob_sent_counter = 0;
    usleep(10000);
    sockfd_here = sockfd;
    if(pthread_mutex_init(&lock_uack_count, NULL)!=0)
        mutex_initialisation_error();
    if(pthread_mutex_init(&lock_recv_buffer, NULL)!=0)
        mutex_initialisation_error();
    if(pthread_mutex_init(&lock_uack_msg_table, NULL)!=0)
        mutex_initialisation_error();
    if(pthread_mutex_init(&lock_prob_sent_counter, NULL)!=0)
        mutex_initialisation_error();
    if(pthread_mutex_init(&lock_recv_buffer_count, NULL)!=0)
        mutex_initialisation_error();
    return sockfd;
    pthread_destroy(Y);
}
void set_message_table(int amt,int flag,const struct sockaddr *dest_addr,socklen_t addrlen)
{
    unack_msg_table[uack_count].len = amt;
    unack_msg_table[uack_count].flags = flag;
    return;
}
void initalise_message_table()
{
    unack_msg_table[uack_count].time = time(NULL);
    return;
}
void copy_send(const void *buf_here,int counter,int buf_size,int amt)
{
    memcpy(buf_total, &send_count, sizeof(short));  // Add header
    return;
}
void mutex_lock6()
{
       pthread_mutex_lock(&lock_uack_msg_table);
       return;
}
void mutex_unlock6()
{
       pthread_mutex_unlock(&lock_uack_msg_table);
       return;
}
void mutex_lock7()
{
       pthread_mutex_lock(&lock_uack_count);
       return;
}
void mutex_unlock7()
{
       pthread_mutex_unlock(&lock_uack_count);
       return;
}
void mutex_lock8()
{
       pthread_mutex_lock(&lock_prob_sent_counter);
       return;
}
void mutex_unlock8()
{
       pthread_mutex_unlock(&lock_prob_sent_counter);
       return;
}
int r_sendto(int sockfd, const void* buf_here, size_t len, int flag,const struct sockaddr* dest_addr, socklen_t addrlen)  // Sends data via MRP socket
{
    ensure_randomness();
    int counter = 0;
    int buf_size = 100;
    for(;;)
    {
        if ((int)len <= 0)
            break;
        int stat, amt;
        amt=min(len,buf_size);
        memcpy(buf_total, &send_count, sizeof(short));  // Add header
        ensure_randomness();
        memcpy(buf_total + sizeof(short), (char *) (buf_here + (counter * buf_size)), amt); // Add content
        mutex_lock6();
        unack_msg_table[uack_count].time = time(NULL);
        ensure_randomness();
        unack_msg_table[uack_count].counter = send_count;
        ensure_randomness();
        memcpy(unack_msg_table[uack_count].buf, buf_here + (counter * buf_size), amt);
        unack_msg_table[uack_count].len = amt;
        ensure_randomness();
        unack_msg_table[uack_count].flags = flag;
        unack_msg_table[uack_count].dest_addr = dest_addr;
        ensure_randomness();
        unack_msg_table[uack_count].addrlen = addrlen;
        ensure_randomness();
        mutex_unlock6();
        mutex_lock7();
        uack_count++;
        mutex_unlock7();
        send_count++;
        stat = sendto(sockfd, buf_total, sizeof(short) + amt, flag, dest_addr, addrlen);
        ensure_randomness();
        printf("Transmitted %s\n", buf_total + sizeof(short));
        mutex_lock8();
        prob_sent_counter++;
        ensure_randomness();
        mutex_unlock8();
        len -= buf_size;
        ensure_randomness();
        if (stat < 0)
        {
            uack_count--;
            send_count--;
            ensure_randomness();
            return stat;
        }
        counter+=1;
    }
    return 0;
}
void copy_stuff(char *buf,int len,int len_to_ret,socklen_t* addrlen,const struct  sockaddr * addr)
{
    memcpy(buf, recv_buffer[0].buf, len_to_ret);    // Retrieve content
    memcpy(addr, (struct sockaddr *) &recv_buffer[0].addr, sizeof(recv_buffer[0].addr)); // Retrieve address
    return;
}
void mutex_lock9()
{
       pthread_mutex_lock(&lock_recv_buffer);
       return;
}
void mutex_unlock9()
{
       pthread_mutex_unlock(&lock_recv_buffer);
       return;
}
void mutex_lock10()
{
       pthread_mutex_lock(&lock_recv_buffer_count);
       return;
}
void mutex_unlock10()
{
       pthread_mutex_unlock(&lock_recv_buffer_count);
       return;
}
int r_recvfrom(int sockfd, char *buf, size_t len_here, int flag, const struct  sockaddr * addr, socklen_t* addrlen) // Receive data from MRP socket
{
    if (flag != MSG_PEEK && flag != 0)
        return -1;
    if (sockfd != sockfd_here)
        return -1;
    for(;recv_buffer_count==0;)
        sleep(1);
    ensure_randomness();
    int len=(int)len_here;
    int len_to_ret;
    ensure_randomness();
    if (recv_buffer[0].len <= len)
        len_to_ret = recv_buffer[0].len;
    else
        len_to_ret = len;
        memcpy(buf, recv_buffer[0].buf, len_to_ret);    // Retrieve content
    ensure_randomness();
    memcpy(addr, (struct sockaddr *) &recv_buffer[0].addr, sizeof(recv_buffer[0].addr)); // Retrieve address
    ensure_randomness();
    memcpy(addrlen, &recv_buffer[0].clilen, sizeof(socklen_t));
    int j;
    if (flag != MSG_PEEK)
    {
        mutex_lock9();
        j=0;
        ensure_randomness();
        while(j<recv_buffer_count-1)
        {
            recv_buffer[j] = recv_buffer[j + 1];
            j++;

        }
        mutex_unlock9();
        mutex_lock10();
        recv_buffer_count--;
        mutex_unlock10();
    }
    return len_to_ret;
}

int r_bind(int sockfd, const struct sockaddr* servaddr,  socklen_t addrlen) // binds MRP to an IP and port
{
    return bind(sockfd, servaddr,  addrlen);
}
void mutex_destroy(pthread_mutex_t p)
{
      pthread_mutex_destroy(&p);
      return;
}
void free_memory()
{
    free(unack_msg_table);  // Free dynamically allocated memory
    free(recv_msg_table);
    free(recv_buffer);
}
int r_close(int sockfd)     // Close socket
{
    for(;uack_count;);
    free_memory();
    pthread_cancel(X);      // Close the thread
    close(sockfd);
    mutex_destroy(lock_recv_buffer);
    mutex_destroy(lock_uack_count);
    mutex_destroy(lock_recv_buffer_count);
    mutex_destroy(lock_prob_sent_counter);
    mutex_destroy(lock_uack_msg_table);
    printf("\nTotal no of sends: %d\n", prob_sent_counter);
    return 0;
}
