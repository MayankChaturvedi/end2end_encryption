#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>

#include<string.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <sys/types.h>
#include <sys/wait.h>

#define MAX 4098

void server_f(int me, int friend){
    printf("my client id is %d\n",me);
    unsigned char buff[MAX];
    // infinite loop for chat
    for (;;) {
        bzero(buff, MAX);
  
        // read the message from client and copy it in buffer
        int ret = read(me, buff, sizeof(buff));
        // print buffer which contains the client contents
        printf("Sending message from id %d to id %d\n", me,friend);
        // if msg contains "Exit" then server exit and chat ended.
        if (ret==0) {
            printf("Client id %d disconnected...\n",me);
            ret = send(friend,"exit",4,0);
            exit(0);
        }
        else{
            ret = send(friend,buff,ret,0);//previous ret length used 
            if(ret == -1){
                exit(0);
            } 
        }
    }
    exit(0);
}
int main(int argc, char const *argv[])
{
    if(argc<2){
        printf("please input the port number to bind on\n");
        exit(0);
    }
    int PORT = atoi(argv[1]);
    int server_fd, new_socket1, new_socket2, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
       
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
       
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
       
    
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    pid_t pid[2],wpid;
    while(1){
        printf(">>Waiting for a new pair of friends to join\n");
        
        while ((new_socket1 = accept(server_fd, (struct sockaddr *)&address, 
                       (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("first friend joined\n");

        //Do we need another listen here?
        
        while ((new_socket2 = accept(server_fd, (struct sockaddr *)&address, 
                        (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("second friend joined\n");


        if((pid[0]=fork())==0){
            printf("child 1 %d\n",pid[0]);
            server_f(new_socket1,new_socket2);
        }
        else{
            if((pid[1]=fork())==0){
                printf("child 2 %d\n",pid[1]);
                server_f(new_socket2,new_socket1);
            }
        }
        
        int status=0;
        while((wpid=wait(&status))>0);
        printf("Both the friends disconnected successfully\n");
        close(new_socket1);
        close(new_socket2);
    }
    
    return 0;
}