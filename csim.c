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

int ** record = NULL;

char * buf = NULL;

char * code = NULL;

char *** bptr = NULL;

char * temp_ch = NULL;

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

void myparse(){
    if (buf[0] == 'I'){
            cmd = 'I';
            return;
    }

    else{
        cmd = buf[1];
        int i = 3, addr_len = 0, temp = 0, tail = 1, bit = 1;
        while (buf[i] != ','){ 
            addr_len += 1, i += 1;
        } 
        size = atoi(buf + i + 1);
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
                    
                }
            i -= 1;
        }
        while (tail < 64)
        {
            code[63 - tail] = '0';//code form "00...00_addr__set_code__block_code__"
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

        strncpy(temp_ch, code, 63 - block_bit - set_bit);
    } 
}

void init(){
    buf = (char *) calloc(32, sizeof(char));
    code = (char *) calloc(64, sizeof(char));
    bptr = (char ***) calloc(set, sizeof(char **));
    record = (int **) calloc(set, sizeof(int*));
    temp_ch = (char *) calloc(64, sizeof(char));
    for(int i = 0; i < set; i++){   
        record[i] = (int *)calloc(lps, sizeof(int)); 
    }
    for(int i = 0; i < set; i++)
        bptr[i] = (char **) calloc(lps, sizeof(char *));
    for(int i = 0; i < set; i++)
        for(int j = 0; j < lps; j++)
        {
            bptr[i][j] = (char *) calloc(64, sizeof(char));
            bptr[i][j][0] = '0';//set the effective flag to zero             
        }
    
}

void load()
{
    int temp_order = 0;
    int temp_val = 0;
    bool init = 0;
    for(int i = 0; i < lps; i++)
    {
        if((bptr[set_num][i][0] == '0'))
        {
            strcpy(bptr[set_num][i] + 1, temp_ch);
            record[set_num][i] += 1;
            miss_count += 1;
            
            bptr[set_num][i][0] = '1';
            if(ver)
                printf(" : miss\n");
            return;
        }

        else
        {
            if(!init)
            {
                temp_val = record[set_num][0];
                init = 1;
            }
            
            
            if(strcmp(temp_ch, (bptr[set_num][i] + 1)) == 0)
            {
                record[set_num][i] += 1;
                hit_count += 1;
                if(ver)
                    printf(" : hit\n");
                return;
            }
            else
            {

                if(record[set_num][i] < temp_val)
                {
                    temp_val = record[set_num][i];
                    temp_order = i;
                }
            }

        }
    }
    strcpy((bptr[set_num][temp_order] + 1), temp_ch);
    record[set_num][temp_order] += 1;
    eviction_count += 1;
    miss_count += 1;
    if(ver)
    {
        printf(" : miss\n");
        printf(" : eviction\n");
    }
    return;
}
//wire through && not-write-allocate
void store()
{
    int i;
    for(i = 0; i < lps; i++)
    {
        if(bptr[set_num][i][0] == '0')
        {
            if(ver)
                printf(" : miss\n");
            record[set_num][i] += 1;
            miss_count += 1;
            bptr[set_num][i][0] = '1';
            strcpy(bptr[set_num][i] + 1, temp_ch);
            return;
        }
        else
        {
            if((strcmp(temp_ch, bptr[set_num][i] + 1)) == 0)
            {
                if(ver)
                    printf(" : hit\n");
                record[set_num][i] += 1;
                hit_count += 1;
                return;
            }
        }
    }
    if(ver)
        printf(" : hit\n");
    hit_count += 1;       
    return;
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
    FILE * fp;
    char ch;
	while ((op = getopt(argc, argv, "-hvs:E:b:t:")) != -1){
        switch(op){
            case 'h':
                fp = fopen("./help.txt", "r");
                ch = fgetc(fp);
                while (!feof(fp)){
                    putchar(ch);
                    ch = fgetc(fp);
                }
                fclose(fp);   
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
                                printf("%s", buf);
                            myparse(); 
                            judge();
                    } 
                }
                fclose(tracptr);
                    
        }
	}
    free(buf);    
    free(code);
    free(record);
    free(bptr);
    free(temp_ch); 
	printSummary(hit_count, miss_count, eviction_count);
	return 0;	
} 

