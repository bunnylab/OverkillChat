#include "otp.h"
#include <iostream>
#include <string.h>
#include <string>
#include <time.h>

// Initialize OTP with a key file
Otp::Otp(char const *filename){
  srand(time(NULL)); // seed Random
  this->filename = filename;

  // set buffer and file indexes to zero will indicate if open succeeded
  buffer_location = 0;
  buffer_fill = 0;

  // try to open file and fill buffer
  otp_file = fopen(this->filename, "rb+");
  if(otp_file!=NULL){
    buffer_fill = this->FillBuffer();
  }
  else{
    std::cout << "No File at " << filename << std::endl;
  }
}

// Class destructor
Otp::~Otp(){
  fclose(otp_file);
}


// Get bits from keyfile until buffer is full
int Otp::FillBuffer(){
  int sz = 0;
  int bytes_read = 0;

  // get size of file
  fseek(otp_file, 0L, SEEK_END);
  sz = ftell(otp_file);
  rewind(otp_file);

  // if size(keyfile) is less than buffer_size
  if(sz<buffer_size && sz>0){
    fread(&buffer, 1, sz, otp_file);
    bytes_read = sz;
  }
  // if keyfile is empty
  else if(sz<=0){
    std::cout << "Error: keyfile is empty" << std::endl;
    bytes_read = 0;
    return bytes_read;
  }
  // else keyfile is greater than buffer_size
  else{
    fread(&buffer, 1, buffer_size, otp_file);
    bytes_read = buffer_size;
  }
  std::cout << "Loading " << bytes_read << " bytes of key ";
  std::cout << sz - bytes_read << " bytes left in keyfile" << std::endl;

  // Overwrite used keyfile and then delete that section
  rewind(otp_file);
  for(int i=0; i<bytes_read; ++i){
    fputc((char)(rand() % 127 + 33), otp_file);
  }
  // read in remaining keyfile
  char file_buffer[sz-bytes_read];
  fread(&file_buffer, 1, sz-bytes_read, otp_file);

  // close, open as empty, write buffer, close, then reopen for read
  fclose(otp_file);
  otp_file = fopen(this->filename, "wb");
  fwrite(&file_buffer, 1, sz-bytes_read, otp_file);
  fclose(otp_file);
  otp_file = fopen(this->filename, "rb+");

  return bytes_read;

}

// The output shows the last couple bytes of the temp Array
// as consistently messed up check what's going on there
// seems to work otherwise though
int Otp::Encrypt(unsigned char input_array[], int size){
  char temp[size];
  int current_byte = 0;

  if(size < buffer_size - buffer_location){
    memcpy(temp, buffer+buffer_location, size);
    this->buffer_location += size;
  }
  // ban messages longer than our max buffer size and then do only one fill call
  else{
    std::cout << "Rolling over the buffer..." << std::endl;
    // copy remaining bytes to temp
    memcpy(temp, buffer+buffer_location, buffer_size-buffer_location);
    current_byte += buffer_size - buffer_location;

    // fill buffer
    this->buffer_fill = this->FillBuffer();
    this->buffer_location = 0;

    // finish copying to temp
    memcpy(temp+current_byte, buffer, size-current_byte);
    this->buffer_location += size-current_byte;

    }
  for(int x=0; x<size; ++x){
    input_array[x] = temp[x]^input_array[x];
  }
  return size;
}
