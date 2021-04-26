#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>

#define MAX 4098

int padding = RSA_PKCS1_PADDING;
RSA * createRSAWithFilename(char * filename,int public)
{
    FILE * fp = fopen(filename,"rb");
 
    if(fp == NULL)
    {
        printf("Unable to open file %s \n",filename);
        return NULL;    
    }
    RSA *rsa= RSA_new() ;
 
    if(public)
    {
        rsa = PEM_read_RSA_PUBKEY(fp, &rsa,NULL, NULL);
    }
    else
    {
        rsa = PEM_read_RSAPrivateKey(fp, &rsa,NULL, NULL);
    }
 
    return rsa;
}
 
int private_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted)
{
	printf("Creating rsa\n");
    RSA * rsa = createRSAWithFilename(key,0);//key = my private key
    printf("Created rsa\n");
    int result = RSA_private_encrypt(data_len,data,encrypted,rsa,padding);
    printf("Encrypted message\n");
    return result;
}

int public_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted)
{
    RSA * rsa = createRSAWithFilename(key,1);//key = sender's public key
    int  result = RSA_public_decrypt(data_len,enc_data,decrypted,rsa,padding);
    return result;
}

void printLastError(char *msg)
{
    char * err = malloc(130);;
    ERR_load_crypto_strings();
    ERR_error_string(ERR_get_error(), err);
    printf("%s ERROR: %s\n",msg, err);
    free(err);
}

void * writer_f(void *arg)
{
    char buffer[MAX];
    int sock = *((int *)arg);
    while(1){
        bzero(buffer, MAX);
        char inp[256]={0};
        scanf("%s",inp);
        printf("scanned thing : %s\n",inp);
        int encrlen = private_encrypt(inp,strlen(inp),"private.pem",buffer);
        printf("encrypted thing : %s ->>%d<<- %d\n",buffer,encrlen,strlen(buffer));
        int valread = send( sock , buffer, encrlen, 0);
        if(valread==-1 || strncmp("exit", inp, 4) == 0){
            exit(0);
        }
        printf("Your message was sent\n");
    }
}
void * reader_f(void *arg)
{
    char buff[MAX];
    char decr[256];
    int newSocket = *((int *)arg);
    for (;;) {
        bzero(buff, MAX);
        bzero(decr,256);
        // read the message from client and copy it in buffer
        int retval = read(newSocket, buff, sizeof(buff));
        // print buffer which contains the client contents
        printf("From client: %s\n", buff);
        // if msg contains "Exit" then server exit and chat ended.
        if (retval==0 || strncmp("exit", buff, 4) == 0) {
            printf("Client Exit...\n");
            exit(0);
        }
        int decrlen = public_decrypt(buff,retval,"public.pem", decr);
        printf("Decrypted message : %s\n",decr);
        if (retval==0 || strncmp("exit", decr, 4) == 0) {
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
    char buffer[MAX] = {0};
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
   
    printf("Trying to connect with server\n");
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