/*
* Renan Santana / PID: 4031451 / HW4 / DUE: DEC 4, 2014
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const int P10_SIZE;
extern const int P8_SIZE;
extern const int VECTOR_SIZE;

unsigned char ip(unsigned char c);
void p10(int key[]);
void p8(int left[], int right[], int subkey[]);
unsigned char p4(int s1, int s2);
unsigned char inv_ip(unsigned char ip1);
unsigned char sw(unsigned char flip);
unsigned char fk(unsigned char ip, int sk[]);
void make_subkeys(int subkeys[][P8_SIZE], int key[]);
void printbits(char c);
