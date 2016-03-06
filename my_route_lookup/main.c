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
#define  TBLLONG_SIZE 65536
int16_t *createTBL(void);

struct FIBRoute {
    uint32_t *prefix ;  /* Next in list */
    int *prefixLength;          /* IP address   */
    int *outInterface;      /* Bytes sent   */
};

int16_t formatTableEntry(int16_t value);

const char* toBinaryString(unsigned int num)
{
    static char buffer[32*sizeof(num)+1];
    char* pBuffer = &buffer[sizeof(buffer)-1];
    
    do *--pBuffer = '0' + (num & 1);
    while (num >>= 1);
    return pBuffer;
}

void insert(int16_t *TBL24, uint8_t *TBLLong, struct FIBRoute *fibLine );

void insertInLongTable(uint8_t  *TBL24, int16_t position, int16_t nextHop , int repeat);

int main(int argc, const char * argv[]) {
    // insert code here...
    print ("Starting Routing Algorithm");
    int16_t test =  2;
    test = formatTableEntry(2);
    
    if ( initializeIO(argv[1], argv[2]) == OK) {
        
        print("Intialize IO OK");
        
        int16_t *TBL24 = createTBL();
        uint8_t *TBLLong =  calloc(TBLLONG_SIZE, sizeof(uint8_t));
        
        uint32_t *prefix = calloc(1, sizeof(uint32_t));
        int *prefixLength = (int*)calloc(1,sizeof(int));
        int *outInterface = (int*)calloc(1,sizeof(int));
        
        
        // readFIBLine(prefix, length, outInterface);
        
        while ((REACHED_EOF != readFIBLine(prefix, prefixLength, outInterface)) ) {
            
            struct FIBRoute *fib = calloc(1,sizeof(struct FIBRoute));;
            
            fib->prefix = prefix;
            fib->prefixLength = prefixLength;
            fib->outInterface = outInterface;
            
            insert(TBL24, TBLLong, fib);
            
          //  printf("Inseting %u \n" , *prefix);
        }
        
        printf("\n\nfinally TBL24[0] = %hd, \n TBL24[ 8394499] = %hd \n TBL24[16777216] = %hd \n TBL24[4194304]= %hd\n",TBL24[0], TBL24[ 8394499],TBL24[ 8388608],TBL24[419430] );
        printf("\n TBL24[ 13287424] = %hd  \n TBL24[ 13078528] %hd \n  TBL24[ 11263999]  = %hd \n",TBL24[ 13287424],TBL24[ 13078528], TBL24[11263999]  );
        printf("stop \n");
    }
    
    return 0;
}


void addressRanges (int prefixLength, int *positionsToSave){
    
    *positionsToSave = pow(2, 24-prefixLength);
   // printf("Should set %d \n", *positionsToSave);
    
}


void insert(int16_t *TBL24, uint8_t *TBLLong, struct FIBRoute *fibLine ) {
    
    int *netMask = (int*)calloc(1,sizeof(int));
    int *range = (int*)calloc(1,sizeof(int));
    

    getNetmask(24, netMask);
    print(toBinaryString(*netMask));
    print(toBinaryString(*fibLine->prefix));
    
    addressRanges(*fibLine->prefixLength, range);
   
    uint32_t tmp = (*fibLine->prefix) & (*netMask);
    uint32_t tbl24Position = tmp >> 8;

    if (*fibLine->prefixLength < 24) {
        
       
        int repeat = *range;
        for (int i =0; i <repeat ; i++) {
            
            int tmp = tbl24Position +i;
            TBL24[ tmp] = *fibLine->outInterface;
            
//            printf(" TBL24[ %i] = %i \n",tmp,TBL24[tmp]);
        }
       
        
        printf("range = %i \n",repeat);
        printf(" TBL24[ %i] = %i \n",tbl24Position,TBL24[tbl24Position]);
        printf(" TBL24[ %i] = %i \n",(tbl24Position +repeat),TBL24[(tbl24Position +(repeat -1))]);
        
    }else {
        int d  = 0xFFFFFFFF >> 24;
        int offset = (*fibLine->prefix) & d;
      
        int negative  =formatTableEntry(offset);
        TBL24[tbl24Position] = negative; // se mete el valor en negativo, para indentificarlo luego
          printf(" TBL24[ %i] = %i \n",tbl24Position,TBL24[tbl24Position]);
        insertInLongTable(TBLLong , offset * 256, *fibLine->outInterface, 256-offset);
    }
    

}


void insertInLongTable(uint8_t *TBLLong, int16_t position, int16_t nextHop , int repeat){

    for (int i =0; i < repeat; i++) {
        
         TBLLong[abs(position)] = nextHop;
    }

}

int16_t formatTableEntry(int16_t value) {
    
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

