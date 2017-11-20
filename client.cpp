#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <thread>
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "ctrcodes.h"
#include "otp.h"
#include "base64.h"

using namespace std;

void asyncRecieve(int* socket, int buff_size, Otp* pad);
const int buff_size = 1025;

int main(int argc, char** argv){
  int sockfd;
  struct sockaddr_in servaddr;
  char buffer[buff_size];

  // check our arguments
  if(argc != 5){
      cout << "CLIENT address port keyfile_out keyfile_in\n";
      return 1;
    }

  // Initialize OTP
  Otp otp_out = Otp(argv[3]);
  Otp otp_in = Otp(argv[4]);

  // setup sending socket and addresses
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons( atoi(argv[2]) );
  inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

  // initialize socket
  if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    cout << "Socket Creation Error" << endl;
    return 1;
  }

  // connect socket
  cout << "Connecting to Server..." << endl;
  if( connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0){
    cout << "Connection Error" << endl;
    return 1;
  }

  // read intro message
  cout << "Getting Welcome Message..." << endl;

  // create thread to listen for messages from server
  thread tRead(asyncRecieve, &sockfd, buff_size, &otp_in);

  // send on socket using main loop
  cout << "Send Commands with #x default is messages" << endl;
  char input_string[1025];
  for(;;){
    //cout << "> ";
    cin.getline(input_string, 1025);

    // send command no encryption
    if(input_string[0]=='#'){
      strcpy(buffer, input_string);
      send(sockfd , buffer , strlen(buffer) , 0 );
    }
    // send encrypted message
    else{
      if(cin.gcount() > otp_out.buffer_size){
        cout << "Message must be under" << otp_out.buffer_size << " bytes..." << endl;
        continue;
      }

      // This is so messy there has to be a way to simplify
      strcpy(buffer, "#m:");
      unsigned char message_buffer[strlen(input_string)];
      strcpy( (char*) message_buffer, input_string);

      // pass our message_buffer by reference to otp_encrypt
      int bytes_encrypted = otp_out.Encrypt(message_buffer, sizeof(message_buffer));
      if(bytes_encrypted <= 0){
        cout << "Encryption Error... " << endl;
        continue;
      }

      string cipher_encode = base64_encode(message_buffer, sizeof(message_buffer));
      strcpy( buffer+3, cipher_encode.c_str());
      send(sockfd , buffer , strlen(buffer) , 0 );
    }

    memset(buffer, 0, buff_size);
  }

  tRead.join();  // make sure thread comes back

  return 0;
}


// asyncRecieve
void asyncRecieve(int* socket, int buff_size, Otp* pad){
  char buffer[buff_size];
  int n = 0;
  while( (n = recv(*socket, buffer, buff_size, 0)) > 0){
    if(memcmp(buffer, MESSAGE, 3) == 0){
      string cipher_encode(buffer+3);
      string cipher_decode = base64_decode(cipher_encode);
      unsigned char message_buffer[cipher_decode.size()];
      strcpy( (char*) message_buffer, cipher_decode.c_str());
      int x = pad->Encrypt(message_buffer, sizeof(message_buffer));
      cout << "Decrypted Plaintext: " << message_buffer << endl << endl;

    } else{
      cout << endl << buffer;
    }
    flush(cout);
    memset(buffer, 0, buff_size);
  }
}
