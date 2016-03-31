#include "aux.h"

/********************************************************************
 * Generate a netmask of length prefixLength
 ********************************************************************/
void getNetmask (int prefixLength, int *netmask){

	*netmask = (0xFFFFFFFF << (IP_ADDRESS_LENGTH - prefixLength)); 

}

/********************************************************************
 * Example of a very simple hash function using the modulus operator
 * For more info: https://gist.github.com/cpq/8598442
 ********************************************************************/
int hash(uint32_t IPAddress, int sizeHashTable){
	
	//Map the key (IPAddress) to the appropriate index of the hash table
  int index = IPAddress % sizeHashTable; 
  return (index);

}


/**
 Helper function that allows printing in binary format
 
 */
const char* toBinaryString(unsigned int num)
{
    static char buffer[32*sizeof(num)+1];
    char* pBuffer = &buffer[sizeof(buffer)-1];
    
    do *--pBuffer = '0' + (num & 1);
    while (num >>= 1);
    return pBuffer;
}

void print_ip(int ip)
{
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;
    printf("%d.%d.%d.%d\n", bytes[3], bytes[2], bytes[1], bytes[0]);
}
