#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#include "ctrcodes.h"

#define TRUE   1
#define FALSE  0
#define PORT 8888
#define MAX_CLIENTS 30


int main(int argc , char *argv[])
{
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[30] ,
          activity, i , valread , sd;
    int max_sd;
    struct sockaddr_in address;
    // Make a connection matrix for clients
    static int connection_matrix[MAX_CLIENTS][MAX_CLIENTS] = {0};

    int max_buffer = 1025;
    char buffer[max_buffer];  //data buffer of 1K

    //set of socket descriptors
    fd_set readfds;

    //a message
    char *message = "P2P Chat Test \r\n #a: list available peers \r\n #l: list connected peers\
      \r\n #c:0-999 connect to peer # \r\n #p:0-999 disconnect from peer # \r\n\
      #d: disconnect from server \r\n #m: hello, send message to connected peers \r\n";

    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        client_socket[i] = 0;
    }

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
          sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while(TRUE)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for ( i = 0 ; i < MAX_CLIENTS ; i++)
        {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno!=EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket,
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n"
              , new_socket , inet_ntoa(address.sin_addr) , ntohs
                  (address.sin_port));

            //send new connection greeting message
            if( send(new_socket, message, strlen(message), 0) != strlen(message) )
            {
                perror("send");
            }

            puts("Welcome message sent successfully");

            //add new socket to array of sockets
            for (i = 0; i < MAX_CLIENTS; i++)
            {
                //if position is empty
                if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n" , i);

                    break;
                }
            }
        }

        //else its some IO operation on some other socket
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET( sd , &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" ,
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }

               //Echo back the message that came in
               else
                {
                    // Control codes for chat server
                    if(memcmp(buffer, LIST_AVAILABLE, 3) == 0){
                      listAvailable(&sd, &client_socket, &buffer, MAX_CLIENTS, max_buffer);
                      printf("We got #a: \n");
                    } // #a:

                    // List current connected
                    // way its coded right now will prolly overflow buffer
                    else if(memcmp(buffer, LIST_CONNECTED, 3) == 0){
                      printf("We got #l: \n");
                      char temp_buffer[1025];
                      sprintf(temp_buffer, "You are connected to \n");
                      for(int j=0; j<MAX_CLIENTS; j++){
                        if(connection_matrix[i][j] > 0){
                          sprintf(temp_buffer + strlen(temp_buffer), "Client #%d\n", j+1);
                        }
                      }
                      send(sd , temp_buffer , strlen(temp_buffer) , 0 );


                    } // #l:

                    else if(memcmp(buffer, CONNECT_TO, 3) == 0){

                      printf("We got #c: \n");
                      char temp[4];
                      int requested_client = 0;
                      strncpy(temp, buffer+3, 4);

                      if( ( requested_client = atoi(temp) ) > 0 ){
                        //printf("requesting client: %d\n", i);
                        //printf("requestd client: %d\n", requested_client);

                        if(client_socket[requested_client-1]>0){
                          //printf("requested client is alive\n");
                          connection_matrix[i][requested_client-1] = 1;
                          for(int x=0; x<MAX_CLIENTS; x++)
                            printf("%d ", connection_matrix[i][x]);

                        }else{
                          printf("Requested Client Not Connected\n");
                        }

                      } else{
                        printf("Error Converting Input");
                      }


                    } // #c:

                    else if(memcmp(buffer, DISCONNECT_PEER, 3) == 0){
                      printf("We got #p: \n");
                      char temp[4];
                      int requested_client = 0;
                      strncpy(temp, buffer+3, 4);
                      if( ( requested_client = atoi(temp) ) > 0 ){
                        connection_matrix[i][requested_client-1] = 0;
                      } else{
                        printf("Error Converting Input");
                      }

                    }

                    else if(memcmp(buffer, DISCONNECT, 3) == 0){
                      printf("We got #d: \n");
                      close( sd );
                      for(int j=0; j<MAX_CLIENTS; j++)
                        connection_matrix[i][j] = 0;
                      client_socket[i] = 0;
                    }

                    // #m: Message Passing Code
                    else if(memcmp(buffer, MESSAGE, 3) == 0){
                      printf("We got #m: \n");
                      buffer[valread] = '\0';
                      for(int j=0; j<MAX_CLIENTS; j++){
                        if(connection_matrix[i][j] > 0){
                          printf("Sending Client %d to client %d\n", i, j);
                          send(client_socket[j] , buffer , strlen(buffer) , 0 );
                        }
                      }
                    }

                    else{
                      printf("Error no proper control code\n");
                      printf(buffer);
                    }
                    //set the string terminating NULL byte on the end
                    //of the data read
                    //buffer[valread] = '\0';

                    //printf("valread %d\n", valread);
                    //printf("buffer content: %s \n", buffer);
                    //send(sd , buffer , strlen(buffer) , 0 );
                }
            }
        }
    }

    return 0;
}
