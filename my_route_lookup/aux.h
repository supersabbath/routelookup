#include <stdint.h>

/********************************************************************
 * Constant definitions 
 ********************************************************************/
#define IP_ADDRESS_LENGTH 32


/********************************************************************
 * Generate a netmask of length prefixLength
 ********************************************************************/
void getNetmask (int prefixLength, int *netmask);

/********************************************************************
 * Example of a very simple hash function using the modulus operator
 * For more info: https://gist.github.com/cpq/8598442
 ********************************************************************/
int hash(uint32_t IPAddress, int sizeHashTable);
/* Helper fx to debug binary ips*/
const char* toBinaryString(unsigned int num);
/* Prints an inter in IP format 10.0.0.0 */
void print_ip(int ip);