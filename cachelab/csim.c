
# include <stdio.h>
# include <stdint.h>
# include <stdbool.h>
# include <math.h>
# include <stdlib.h>
# include "cachelab.h"

char* path;
char str[1024];
int Hits = 0;
int Misses = 0; 
int Evics = 0;  //换出次数

typedef struct CacheStruct {
    int valid; //有效位
    int tag;   //标记位
    int stamp; //时间戳，用于LRU算法
}CacheLine;
CacheLine **Cache= NULL; //cache定义为二维数组结构体

void InitCache(int S,int E){
    Cache = ( CacheLine**)malloc(sizeof(CacheLine*) * S);   //分配S组
    for(int i=0;i<S;i++){
        Cache[i]=( CacheLine*)malloc(sizeof( CacheLine)*E); //每组分配E行
        for(int j=0;j<E;j++){
            Cache[i][j].tag=-1;
            Cache[i][j].stamp=-1;
            Cache[i][j].valid=0;
        }
    }
}
void FreeCache(int S){
    for(int i=0;i<S;i++)free(Cache[i]);
    free(Cache);
}
void update(uint64_t address, int s, int E, int b){ // 地址组成:标记位 | 组号 | 块偏移
    int index = (address >> b) & (-1U >> (64-s)); // 块号，代表在第几组
    int this_tag = (address >> (b+s));     // 标记
    bool find_cache = false;               // 是否命中
    bool is_full = true;                   // 没命中的话该组cache_line是否满了
    int invalid_id, max_stamp_id;
    int max_stamp = 0;

    for (int i = 0; i < E; ++i){ // 遍历记录是否命中，是否有空闲cache_line，最大时间戳id
        if (Cache[index][i].tag == this_tag && Cache[index][i].valid == 1){ //命中了，加Hits，时间戳设置为0
            find_cache = true;
            ++Hits;
            Cache[index][i].stamp = 0;
        }
        if (Cache[index][i].valid == 0){
            is_full = false;
            invalid_id = i;
        }
        else
            ++Cache[index][i].stamp;
        if(Cache[index][i].stamp >= max_stamp){
            max_stamp = Cache[index][i].stamp;
            max_stamp_id = i;
        }
    }
    if(!find_cache){ //没命中
        ++Misses;
        if (is_full){  //满了则根据max_stamp_id换出
            Cache[index][max_stamp_id].stamp = 0;
            Cache[index][max_stamp_id].valid = 1;
            Cache[index][max_stamp_id].tag = this_tag;
            ++Evics;
        }
        else{ // 否则放入空闲位置
            Cache[index][invalid_id].stamp = 0;
            Cache[index][invalid_id].valid = 1;
            Cache[index][invalid_id].tag = this_tag;
        }
    }
    return;
}


int main(int argc, char *argv[])
{
    int s,E,b, size;
    char op;
    uint64_t address;//address
    FILE *fp;
    for (int i = 1; i < argc; ++i){ //根据命令行参数设置初始值
        if (argv[i][0] == '-'){
            switch(argv[i][1]){
                case 's':
                    s = argv[++i][0] - '0';
                    break;
                case 'E':
                    E = argv[++i][0] - '0';
                    break;
                case 'b':
                    b = argv[++i][0] - '0';
                    break;
                case 't':
                    path = argv[++i];
                    break;
                default:
                    break;
            }
        }
    }
    int S = pow(2, s);
    InitCache(S, E);

    fp=fopen(path,"r");
    while( fgets(str, 100, fp) != NULL ) {
        sscanf(str, " %c %lx,%d", &op, &address, &size);//处理我们读入的每一行每一列。
        switch (op) {
            case 'I':
                continue;
            case 'L':case 'S':
                update(address, s, E, b);
                break;
            case 'M':
                update(address, s, E, b);
                update(address, s, E, b);
                break;
        }
        }

    FreeCache(S);
    fclose(fp);
    printSummary(Hits, Misses, Evics);
    return 0;
}
