#include <iostream>
#include <cstring>
#include <bitset>
#include <cstdlib>

#include "destables.h"

using namespace std;

bitset<32> sbox(bitset<48> input);
bitset<48> keysched(int round, bitset<56> inkey);
bitset<32> feistel(bitset<48> subkey, bitset<32> rinput);

int main(int argc, char* argv[])
{
	string input;
	bitset<64> inputChunk("0000000100100011010001010110011110001001101010111100110111101111");
	//bitset<64> inputChunk("1000010111101000000100110101010000001111000010101011010000000101");
	bitset<64> key("0001001100110100010101110111100110011011101111001101111111110001");
	
	for (int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i],"-i") == 0)
		{
			cout << "test" << endl;
			input = argv[(i++)+1];
		}
		else if(strcmp(argv[i],"-if") == 0)
		{
			
		}
		else if(strcmp(argv[i],"-k") == 0)
		{
			key = (uint64_t)atoi(argv[(i++)+1]);
		}
		else if(strcmp(argv[i],"-kf") == 0)
		{
			
		}
	}
	
	if (strcmp(argv[1],"-e") == 0)
	{
		// Apply IP to the input
		bitset<64> inputChunkInit = 0;
		for(int i = 0; i < 64; i++)
		{
			inputChunkInit.set(63-i, inputChunk.test(64-IP[i]));
		}
		
		// Split input
		bitset<32> linput, rinput;
		for(int i = 0; i < 32; i++)
		{
			linput.set(i, inputChunkInit.test(i+32));
			rinput.set(i, inputChunkInit.test(i));
		}
		
		// Apply PC-1 to the key
		bitset<56> inkey = 0;
		for(int i = 0; i < 56; i++)
		{
			inkey.set(55-i, key.test(64 - PC1[i]));
		}
		
		for(int round = 0; round < 16; round++)
		{
			//***** Key Scheduler *****
			bitset<48> subkey = keysched(round, inkey);
			
			//***** Feistel Cipher *****
			bitset<32> fOut = feistel(subkey, rinput);
			
			// XOR Left side and output of Feistel
			linput ^= fOut;
			
			// Swap Left and Right
			bitset<32> temp = linput;
			linput = rinput;
			rinput = temp;
		}
		
		bitset<64> output(rinput.to_string() + linput.to_string());
		
		// Apply IP-1 to the output
		bitset<64> result;
		for(int i = 0; i < 64; i++)
		{
			result.set(63-i, output.test(64 - IP1[i]));
		}
		cout << endl << endl << result << endl;
	}
	else if(strcmp(argv[1],"-d") == 0)
	{
		// Apply IP-1 to the input
		bitset<64> ip1Out;
		for(int i = 0; i < 64; i++)
		{
			ip1Out.set(63-i, inputChunk.test(64 - IP1[i]));
		}
		
		// Split input
		bitset<32> linput, rinput;
		for(int i = 0; i < 32; i++)
		{
			linput.set(i, ip1Out.test(i+32));
			rinput.set(i, ip1Out.test(i));
		}
		
		// Apply PC-1 to the key
		bitset<56> inkey = 0;
		for(int i = 0; i < 56; i++)
		{
			inkey.set(55-i, key.test(64 - PC1[i]));
		}
		
		for(int round = 15; round >= 0; round--)
		{
			//***** Key Scheduler *****
			bitset<48> subkey = keysched(round, inkey);
			
			//***** Feistel Cipher *****
			bitset<32> fOut = feistel(subkey, rinput);
			
			// XOR Left side and output of Feistel
			linput ^= fOut;
			
			// Swap Left and Right
			bitset<32> temp = linput;
			linput = rinput;
			rinput = temp;
		}
		
		// Apply IP to the input
		bitset<64> result = 0;
		for(int i = 0; i < 64; i++)
		{
			result.set(63-i, inputChunk.test(64-IP[i]));
		}
		cout << result << endl;
		
	}
	
	return(0);
}

bitset<32> sbox(bitset<48> input)
{
	bitset<32> sOut;
	for(int i = 0; i < 8; i++)
	{
		bitset<2> row;
		//cout << 6*i + 0 << " " << input.test(47 - (6*i + 0)) << endl;
		row.set(1, input.test(47 - (6*i + 0)));
		//cout << 6*i + 5 << " " << input.test(47 - (6*i + 5)) << endl;
		row.set(0, input.test(47 - (6*i + 5)));
		
		bitset<4> col;
		for(int j = 0; j < 4; j++)
		{
			//cout << 6*i + j + 1 << " " << input.test(47 - (6*i + j + 1)) << endl;
			col.set(3 - j, input.test(47 - (6*i + (j+1))));
		}
		//cout << row << ", " << col << endl;
		
		
		bitset<4> newVal(S[i][(int)row.to_ulong()][(int)col.to_ulong()]);
		//cout << newVal << endl;
		
		for(int j = 0; j < 4; j++)
		{
			sOut.set(31 - (4 * i + j), newVal.test(3-j));
		}
	}
	
	return(sOut);
}

bitset<48> keysched(int round, bitset<56> inkey)
{
	// Split key in half
	bitset<28> C = 0;
	bitset<28> D = 0;
	for(int i = 0; i < 28; i++)
	{
		C.set(i, inkey.test(i+28));
		D.set(i, inkey.test(i));
	}
	
	// Left circular shift each half
	for(int i = 0; i < round+1; i++)
	{
		C = C << LCS[i] | C >> (28 - LCS[i]);
		D = D << LCS[i] | D >> (28 - LCS[i]);
	}
	
	// Recombine both sides of the roundkey
	inkey = bitset<56>(C.to_string() + D.to_string());
	
	// Apply PC-2 to key
	bitset<48> subkey = 0;
	for(int i = 0; i < 48; i++)
	{
		subkey.set(47-i, inkey.test(56 - PC2[i]));
	}
	
	return(subkey);
}

bitset<32> feistel(bitset<48> subkey, bitset<32> rinput)
{
	// Apply E-bit selection table to right half of input
	bitset<48> eOut;
	for(int i = 0; i < 48; i++)
	{
		eOut.set(47-i, rinput.test(32-E[i]));
	}
	
	// XOR with Sub-Key
	eOut ^= subkey;
	
	// Apply S-boxes
	bitset<32> sOut = sbox(eOut);
	
	// Apply permutation matrix P
	bitset<32> pOut;
	for(int i = 0; i < 32; i++)
	{
		pOut.set(31 - i, sOut.test(32 - P[i]));
	}
	
	return(pOut);
}