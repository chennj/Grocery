//g++  -o encrypt encrypt.cpp -L/usr/lib -lssl -lcrypto
#include <openssl/aes.h>
#include <stdio.h>

int main() {

	int bytes_read, bytes_written;
	unsigned char indata[AES_BLOCK_SIZE];
	unsigned char outdata[AES_BLOCK_SIZE];

	/* ckey and ivec are the two 128-bits keys necesary to
	en- and recrypt your data.  Note that ckey can be
	192 or 256 bits as well */
	unsigned char ckey[] = "thiskeyisverybad";
	unsigned char ivec[] = "dontusethisinput";

	/* data structure that contains the key itself */
	AES_KEY key;

	/* set the encryption key */
	AES_set_encrypt_key(ckey, 128, &key);

	/* set where on the 128 bit encrypted block to begin encryption*/
	int num = 0;

	FILE *ifp = fopen("test.txt", "rb");
	FILE *ofp = fopen("test_en.txt", "wb");

	while (true) {
		bytes_read = fread(indata, 1, AES_BLOCK_SIZE, ifp);

		AES_cfb128_encrypt(indata, outdata, bytes_read, &key, ivec, &num,
			AES_ENCRYPT); //or AES_DECRYPT

		bytes_written = fwrite(outdata, 1, bytes_read, ofp);
		if (bytes_read < AES_BLOCK_SIZE)
			break;
	}
}
