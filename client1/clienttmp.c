#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX 300
void * writer_f(void *arg)
{
    char buffer[300];
    int sock = *((int *)arg);
    while(1){
        bzero(buffer, 300);
        scanf("%s",buffer);
        printf("Scanned something %s\n",buffer);
        int valread = send( sock , buffer, 300, 0);
        if(valread==-1 || strncmp("exit", buffer, 4) == 0){
            exit(0);
        }
    }
}
void * reader_f(void *arg)
{
    printf("inside reader\n");
    char buff[300];
    int newSocket = *((int *)arg);
    for (;;) {
        printf("in here\n");
        bzero(buff, MAX);
  
        // read the message from client and copy it in buffer
        int retval = read(newSocket, buff, sizeof(buff));
        // print buffer which contains the client contents
        printf("From client: %s %d %d\n", buff,(int)strlen(buff),strcmp("exit",buff));
        // if msg contains "Exit" then server exit and chat ended.
        if (retval==0 || strncmp("exit", buff, 4) == 0) {
            printf("Client Exit...\n");
            exit(0);
        }
    }
}
int main(int argc, char const *argv[])
{
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[300] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
   
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(7799);
       
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    printf("client ready\n");
    pthread_t tid[2];

    //Reader thread
    printf("Creating reader thread\n");
    if( pthread_create(&tid[0], NULL, reader_f, &sock) != 0 )
        printf("Failed to create reader thread \n");
    printf("Creating writer thread\n");
    if( pthread_create(&tid[1],NULL,writer_f,&sock) != 0)
        printf("Failed to create writer thread \n");
    
    pthread_join(tid[0],NULL);
    pthread_join(tid[1],NULL);
    return 0;
}