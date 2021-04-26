# end2end_encryption

Application of RSA Encryption in the messages between a pair of clients

Server transfers message from one friend to another, without decrypting the message

To start the server:-
The source code is inside the folder "server"
>>gcc serverfork.c -o serverfork
>>./serverfork 4242

We bind the socket at the port 4242

To start the client 1 :-
The source code is inside the folder "client1"
>>gcc client.c -o client -pthread -lcrypto
>>./client 4242 127.0.0.1 private.pem public.pem
4242 is the port of server, replace 127.0.0.1 with the IP address of server
private.pem is the private key of Encryption 1 (private to client 1)
public.pem is the public key of Encryption 2 (public of client 2)

To start the client 2 :-
The source code is inside the folder "client2"
>>gcc client.c -o client -pthread -lcrypto
>>./client 4242 127.0.0.1 private.pem public.pem
4242 is the port of server, replace 127.0.0.1 with the IP address of server
private.pem is the private key of Encryption 2 (private to client 2)
public.pem is the public key of Encryption 1 (public of client 1)

Now, the clients are ready to communicate with each other
