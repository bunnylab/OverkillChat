#ifndef OTP_H
#define OTP_H

#include <stdio.h>
#include <iostream>
#include <string>

class Otp{
private:
  const int buffer_size = 100;
  char buffer[100];
  int buffer_fill;
  int buffer_location;
  FILE * otp_file;
  const char * filename;

public:
  Otp(char const *filename);
  ~Otp();
  char generateRandomChar();
  int FillBuffer();
  int Encrypt(unsigned char* input_array, int size);
  int Decrypt(unsigned char* input_array, int size);

};



#endif /* OTP_H */
