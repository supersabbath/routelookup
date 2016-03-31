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
    
    int processedPackets = 0;
    int totalTableAccess = 0;
    
    int status = initializeIO((char *)argv[1],(char *) argv[2]) ;

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
       double averageTime, sec, usec = 0.0;
        while ((REACHED_EOF != readInputPacketFileLine(ip) ) )
        {
            int *memAccesses = calloc(1, sizeof(int));
            gettimeofday(&start, NULL);
            int outInterface = lookup(*ip,TBL24,TBLLong,memAccesses);
            gettimeofday(&end, NULL);
            double *search = calloc(1, sizeof(double));
            printOutputLine(*ip, outInterface, &start, &end, search, *memAccesses);
            //printf("Total time %f \n",*search);
            processedPackets++;
            totalTableAccess += *memAccesses;
            usec = end.tv_usec - start.tv_usec;
            if (usec > end.tv_usec){
                start.tv_sec += 1;
            }
            sec = end.tv_sec - start.tv_sec;
            
            averageTime += 10000*sec + usec;
            free(memAccesses);
            free(search);
        }
        free(ip);
        /// Impresion de resultados
        double averageAccesses =  totalTableAccess / processedPackets;
        double averageProcessing = averageTime / processedPackets;
        printSummary(processedPackets, averageAccesses, averageProcessing);
        print("Ver resultados");
    
    }else {
        printIOExplanationError(status);
    }

    
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

    
    if (*fibLine->prefixLength < 24) {
        
        int repeat = *range;
        int i;
        for ( i =0; i <repeat ; i++) {
            
            int tmp = tbl24Position +i;
            
            TBL24[ tmp] = *fibLine->outInterface;

        }
        
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

    struct IpRoute *tmp, *head = &TBLLon[position];
    
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
        print("Mem Reservada para TBL24");

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
        *memoryAccesses =1;
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



