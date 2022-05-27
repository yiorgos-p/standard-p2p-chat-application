#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>

#define MAX 258
#define PORT 8080
#define DNSPORT 8010
#define DNSIP "127.0.0.1"
#define SA struct sockaddr

int clientsocket;
int serversocket;
int serverconn;
bool isServer;

char* encode_packet(char type, char text[]) {
	int len=strlen(text);
	//max M text length
	if(len>256) len=256;
	char *payload=malloc(len+2);
	payload[0]=type;
	payload[1]=(char)len;
	for(int i=0; i<len; i++){
		payload[i+2]=text[i];
	}
	return payload;
}

void sigintHandler(int sig_num){
	if(isServer){
		char *payload=encode_packet('Q',"");
   	 	write(serverconn, payload, strlen(payload));
		close(serversocket);
		printf("\nDisconnected.\n");
	}
	else {
    	char *payload=encode_packet('Q',"");
    	write(clientsocket, payload, strlen(payload));
		printf("\nDisconnected.\n");
	}
	exit(1);
}

void msleep(int ms){
	usleep(ms*1000);
}

void auto_refresh(int socket_id){
	char payload[MAX];
	for (;;){
		bzero(payload, MAX);
		read(socket_id, payload, MAX);
		if(strlen(payload)!=0){
			if(payload[0]=='Q'){
				sigintHandler(1);
			}
			else{
				int len=(int)payload[1];
				char msg[len];
				bzero(msg,len);
				memmove(msg, payload+2, strlen(payload));
				printf("User: %s\n",msg);
			}
		}
		msleep(1000);
	}
}

void send_packet(int socket_id, char string[]){
	char type='M';
	if(!strcmp(string,"/quit")) type='Q';
	char *payload=encode_packet(type,string);
	write(socket_id, payload, strlen(payload));
	if(type=='Q') sigintHandler(0);
}

void read_input(int socket_id){
	for(;;){
		char inputarr[MAX-2];
		scanf("%s",&inputarr);
		printf("\x1b[1F"); // Move to beginning of previous line
		printf("\x1b[2K"); // Clear entire line
		printf("You: %s\n",&inputarr);
		send_packet(socket_id,inputarr);
	}
}

void connect_ipv6(char addr6[], int port){
    int sockfd, connfd;
    char response[1];
	struct sockaddr_in6 servaddr, cli;

	// socket create and verification
	sockfd = socket(AF_INET6, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("Socket creation failed...\n");
		exit(0);
	}
	else {
		printf("Socket successfully created.\n");
	}
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin6_family = AF_INET6;
	inet_pton(AF_INET6, addr6, &servaddr.sin6_addr);
	servaddr.sin6_port = htons(port);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("Connection with the server failed...\n");
		exit(0);
	}
	else {
		printf("Connected to the server.\n");
	}
	clientsocket=sockfd;
	pthread_t thread_id;
    pthread_create(&thread_id, NULL, auto_refresh, sockfd);
	read_input(sockfd);

	//close the socket
	close(sockfd);
}

void connect_ipv4(char addr4[], int port){
	int sockfd, connfd;
    char response[1];
	struct sockaddr_in servaddr, cli;

	//socket creation and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("Socket creation failed...\n");
		exit(0);
	}
	else {
		printf("Socket successfully created.\n");
	}
	bzero(&servaddr, sizeof(servaddr));

	//assign IP, PORT
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, addr4, &servaddr.sin_addr);
	servaddr.sin_port = htons(port);

	//connect client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("Connection with the server failed...\n");
		exit(0);
	}
	else {
		printf("Connected to the server.\n");
	}
	
	clientsocket=sockfd;
	pthread_t thread_id;
    pthread_create(&thread_id, NULL, auto_refresh, sockfd);
	read_input(sockfd);

	//close the socket
	close(sockfd);
}

void validate_addr(char string[], int port){

	struct in6_addr addr6;
	struct in_addr addr4;

	if (inet_pton(AF_INET, string, &addr4)==1)
	{
		printf("IPv4 address is valid\n");
		connect_ipv4(string, port);
	}
	else if (inet_pton(AF_INET6, string, &addr6)==1)
	{
		printf("IPv6 address is valid\n");
		connect_ipv6(string, port);
	}
	else {
		printf("invalid address\n");
		exit(0);
	}
}

void client(){
	isServer=false;
	char response[MAX-2];
    printf("Input IPv4 or IPv6 address: ");
    scanf("%s",&response);
	validate_addr(response, PORT);
}

void server_ipv6(){
	int sockfd, connfd, len;
	struct sockaddr_in6 servaddr, cli;
	//socket creation and verification
	sockfd = socket(AF_INET6, SOCK_STREAM, 0);
	serversocket = sockfd;
	if (sockfd == -1) {
		printf("Socket creation failed...\n");
		exit(0);
	}
	else {
		printf("Socket successfully created..\n");
	}
	bzero(&servaddr, sizeof(servaddr));
	//assign IP, PORT
	servaddr.sin6_family = AF_INET6;
	servaddr.sin6_addr = in6addr_any;
	servaddr.sin6_port = htons(PORT);
	//binding newly created socket to given address and verification
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("Socket bind failed...\n");
		exit(0);
	}
	else {
		printf("Socket successfully binded..\n");
	}
	//server is ready to listen
	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else {
		printf("Server listening..\n");
	}
	len = sizeof(cli);
	//accept data from client
	connfd = accept(sockfd, (SA*)&cli, &len);
	serverconn=connfd;
	if (connfd < 0) {
		printf("Server accept failed...\n");
		exit(0);
	}
	else {
		printf("Server accepted the client...\n");
	}
	pthread_t thread_id;
    pthread_create(&thread_id, NULL, auto_refresh, connfd);
	read_input(connfd);
}

void server_ipv4(){
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;
	//socket creation and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	serversocket=sockfd;
	if (sockfd == -1) {
		printf("Socket creation failed...\n");
		exit(0);
	}
	else {
		printf("Socket successfully created..\n");
	}
	bzero(&servaddr, sizeof(servaddr));
	//assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);
	//binding newly created socket to given address and verification
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("Socket bind failed...\n");
		exit(0);
	}
	else {
		printf("Socket successfully binded..\n");
	}
	//server is ready to listen
	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else {
		printf("Server listening..\n");
	}
	len = sizeof(cli);
	//accept data from client
	connfd = accept(sockfd, (SA*)&cli, &len);
	serverconn=connfd;
	if (connfd < 0) {
		printf("Server accept failed...\n");
		exit(0);
	}
	else {
		printf("Server accepted the client...\n");
	}
	pthread_t thread_id;
    pthread_create(&thread_id, NULL, auto_refresh, connfd);
	read_input(connfd);
}

void server(){
	isServer=true;
	printf("Type 1 for IPv4, type 2 for IPv6: ");
	int answer;
	scanf("%d",&answer);
	if(answer==1)
	{
		server_ipv4();
	}
	else if(answer==2)
	{
		server_ipv6();
	}
	else{
		printf("Incorrect answer\n");
		exit(0);
	}
}

int main()
{
	printf("Type 1 for hosting, 2 for connecting, 3 for NS service: ");
	int answer;
	scanf("%d",&answer);
	signal(SIGINT, sigintHandler);
	if (answer==1)
	{
		server();
	}
	else if (answer==2)
	{
		client();
	}
	else if (answer==3){
		connect_ipv4(DNSIP,DNSPORT);
	}
	else {
		printf("Incorrect answer\n");
		exit(0);
	}	
}