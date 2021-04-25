#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include<pthread.h>




void * socketThread(void *arg)
{
  int *newSocket = ((int *)arg);
  int me = newSocket[0];
  int friend = newSocket[1];
  
  while(1){
      char my_message[300]={0};
      int ret = recv(me , my_message , 300 , 0);
      if(ret==0 || strcmp(my_message,"exit")==0){
          break;
      }
      else{
          send(friend,my_message,sizeof(my_message),0);
      }
  }
  

  
  pthread_exit(NULL);
}

int main(){
    int serverSocket, newSocket1, newSocket2;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;

    //Create the socket. 
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);

    // Configure settings of the server address struct
    // Address family = Internet 
    serverAddr.sin_family = AF_INET;

    //Set port number, using htons function to use proper byte order 
    serverAddr.sin_port = htons(7799);

    //Set IP address to localhost 
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");


    //Set all bits of the padding field to 0 
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    //Bind the address struct to the socket 
    bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    //Listen on the socket, with 40 max connection requests queued 
    if(listen(serverSocket,50)==0)
        printf("Listening\n");
    else
        printf("Error\n");
    pthread_t tid[2];
    while(1)
    {
        int i=0;

        addr_size = sizeof serverStorage;
        printf("Waiting for the first client to join");

        //Accept call creates a new socket for the incoming connection
        newSocket1 = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);

        printf("First client joined, waiting for the second client to join");

        //Accept call creates a new socket for the incoming connection
        newSocket2 = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);

        printf("Both the client joined, communication channel opens");
        
        //for each client request creates a thread and assign the client request to it to process
        //so the main thread can entertain next request
        
        int sockets[2]={newSocket1, newSocket2};
        if( pthread_create(&tid[i++], NULL, socketThread, sockets) != 0 )
            printf("Failed to create thread 1\n");
        sockets[0]=newSocket2;sockets[1]=newSocket1;
        if( pthread_create(&tid[i++], NULL, socketThread, sockets) != 0 )
            printf("Failed to create thread 2\n");
        

        pthread_join(tid[0],NULL);
        pthread_join(tid[1],NULL);
        printf("Both client pair disconnected, waiting for new pair to come");
    }
    return 0;
}