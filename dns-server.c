#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
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
struct client{
    char name[128];
    char ip[128];
};
struct client users[256];

int count;

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

char* search(char name[]){
	char *ip=malloc(128);
    ip="No user found";
    for(int i=0; i<count; i++){
        if(!strcmp(users[i].name,name)){ 
            printf("IP of user is:%s\n",users[i].ip);
            ip=users[i].ip;
        }
    }
    name[strcspn(name, "\n")] = 0;
    return ip;
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
                char *name, *name2;
				bzero(msg,len);
				memmove(msg, payload+2, strlen(payload));
				name=search(msg);
                printf("'%s'",name);
                name2=encode_packet('M',name);
                send_packet(socket_id,name);
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
	servaddr.sin_port = htons(DNSPORT);
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
	auto_refresh(connfd);
}

void database(){
    char const* const fileName = "database.txt";

    FILE* file = fopen(fileName, "r"); 

    if(!file){
        printf("\n Unable to open : %s ", fileName);
		exit(0);
    }

    char line[128];
    char address;
    int n=0;
    while (fgets(line, sizeof(line), file)) {
        char *token;
        char *search = "="; 
        strcpy(users[n].name,strtok(line, search));
        strcpy(users[n].ip,strtok(NULL, search));
        n+=1;
    }
    count=n;
    fclose(file);
}

int main()
{
    isServer=true;
	signal(SIGINT, sigintHandler);
	database();
    server_ipv4();
}