/*
 * socket_server.c
 *
 * Created on: April 2, 2018
 * Author: Layla Lai
 *
 */

#include <stdio.h>
#include <sys/socket.h>     // func: socket;
#include <stdlib.h>         // func: exit;
#include <errno.h>          // func: errno;
#include <string.h>         // func: memset;
#include <netinet/in.h>     // func: htonl;
#include <pthread.h>        // func: pthread;
#include <arpa/inet.h>      // func: inet_ntoa;
#include <unistd.h>         // func: close;

#define SOCK_PORT 5330
#define MAX_CONN_LIMIT 512  // max connection limit
#define BUFFER_LENGTH 1024
#define ADDR_LENGTH 40

pthread_t thread_id[MAX_CONN_LIMIT];    // array of pthread
int allfd[MAX_CONN_LIMIT];
char* alladdr[MAX_CONN_LIMIT];          // array of address
int allport[MAX_CONN_LIMIT];            // array of port
int isConnect[MAX_CONN_LIMIT];
int id = 0;

void communicate(void* newfd_ptr){
    int newfd = *((int*)newfd_ptr);
    int maxfd;
    struct timeval tv;
    fd_set rfds;
    int retval;
    char data_send[BUFFER_LENGTH];  // data send
    char data_recv[BUFFER_LENGTH];  // data receive
    char data_temp[BUFFER_LENGTH];
    int sendlen, recvlen;
    const char* hello = "hello from server!\n"; // please input:\n time: to get local time;\n name: to get hostname;\n list: to list all clients' information;\n send:(id),(message): to communicat with others;\n";
	int nowid;
    int totalsec = 0;
    int hour = 0, min = 0, sec = 0;
    char localtime[10];

    char hostname[32];

    int i;
    int j;
    int toID;
    char toMessage[BUFFER_LENGTH];
	nowid = id - 1;
    allfd[id] = newfd;
    // say hello to client
    if( send(newfd, hello, strlen(hello), 0) > 0){
        printf("data send success!\n");
    }
    else{
        perror("data send: Server has NOT sent your message!\n");
        exit(errno);
    }

    while(1){
        // set rfds
        FD_ZERO(&rfds);         // initialize rfds
 //       FD_SET(0, &rfds);       // add 0 to set
        maxfd = 0;              // set 0 as maxfd
        FD_SET(newfd, &rfds);   // add sockfd to set
        if(maxfd < newfd){      // set maxfd
            maxfd = newfd;
        }

        // timeout
        tv.tv_sec = 3;  // second
        tv.tv_usec = 0;

        // wait
        retval = select(maxfd+1, &rfds,NULL, NULL, &tv);
        if(retval == -1){
            perror("select error and client quit!\n");
            break;
        }
        else if(retval == 0){
            continue;
        }
        else{
            // server send message
            if(FD_ISSET(0, &rfds)){
                bzero(data_send, BUFFER_LENGTH);        // initialize
                fgets(data_send, BUFFER_LENGTH, stdin); // input

                if(!strncasecmp(data_send, "quit", 4)){
                    printf("server require to quit!\n");
                    break;
                }
                sendlen = send(newfd, data_send, strlen(data_send), 0);
                if( sendlen > 0){
                    printf("data send success!\n");
                }
                else{
                    perror("[data send]: Server has NOT sent your message!\n");
                    break;
                }
            }

            // receive message from client
            if(FD_ISSET(newfd, &rfds)){
                bzero(data_recv, BUFFER_LENGTH);
                recvlen = recv(newfd, data_recv, BUFFER_LENGTH, 0);
                if( recvlen > 0){
                    printf("[data receive]: %s\n", data_recv);
                    // time()
                    if(!strncmp(data_recv, "time", 4)){
                        totalsec = (int)time(0);
                        sec = totalsec % 60;
                        min = totalsec % 3600 / 60;
                        hour = (totalsec % (24 * 60) / 3600 + 8) % 24;
                        sprintf(localtime, "%02d:%02d:%02d\n", hour, min, sec);
                        // send time to client
                        if( send(newfd, localtime, strlen(localtime), 0) > 0){
                            printf("data send success!\n");
                        }
                        else{
                            perror("[data send]: Server has NOT sent your message!\n");
                            exit(errno);
                        }
                    }
                    else if(!strncmp(data_recv, "name", 4)){
                        gethostname(hostname, sizeof(hostname));
                        sprintf(hostname, "%s\n", hostname);
                        // send time to client
                        if( send(newfd, hostname, strlen(hostname), 0) > 0){
                            printf("data send success!\n");
                        }
                        else{
                            perror("[data send]: Server has NOT sent your message!\n");
                            exit(errno);
                        }
                    }
                    if(!strncmp(data_recv, "list", 4)){
                        for(i = 0; i < id; i++){
                            bzero(data_send, BUFFER_LENGTH);        // initialize
                            if(isConnect[i] == 1){
                                sprintf(data_send, "id: %d, addr: %s, port: %d\n", i+1, alladdr[i], allport[i]);
                                if( send(newfd, data_send, strlen(data_send), 0) > 0){
                                    printf("data send success!\n");
                                }
                                else{
                                    perror("[data send]: Server has NOT sent your message!\n");
                                    exit(errno);
                                }
                            }
                        }
                    }
					
                	if(!strncasecmp(data_recv, "quit", 4)){
                    	printf("A socket closed\n");
                   	 	break;
                	}
                    // client want to send: "send:58,message"
                    if(!strncmp(data_recv, "send", 4)){
                        // id of the receiver client
                        j = 0;
                        bzero(data_temp, BUFFER_LENGTH);        // initialize
                        for(i = 5; i < BUFFER_LENGTH; i++){
                            if(data_recv[i] != ','){
                                data_temp[j] = data_recv[i];
                                j++;
                            }
                            else{
                                break;
                            }
                        }
                        toID = atoi(data_temp);
                        j = 0;
                        bzero(toMessage, BUFFER_LENGTH);
                        for(i++; i < BUFFER_LENGTH; i++){
                            if(data_recv[i] != '\n'){
                                toMessage[j] = data_recv[i];
                                j++;
                            }
                            else{
                                break;
                            }
                        }
                        if( send(allfd[toID], toMessage, strlen(toMessage), 0) > 0){
                            printf("data send success!\n");
                        }
                        else{
                            perror("[data send]: Server has NOT sent your message!\n");
                            exit(errno);
                        }
                    }
                }
                /*else if(recvlen == 0){
                    perror("data receive: Client has quit and stopped chatting!\n");
                }
                else{
                    perror("data receive: Client has NOT sent message successfully!\n");
                }*/
            }
        }
    }

    // clear current pthread
    printf("terminating current client connection...\n");
    isConnect[nowid] = 0;
    close(newfd);
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    int sockfd;
    int newfd;
    struct sockaddr_in s_addr;
    struct sockaddr_in c_addr;
    int length;

    bzero(isConnect, MAX_CONN_LIMIT);

    // create sockfd
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // ipv4, TCP
    if(sockfd == -1){
        perror("socket");
        exit(errno);
    }
    else{
        printf("socket create success!\n");
    }

    // set the attr of structure socket address
    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);     //trans addr from uint32_t host byte order to network byte order.
    s_addr.sin_port = htons(SOCK_PORT);          //trans port from uint16_t host byte order to network byte order.

    // bind ip address and port
    if(bind(sockfd, (struct sockaddr*)&s_addr, sizeof(struct sockaddr)) == -1){
        perror("bind");
        exit(errno);
    }
    else{
        printf("bind success!\n");
    }

    // listen
    if(listen(sockfd, MAX_CONN_LIMIT)  == -1){
        perror("listen");
        exit(errno);
    }
    else{
        printf("listen success\n");
    }

    // communicate
    while(1){
        // accept
        length = sizeof(struct sockaddr);
        newfd = accept(sockfd, (struct sockaddr *)(&c_addr), &length);
        if(newfd == -1){
            perror("accept error!\n");
            continue;
        }
        else{
            printf("A new connection occurs!\n");
            printf("The client is: %s: %d \n", inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port));
            // store address and port
            alladdr[id] = (char*)malloc(sizeof(char) * ADDR_LENGTH);
            strcpy(alladdr[id], inet_ntoa(c_addr.sin_addr));
            allport[id] = ntohs(c_addr.sin_port);
            isConnect[id] = 1;
            // new thread
            if(pthread_create(&thread_id[id], NULL, (void *)(&communicate),(void *)(&newfd)) == -1){
                id--;
                perror("thread create error!\n");
                break;
            }
            id++;
        }
    }

    // clear all
    if(shutdown(sockfd, SHUT_WR) == -1){
        perror("shut down error!\n");
    }
    else{
        printf("server shuts down!\n");
    }
    return 0;
}
