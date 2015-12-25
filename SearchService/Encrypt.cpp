#include "Encrypt.h"
#include <assert.h>

#include <cstring>
#include <iostream>

using namespace std;

char* Base64::encode(const unsigned char *data, size_t input_len, size_t& output_len)
{
  char encoding_table[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  output_len= 4 * ((input_len+ 2) / 3);

  char *encoded_data = (char*) malloc(output_len+1);
  if (encoded_data == NULL) return NULL;

  for (int i = 0, j = 0; i < (int)input_len;) {
    unsigned int octet_a = i < (int)input_len? (unsigned char)data[i++] : 0;
    unsigned int octet_b = i < (int)input_len? (unsigned char)data[i++] : 0;
    unsigned int octet_c = i < (int)input_len? (unsigned char)data[i++] : 0;

    unsigned int triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

    encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
  }

  int mod_table[] = {0, 2, 1};
  for (int i = 0; i < mod_table[input_len% 3]; i++)
    encoded_data[output_len- 1 - i] = '=';
  encoded_data[output_len] = '\0';
  return encoded_data;
}

char* Base64::decode(const char *data, size_t input_len, size_t& output_len)
{
  char encoding_table[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  char decoding_table[256];

  for (int i = 0; i < 64; i++)
    decoding_table[(unsigned char) encoding_table[i]] = i;

  if (input_len% 4 != 0) return NULL;

  output_len= input_len/ 4 * 3;
  if (data[input_len- 1] == '=') output_len--;
  if (data[input_len- 2] == '=') output_len--;

  char *decoded_data = new char[output_len];
  if (decoded_data == NULL) return NULL;

  for (int i = 0, j = 0; i < (int)input_len; i+=4) {
//    cerr << data[i] <<" " << data[i+1] << " " << data[i+2] << " " << data[i+3] 
//         << uppercase << hex << (unsigned short)data[i+3] << "==>"; 
/*
    unsigned int sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
    unsigned int sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
    unsigned int sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
    unsigned int sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
    cerr << uppercase << hex << sextet_a << " "<< sextet_b << " "<< sextet_c << " "<< sextet_d;
    unsigned int triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6)
                    + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);
*/
    unsigned int triple = 0;
    for (size_t q= 0; q< 4; q++) {
      triple <<= 6;
      char cursor = data[i+q] == ' ' ? '+' : data[i+q];
      unsigned int sextet = cursor == '=' ? 0 : decoding_table[(int)cursor];
//      cerr << uppercase << hex << sextet << " ";
      triple |= sextet;
    }
    if (j < (int)output_len) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
    if (j < (int)output_len) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
    if (j < (int)output_len) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
//    cerr << " :: " << uppercase << hex << triple << " :: "
//         << (unsigned short) decoded_data[j-3] << " " 
//         << (unsigned short) decoded_data[j-2] << " " << (unsigned short) decoded_data[j-1] << endl;
  }

  return decoded_data;
}

