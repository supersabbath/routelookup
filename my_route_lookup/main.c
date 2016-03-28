//
//  main.c
//  my_route_lookup
//
//  Created by Fernando Canon on 24/02/16.
//  Copyright © 2016 Aphabit. All rights reserved.
//

#include <stdio.h>
#include "io.h"
#include "aux.h"

#define print(...) fprintf(stderr, "[Route-Lookup : Group-16] :%s \n", __VA_ARGS__)

#define  TBL24_SIZE 16777216
#define  TBLLONG_SIZE 32767
#define NO_INTERFACE 0

struct IpRoute {
    struct IpRoute *nextRoute;  /* Next in list */
    uint32_t ip;          /* IP address   */
    uint16_t  outputInterface;      /* Bytes sent   */
};

struct IpRoute* ipRoute(uint32_t ip, uint16_t  nextHop)
{
    struct IpRoute *p = malloc(sizeof(struct IpRoute));
    p->ip = ip;
    p->outputInterface = nextHop;
    return p;
}// Now you returning the object what p is pointing to.

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

/******************* INSERTION ***************************
    Tables creation
 *********************************************************/

int16_t *createTBL(void);

struct FIBRoute {
    uint32_t *prefix ;  /* IP address */
    int *prefixLength;          /* Prefijo CIRD   */
    int *outInterface;      /* Router output interface   */
};

/*
 flip the first bit to 1, example 128 returns -128, 0 returns -1
 */
int formatTableEntry(int  value);

void insert(int16_t *TBL24, struct IpRoute *TBLLong, struct FIBRoute *fibLine );

void insertInLongTable(struct IpRoute *TBLLon, uint16_t position, struct FIBRoute *fibLine);

/******************* SEARCH ******************************
    SEARCH ALGORITHM
 *********************************************************/
/*
    lookup()
    @param TBL24 . pointer to an array array[2^24] of int16_t
    @param TBLLong pointer to an array[2^20] of  struct IpRoute
    @returns the output interface associated to an ip .
 */

int lookup(uint32_t ip,int16_t *TBL24 , struct IpRoute *TBLLong,int *memoryAccesses);

int main(int argc, const char * argv[]) {
    // insert code here...
    print ("Starting Routing Algorithm");
  
    int status = initializeIO(argv[1], argv[2]) ;

    if (status == OK) {
        
        print("Intialize IO OK");
        
        int16_t *TBL24 = createTBL();
        // Creates the secondary table
        struct IpRoute *TBLLong =  calloc(TBLLONG_SIZE, sizeof(struct IpRoute));
        //
        uint32_t *prefix = calloc(1, sizeof(uint32_t));
        int *prefixLength = (int*)calloc(1,sizeof(int));
        int *outInterface = (int*)calloc(1,sizeof(int));
      
        
        while ((REACHED_EOF != readFIBLine(prefix, prefixLength, outInterface)) ) {
            
            struct FIBRoute *fib = calloc(1,sizeof(struct FIBRoute));;
        
            fib->prefix = prefix;
            fib->prefixLength = prefixLength;
            fib->outInterface = outInterface;
            //printf("\n\n ***************  Inserting : %u ***************\n", *fib->prefix);

            //printf ( "  length: %u\n",*fib->prefixLength);
            //printf ( "  outInterface: %u\n",*fib->outInterface);
            insert(TBL24, TBLLong, fib);
            
            free(fib);

        }
        free(prefix);
        free(outInterface);
        free(prefixLength);
        //printf("Build of table routing Done!");
        
        uint32_t *ip = calloc(1, sizeof(uint32_t));
       struct timeval start, end;
    
        while ((REACHED_EOF != readInputPacketFileLine(ip) ) )
        {
            gettimeofday(&start, NULL);
            int *memAccesses = calloc(1, sizeof(int));
            int outInterface = lookup(*ip,TBL24,TBLLong,memAccesses);
            gettimeofday(&end, NULL);
            double *search = calloc(1, sizeof(double));
            printOutputLine(*ip, outInterface, &start, &end, search, *memAccesses);
            //printf("Total time %f \n",*search);
        }

    }else {
        printIOExplanationError(status);
    }
    /// Busqueda
     printMemoryTimeUsage();
    
    return 0;
}


void addressRanges (int prefixLength, int *positionsToSave){
    
    *positionsToSave = pow(2, 24-prefixLength);
   // //printf("Should set %d \n", *positionsToSave);
    
}


void insert(int16_t *TBL24, struct IpRoute *TBLLong, struct FIBRoute *fibLine ) {
    
    int *netMask = (int*)calloc(1,sizeof(int));
    int *range = (int*)calloc(1,sizeof(int));
    

    getNetmask(24, netMask);
    addressRanges(*fibLine->prefixLength, range);
   
    uint32_t tmp = (*fibLine->prefix) & (*netMask);
    uint32_t tbl24Position = tmp >> 8;
//    int test = 3401580560; // 134.24.124.76/30	24203

    
//    if (*fibLine->prefix == test) {
//        print_ip(test);
//        int hastPosition = hash(*fibLine->prefix,TBL24_SIZE);
//        
//        printf("\n \n hash %i, calculatet %i \n",hastPosition,tbl24Position);
//    }
    
    if (*fibLine->prefixLength < 24) {
        
        int repeat = *range;
        int i;
        for ( i =0; i <repeat ; i++) {
            
            int tmp = tbl24Position +i;
            
            TBL24[ tmp] = *fibLine->outInterface;

        }
 
//        printf("*************** TBL24 *************\n");
//        printf("        range = %i \n",repeat);
//        printf("        TBL24[ %i] = %i \n",tbl24Position,TBL24[tbl24Position]);
//        printf("        TBL24[ %i] = %i \n",(tbl24Position +(repeat-1)),TBL24[(tbl24Position +(repeat -1))]);
        
    }else {
       // printf("*************** TBLLong *************\n");
        int hastPosition = hash(*fibLine->prefix,TBLLONG_SIZE);
        int negative = formatTableEntry(hastPosition);
        TBL24[tbl24Position] = negative;
        
        insertInLongTable(TBLLong , hastPosition, fibLine);
    }
   
    free(netMask);
    free(range);
    
}


void insertInLongTable(struct IpRoute *TBLLon, uint16_t position, struct FIBRoute *fibLine){
   
    
    int d  = 0x000000FF;
 
    int lessImportantBits = *fibLine->prefix & d;

    struct IpRoute *tmp, *head = &TBLLon[abs(position)];
    
    if (head->nextRoute == NULL) {
        
        tmp = head;
        tmp->ip = *fibLine->prefix;
        tmp->outputInterface = *fibLine->outInterface;
        
        TBLLon[position] = *tmp;
        int i;
        for ( i=1; i < 255; i++) {
            struct IpRoute *next = ipRoute(*fibLine->prefix, *fibLine->outInterface);
            
            tmp->nextRoute = next;
            tmp = next;
        }
    } else {
        
        
        int counter = 0;
        for (tmp = head; tmp != NULL; tmp = tmp->nextRoute) {
            counter++;
            if (counter > 255) {
                printf("Problems");
            }else if (counter >= lessImportantBits) {
                    
                tmp->ip = *fibLine->prefix;
                tmp->outputInterface = *fibLine->outInterface;
            }
            
        }
        
    }

}



int formatTableEntry(int  value) {
    
  
    if (value == 0){
        return ~value;
    }
    return -1*value;
    
}


int16_t *createTBL(void){
    
    int16_t *TBL = NULL;
    
    if( (TBL = calloc(TBL24_SIZE, sizeof(int16_t))) == NULL){
        print("No hay suficiente memoria");
    }
    else{
        print("Funciona");
        //        TBL[0] = 22;
        //        TBL[1]= 1;
        //        TBL[SIZE_TBL-1] = 2;
    }
    
    return TBL;
}
/***************************** SEARCH *********************/

int lookup(uint32_t ip,int16_t *TBL24 , struct IpRoute *TBLLong,int *memoryAccesses)
{
    int *netMask = (int*)calloc(1,sizeof(int));
    
  //  print(toBinaryString(ip));
    
    getNetmask(24, netMask);
    uint32_t tmp = ip & (*netMask);
    uint32_t tbl24Position = tmp >> 8;
    
  //  print(toBinaryString(*netMask));
    
    int nextHop = TBL24[tbl24Position];
   // print_ip(ip);
   // printf(" \n output for %i \n", nextHop);
    
    if (nextHop < 0) {
        
        int d  = 0x000000FF;
        int lessImportantBits = ip & d;
       // print_ip(lessImportantBits);
        
      
        struct IpRoute *p , *head  = &TBLLong[abs(nextHop)];
        uint8_t counter = 0;
        
        for (p = head; p != NULL; p = p->nextRoute) {
            counter++;
            *memoryAccesses = counter;
            if (counter == lessImportantBits){
                
                return p->outputInterface;
            }
        }
        
        
    }else{
        *memoryAccesses =1;
        return  nextHop;
    }

    return NO_INTERFACE;
}



