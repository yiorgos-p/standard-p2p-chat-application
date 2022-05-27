# Standard P2P Chat Application in C
A basic Peer-To-Peer chat application written in C for use in a Linux environment. 
This application was created for a university assignment on networking, sockets etc.
## How it works
Once it's compiled, open the chat-client appliaction through a Linux terminal, and set it to host.
In a different terminal tab/window open the application and set it to client, after that it's pretty self-explanatory.

About the DNS server: This separate application reads a list of names and addresses from the database.txt file, if a client connects to the DNS server
they can lookup the address of a user by sending the name of the user they're trying to find, and the DNS replies back with the address of said user.
Adding names to the server on the database.txt file should be done in the same format as the examples written in the file.

This project is designed to work on a LAN.

## How to compile it
client:
```sh
gcc chat-client.c -o "chat-client" -pthread
```
dns server:
```sh
gcc dns-server.c -o "dns-server" -pthread
```
