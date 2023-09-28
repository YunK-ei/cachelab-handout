#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stddef.h>
#include <math.h>
#include <assert.h>

FILE * tracptr;

char * buf = NULL;

char * code = NULL;

char *** bptr = NULL;

//cmd_bit of instructions
char cmd;

//verbose flag
bool ver = 0;

//some numbers
int block_bit = 0;

int block = 0;

int lps = 0;

int set_bit = 0;

int set = 0;

int size = 0;

int hit_count = 0;

int miss_count = 0;

int eviction_count = 0;

//code the last block_bit of code to block_code
int block_num = 0 ;

//code the set_bit of code to set_code
int set_num = 0;

void parse(){
   
    
    if (buf[0] == 'I'){
            cmd = 'I';
            return;
    }

    else{
        cmd = buf[1];
        int i = 2, addr_len = 0, temp = 0, tail = 1, bit = 1;
        while (buf[i] != ','){ 
            addr_len += 1, i += 1;
        } 
        size = atoi(buf[i+1]);
        addr_len = addr_len * 4;
        assert(addr_len < 64);
        i -= 1;
        //hex2binary
        while (buf[i] != ' '){
            temp = (int) (buf[i] - '0');
            for(int k = 0; k < 4; ++k)
                {
                    code[63 - tail] = (char) ('0' + temp % 2);
                    temp = temp / 2;
                    tail += 1;
                    k += 1;
                }
            i += 1;
        }
        while (tail < 64)
        {
            code[63 - tail] = '0';
            tail += 1;
        }
        block_num = 0;
        set_num = 0;

        for(int k = 0; k < block_bit; k++){
            block_num += (int) (code[62 - k] - '0') * bit;
            bit *= 2;
        }
        bit = 1;
        for(int k = 0; k < set_bit; k++){
            set_num += (int) (code[62 - block_bit - k] - '0') * bit;
            bit *= 2;
        }
    } 
}

void init(){
    buf = (char *) malloc(32);
    code = (char *) malloc(64);
    bptr = (char ***) malloc(sizeof(char **) * set);
    for(int i = 0; i < set; i++)
        bptr[i] = (char **) malloc(sizeof(char *) * lps);
    for(int i = 0; i < set; i++)
        for(int j = 0; j < lps; j++)
            bptr[i][j] = (char *) malloc(64);
    
}

void judge(){                
    switch(cmd){
        case 'I':
            return;
        case 'L':
            load();
            break;
        case 'S':
            store();
            break;
        case 'M':
            load();
            store();
            break;
    }
}

int main(int argc, char *argv[])
{
	int op; 
	while ((op = getopt(argc, argv, "-hvs:E:b:t:")) != -1){
        switch(op){
            case 'h':
                FILE * fp = fopen("./help.txt", "r");
                char ch = fgetc(fp);
                while (!feof(fp)){
                    putchar(ch);
                    ch = fgetc(fp);
                }
                fclose(fp);
                getchar();
                return 0;
            case 'v':
                ver = 1;
                break;
            case 's':
                set_bit = atoi(optarg);
                set = pow(2, set_bit);
                break;
            case 'E':
                lps = atoi(optarg);
                break;
            case 'b':
                block_bit = atoi(optarg);
                block = pow(2, block_bit);
                break;
            case 't':
                tracptr = fopen(optarg, "r"); 
                init(); 
                while (!feof(tracptr)){
                    if(fgets(buf, 63, tracptr)){
                            if(ver)
                                printf("%s//", buf);
                            parse(); 
                            judge();
                    }
                   
                }
                    
        }
	}
    free(buf);    
    printf("%d, %d, %d", set, lps, block);
	printSummary(hit_count, miss_count, eviction_count);
	return 0;	
}









   

