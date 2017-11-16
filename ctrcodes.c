#include "ctrcodes.h"

#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

void listAvailable(int *sd, int *client_socket, char *buffer, int max_clients, int max_buffer){
  struct sockaddr_in address;
  int addrlen = sizeof(address);


  for(int i=0; i<max_clients; i++){
    if(client_socket[i]>0)
    {
      printf("%d %d\n", i, client_socket[i]);
      getpeername(client_socket[i], (struct sockaddr*)&address , \
          (socklen_t*)&addrlen);
      // Shift all indexes the users touch up by 1
      sprintf(buffer, "Peer #%d , ip %s , port %d \n" ,
            i+1, inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
      send(*sd , buffer , strlen(buffer) , 0 );
    }

    }

}
