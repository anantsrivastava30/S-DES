/*
* Renan Santana / PID: 4031451 / HW4 / DUE: DEC 4, 2014
*/

#include "functions_header.h"

const int VECTOR_SIZE_P = 8, P8_SIZE_P = 8, P10_SIZE_P = 10;

int main(int argc, char* argv[]){
	int file_size, read_byte = 0, i, byte_print;
	char str_key[P10_SIZE_P], str_init_v[VECTOR_SIZE_P];
	unsigned char buffer, cipher_minus1, vector;
	int subkeys[2][P8_SIZE_P];
	int key[P10_SIZE_P], init_v[VECTOR_SIZE_P];
	
	// format input of mycipher:
	// [-d] <init_key> <init_vector> <original_file> <result_file>
	if(argc < 5 || argc > 6){
		printf("Format of input: [-d] <init_key> <init_vector> <original_file> <result_file>\n");
		return -1;
	}
	else{
		if(argc == 5 && !strcmp(argv[1], "-d")){ 
			printf("Format of input maybe missing a param.\n");
			return -1;
		}
		else if(argc == 6 && strcmp(argv[1], "-d") != 0){ 
			printf("Format has a bad input.\n");
			return -1;
		}
	}
	
	// open file
	FILE *orig;
	FILE *res;
	
	/* Open result file + original plaintext file + get file size */
	res = fopen(argv[argc == 6 ? 5 : 4], "w+b");
	orig = fopen(argv[argc == 6 ? 4 : 3], "rb");
	if(orig == NULL)
		{ printf("Invalid file name: %s\n", argv[argc == 6 ? 4 : 3]); return 0; }
	
	fseek(orig, 0, SEEK_END);
	file_size = ftell(orig);
	fseek(orig, 0, SEEK_SET);
	byte_print = file_size < 5 ? file_size : 5;
	/* Open result file + original plaintext file + get file size */
	
	/* Convert vector str into unsigned char */
	if(strlen(argv[argc == 6 ? 3 : 2]) != 8)
		{ printf("Vector size error.\n"); return 2;}
	memcpy(str_init_v, argv[argc == 6 ? 3 : 2], VECTOR_SIZE_P);
	for(i = 0; i < VECTOR_SIZE_P; i++)
		{ init_v[i] = str_init_v[i] == '1' ? 1 : 0; }
	// convert initialization vertor from str to unsigned char
	for(i = 0; i < P8_SIZE_P; i++){ 
		vector = vector | init_v[i];
		if(i < 7){ vector <<= 1; }
	}
	/* Convert vector str into unsigned char */
	
	/* Extract the main key from param */
	if(strlen(argv[argc == 6 ? 2 : 1]) != 10)
		{ printf("Key size error.\n"); return 2;}
	memcpy(str_key, argv[argc == 6 ? 2 : 1], P10_SIZE_P);
	for(i = 0; i < P10_SIZE_P; i++)
		{ key[i] = str_key[i] == '1' ? 1 : 0; }
	/* Extract the main key from param */
	
	make_subkeys(subkeys, key);
	
	switch(argc){
		// decode
		/* Cipher Block Chaining Note
		 * Decrypting with the incorrect init vector causes the first block of plaintext
		 * to be corrupt but subsequent plaintext will be correct. This is b/c a plaintext
		 * can be recovered from two adjacent blocks of ciphertext. As a consequnce
		 * the decryption can be parallelized.
		*/
		case 6:
			printf("\nDECODE FILE: '%s' -> '%s'\n",argv[argc == 6 ? 4 : 3] ,argv[argc == 6 ? 5 : 4] );
			printf("*** First %d characters decrypted ***\n", byte_print);
			
			for(; read_byte < file_size; read_byte++){
				// read cipher
				fread(&buffer, 1, 1, orig);
				cipher_minus1 = buffer;
				 
				// plaintext = IP-1( fk1( SW( fk2( IP(ciphertext) ) ) ) )
				buffer = inv_ip( fk( sw( fk( ip(buffer), subkeys[1]) ), subkeys[0]) );
				
				// cipher block chaining
				buffer = vector ^ buffer;
				vector = cipher_minus1;
				
				if(read_byte < byte_print){ printbits(buffer); }
				
				// write plaintext
				fwrite(&buffer, 1, 1, res);
			}
			break;
		//encode
		default:
			printf("\nENCODE FILE: '%s' -> '%s'\n",argv[argc == 6 ? 4 : 3] ,argv[argc == 6 ? 5 : 4] );
			printf("*** First %d characters encrypted ***\n", byte_print);
			
			for(; read_byte < file_size; read_byte++){
				// read plaintext
				fread(&buffer, 1, 1, orig);
				
				// cipher block chaining
				buffer = vector ^ buffer;
				
				// ciphertext = IP-1( fk2( SW( fk1( IP(plaintext) ) ) ) )
				vector = buffer = inv_ip( fk( sw( fk( ip(buffer), subkeys[0]) ), subkeys[1]) );
				
				if(read_byte < byte_print){ printbits(buffer); }
				
				// write cipher
				fwrite(&buffer, 1, 1, res);
			}
			break;	
	}
	
	fclose(orig);
	fclose(res);
	return 1;
}
