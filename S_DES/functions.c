/*
* Renan Santana / PID: 4031451 / HW4 / DUE: DEC 4, 2014
*/

#include "functions_header.h"
#define MSB_MASK (1<<(sizeof(char)*8-1))

const int VECTOR_SIZE = 8, P8_SIZE = 8, P10_SIZE = 10;
const int s_matrix0[4][4] = { {1,0,3,2},
										{3,2,1,0},
										{0,2,1,3},
										{3,1,3,2} };
const int s_matrix1[4][4] = { {0,1,2,3},
										{2,0,1,3},
										{3,0,1,0},
										{2,1,0,3} };

/*
 * Used to print the binary representation of the 
 * char (encrypted or decrypted)
*/
void printbits(char c){
	int i = 0;
	for(; i < 8; i++){
		printf("%d", (int)((c & MSB_MASK) >> (sizeof(char)*8-1)));
		c <<= 1;
	}
	puts("");
}

/*
 * Initial Permutation
 * Permutation 	  1 5 2 0 3 7 4 6
 *	Left association 0 1 2 3 4 5 6 7
 * Index in char    7 6 5 4 3 2 1 0
*/
unsigned char ip(unsigned char c){
	unsigned char tmp = 0, mask = 0;
												// index
	mask = 2;
	tmp = tmp | ((c & mask) >> 1);	// 0 : 6
	mask = 8;
	tmp = tmp | ((c & mask) >> 2);	// 1 : 4
	mask = 1;
	tmp = tmp | ((c & mask) << 2);	// 2 : 7
	mask = 0x10;
	tmp = tmp | ((c & mask) >> 1);	// 3 : 3
	mask = 0x80;
	tmp = tmp | ((c & mask) >> 3);	// 4 : 0
	mask = 0x20;
	tmp = tmp | (c & mask);				// 5 : 2
	mask = 4;
	tmp = tmp | ((c & mask) << 4);	// 6 : 5
	mask = 0x40;
	tmp = tmp | ((c & mask) << 1);	// 7 : 1
	
	return tmp;
}

/*
 * Permutation of Key
 * Permutation 	  2 4 1 6 3 9 0 8 7 5
 *	Left association 0 1 2 3 4 5 6 7 8 9
 * Index			     9 8 7 6 5 4 3 2 1 0
*/
void p10(int key[]){
	int i = 0;
	int p10k[P10_SIZE];
	
	p10k[0] = key[2];
	p10k[1] = key[4];
	p10k[2] = key[1];
	p10k[3] = key[6];
	p10k[4] = key[3];
	p10k[5] = key[9];
	p10k[6] = key[0];
	p10k[7] = key[8];
	p10k[8] = key[7];
	p10k[9] = key[5];
	for(; i < P10_SIZE; i++){ key[i] = p10k[i]; }
}

/*
 * Permutation of SubKeys
 * Permutation k1	  6 3 7 4 8 5 10 9
 * Permutation k2	  5 2 6 3 7 4 9  8
 *	Left association 0 1 2 3 4 5 6  7 
 * Index			     7 6 5 4 3 2 1  0
*/
void p8(int left[], int right[], int subkey[]){
	
	subkey[0] = right[0];
	subkey[1] = left[2];
	subkey[2] = right[1];
	subkey[3] = left[3];
	
	subkey[4] = right[2];
	subkey[5] = left[4];
	subkey[6] = right[4];
	subkey[7] = right[3];
}

/*
 * Permutation from s-box (row, col)
 * Permutation 	  			1 3 | 2 0
 *	Left association 			0 1 | 2 3
 * Index in char    			3 2 | 1 0
*/
unsigned char p4(int s1, int s2){
	unsigned char tmp = 0, mask;
												// index		visual
	mask = 2;
	tmp = tmp | ((s1 & mask) >> 1);	// 0 : 0		tmp[0] = s1[1]
	tmp = tmp | (s2 & mask);			// 1 : 2		tmp[1] = s2[1]
	mask = 1;
	tmp = tmp | ((s2 & mask) << 2);	// 2 : 3		tmp[2] = s2[0]
	tmp = tmp | ((s1 & mask) << 3);	// 3 : 1		tmp[3] = s1[0]
	
	return tmp;
}

/*
 * Inverse Permutation
 * Permutation 	  5 7 1 6 4 2 0 3
 *	Left association 0 1 2 3 4 5 6 7
 * Index in char    7 6 5 4 3 2 1 0
*/
unsigned char inv_ip(unsigned char ip1){
	unsigned char tmp = 0, mask;
													// index
	mask = 4;
	tmp = tmp | ((ip1 & mask) >> 2);		// 0 : 5
	mask = 1;
	tmp = tmp | ((ip1 & mask) << 1);		// 1 : 7
	mask = 0x40;
	tmp = tmp | ((ip1 & mask) >> 4);		// 2 : 1
	mask = 2;
	tmp = tmp | ((ip1 & mask) << 2);		// 3 : 6
	mask = 8;
	tmp = tmp | ((ip1 & mask) << 1);		// 4 : 4
	mask = 0x20;
	tmp = tmp | (ip1 & mask);				// 5 : 2
	mask = 0x80;
	tmp = tmp | ((ip1 & mask) >> 1);		// 6 : 0
	mask = 0x10;
	tmp = tmp | ((ip1 & mask) << 3);		// 7 : 3
	
	return tmp;
}

/*
 * Switch (the second key operates of a different set of bits)
 * Operation	7 6 5 4 | 3 2 1 0   =>    3 2 1 0 | 7 6 5 4		
*/
unsigned char sw(unsigned char flip){
	unsigned char tmp = 0, mask1 = 0x10, mask2 = 1;
	int i;
	
	for(i = 0; i < 4; i++){
		tmp = tmp | ((flip & mask1) >> 4);
		tmp = tmp | ((flip & mask2) << 4);
		mask1 <<= 1;
		mask2 <<= 1;
	}
	
	return tmp;
}

/*
 * fk = (L XOR F(R, SK), R)
 *
 * Operations from F()
 * Expand / Permute  (EP: the right side of ip)
 *	Permutation 		3 0 1 2  1 2 3 0
 * Left association	0 1 2 3	0 1 2 3
 *	Index					7 6 5 4  3 2 1 0
 * 
 * F = EP XOR SubKey_i
 * S-box: from F 		3 | 0 1 | 2		=> (32)->ROW | (01)->COL for s1
 *							1 | 2 3 | 0		=> (10)->ROW | (23)->COL for s0
 *
 * Perform P4 ( s0, s1 )
 * Shift Left 4 bits (s0s1) ORed with Right side of IP 
*/
unsigned char fk(unsigned char ip, int sk[]){
	int row = 0, col = 0, i = 0, s1 = 0, s2 = 0;
	unsigned char left = 0, right = 0, sk_tmp = 0, ep = 0, mask = 0, f = 0;
	mask = 0xf0;
	left = left | (ip & mask);
	mask = 0x0f;
	right = right | (ip & mask);
	
	// make the sk into a unsigned char
	for(i = 0; i < P8_SIZE; i++){ 
		sk_tmp = sk_tmp |  sk[i];
		if(i < 7){ sk_tmp <<= 1; }
	}
	
	// EP
												// index
	mask = 8;
	ep = ep | ((right & mask) >> 3);	// 0 : 0
	mask = 1;
	ep = ep | ((right & mask) << 1);	// 1 : 3
	mask = 2;
	ep = ep | ((right & mask) << 1);	// 2 : 2
	mask = 4;
	ep = ep | ((right & mask) << 1);	// 3 : 1
	
	mask = 2;
	ep = ep | ((right & mask) << 3);	// 4 : 2
	mask = 4;
	ep = ep | ((right & mask) << 3);	// 5 : 1
	mask = 8;
	ep = ep | ((right & mask) << 3);	// 6 : 0
	mask = 1;
	ep = ep | ((right & mask) << 7);	// 7 : 3
	
	f = ep ^ sk_tmp;
	
	// right halve of f | matrix 1		| index
	mask = 0x0f;
	right = f & mask;				
	mask = 1;
	row = row | (right & mask);			// 0 : 0
	mask = 8;
	row = row | ((right & mask) >> 2);	// 1 : 3
	mask = 2;
	col = col | ((right & mask) >> 1);	// 0 : 1
	mask = 4;
	col = col | ((right & mask) >> 1);	// 1 : 2
	
	s2 = s_matrix1[row][col];
	
	row = col = 0;
	// left halve of f | matrix 0			| index
	mask = 0xf0;
	left = f & mask;				
	mask = 0x10;
	row = row | ((left & mask) >> 4);	// 0 : 4
	mask = 0x80;
	row = row | ((left & mask) >> 6);	// 1 : 7
	mask = 0x20;
	col = col | ((left & mask) >> 5);	// 0 : 5
	mask = 0x40;
	col = col | ((left & mask) >> 5);	// 1 : 6
	
	s1 = s_matrix0[row][col];
	
	// left (left of ip) XOR (f (p4) shift L 4)
	f = p4(s1, s2);
	left = (ip & 0xf0);
	f = (left ^ (f << 4));
	
	// f (XORed (left , p4) ORed (right (ip))
	return (f | (ip & 0x0f));
}

/*
 * Make K1 & K2
 * K1 & K2 demonstration
 * 	Master Key: 0 1 2 3 4 5 6 7 8 9
 *		K1 shift left 1 / K2 shift left 2
 * 	K1 = P8( 1 2 3 4 0, 6 7 8 9 5 )
 *		K2 = P8( 3 4 0 1 2, 8 9 5 6 7 )
*/
void make_subkeys(int subkeys[][P8_SIZE], int key[]){
	int index = 0, i;
	int left5[5], right5[5], left[5], right[5];
	
	p10(key);
		
	for(i = 0; i < 5; i++){ left5[i] = key[i]; right5[i] = key[i+5]; }

	// left shift 1
	for(i = 1; index < 5; i++){ 
		left[index] = left5[i%5]; 
		right[index++] = right5[i%5]; 
	}
	p8(left, right, subkeys[0]);
	
	// left shift 2
	index = 0;
	for(i = 3; index < 5; i++){ 
		left[index] = left5[i%5]; 
		right[index++] = right5[i%5]; 
	}
	p8(left, right, subkeys[1]);
}
