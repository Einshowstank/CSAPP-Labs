## 本实验包含2部分，Part A 写一个缓存模拟器；Part B 写一个矩阵转置函数以最优化缓存性能
## Part A
文件在 csim.c;

关键点：

定义一个二维数组，元素为CacheLine结构体，包括三个整数：有效位，标记位和时间戳（用于LRU算法）。

其中二维数组行数代表组数S，列数代表E;

参考pdf文件，我们只需要关心S(save); L(load); M(modified)三种指令，其中S, L都算一次更新，L算2次更新。

更新函数逻辑：

首先提取出64位地址的块号和标记，根据块号遍历二维数组特定的行，遍历记录是否命中，是否有空闲cache_line，最大时间戳id，
同时对非空闲的cache_line增加一次时间戳。

如果命中了，增加命中数Hits，该cache_line时间戳设置为0;

如果没命中，则选择插入该组空闲位置或换出时间戳最大的cache_line。同时更新时间戳，有效位，标记位。

## Part B
优化矩阵转置函数，以尽量减小缓存miss次数。

原函数：
```
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    
}
```

简单思路：

针对1kb的直接映射缓存，块大小为32字节，即一个cache_line可以放下8个int整数，总共32组cache_line。

以**32×32**大小矩阵为例，按照普通的顺序转置函数，矩阵A的缓存命中率约为 7/8 （不考虑矩阵B覆盖的情况）；矩阵B缓存命中率为0。
### 针对32×32矩阵优化代码

为尽量增加矩阵B的缓存命中率，一个简单思路如下：

cache最多可容纳A矩阵的1行和B矩阵的7行（每行32个整数各占4组cache_line）；假设保存的B矩阵元素为：
```
B[9][0] ~ B[9][7]
B[10][0] ~ B[10][7]
B[11][0] ~ B[11][7]
...
B[15][0] ~ B[15][7]
```
为了尽量保存以上元素在cache里，可按照以下顺序(8×8小块滑窗访问)访问矩阵A：
```
A[0][8] ~ A[0][15]
A[1][8] ~ A[1][15]
A[3][8] ~ A[3][15]
...
A[7][8] ~ A[7][15]
```
针对32×32矩阵优化代码：
```
void transpose_submit(int M, int N, int A[N][M], int B[M][N]){
    int i, j, t1, t2, t3, t4, t5, t6, t7, t8;
    for(i = 0; i < N-7; i += 8)
        for(int k = 0; k < M-7; k += 8)
            for(j = 0; j < 8; ++j){
                t1 = A[i+j][k+0];
                t2 = A[i+j][k+1];
                t3 = A[i+j][k+2];
                t4 = A[i+j][k+3];
                t5 = A[i+j][k+4];
                t6 = A[i+j][k+5];
                t7 = A[i+j][k+6];
                t8 = A[i+j][k+7];

                B[k+0][i+j] = t1;
                B[k+1][i+j] = t2;
                B[k+2][i+j] = t3;
                B[k+3][i+j] = t4;
                B[k+4][i+j] = t5;
                B[k+5][i+j] = t6;
                B[k+6][i+j] = t7;
                B[k+7][i+j] = t8;
            }
}   
```
这里用多个局部变量而不是循环，以防止A和B读取时互相覆盖。
miss次数从1183减小到287：
```
[root@b07618077bfd cachelab_copy]# ./test-trans -M 32 -N 32

Function 0 (2 total)
Step 1: Validating and generating memory traces
Step 2: Evaluating performance (s=5, E=1, b=5)
func 0 (Transpose submission): hits:1766, misses:287, evictions:255

Function 1 (2 total)
Step 1: Validating and generating memory traces
Step 2: Evaluating performance (s=5, E=1, b=5)
func 1 (Simple row-wise scan transpose): hits:870, misses:1183, evictions:1151

Summary for official submission (func 0): correctness=1 misses=287
```

### 针对64×64矩阵优化代码
思路同上，对于64×64矩阵，cache最多可容纳A矩阵的1行和B矩阵的3行，这里假设不考虑A和B互相覆盖的情况，在原优化代码中，矩阵A的访问顺序为：
```
A[0][0] ~ A[0][7]
...
A[7][0] ~ A[7][7]
```
优化后访问顺序应为：
```
A[0][0] ~ A[0][3]
...
A[3][0] ~ A[3][3]

A[0][4] ~ A[0][7]
...
A[3][4] ~ A[3][7]

A[4][0] ~ A[4][3]
...
A[7][0] ~ A[7][3]

A[4][4] ~ A[4][7]
...
A[7][4] ~ A[7][7]
```
以4×4小块滑窗访问，对应优化代码如下：
```
void transpose_submit(int M, int N, int A[N][M], int B[M][N]){
    int i, j, t1, t2, t3, t4;
    for(i = 0; i < N-7; i += 8)
        for(int k = 0; k < M-7; k += 8){
            for(j = 0; j < 8; ++j){ //对8×8的小块，先处理左上和左下的4×4子块
                t1 = A[i+j][k+0];
                t2 = A[i+j][k+1];
                t3 = A[i+j][k+2];
                t4 = A[i+j][k+3];

                B[k+0][i+j] = t1;
                B[k+1][i+j] = t2;
                B[k+2][i+j] = t3;
                B[k+3][i+j] = t4;
            }
            for(j = 0; j < 8; ++j){ //对8×8的小块，再处理右上和右下的4×4子块
                t1 = A[i+j][k+4];
                t2 = A[i+j][k+5];
                t3 = A[i+j][k+6];
                t4 = A[i+j][k+7];

                B[k+4][i+j] = t1;
                B[k+5][i+j] = t2;
                B[k+6][i+j] = t3;
                B[k+7][i+j] = t4;
            }
        }
} 
```

针对32×32矩阵，miss从1183减为319；针对64×64矩阵，miss从4723减为1651:
```
Function 0 (2 total)
Step 1: Validating and generating memory traces
Step 2: Evaluating performance (s=5, E=1, b=5)
func 0 (Transpose submission): hits:1734, misses:319, evictions:287

Function 1 (2 total)
Step 1: Validating and generating memory traces
Step 2: Evaluating performance (s=5, E=1, b=5)
func 1 (Simple row-wise scan transpose): hits:870, misses:1183, evictions:1151
```

```
Function 0 (2 total)
Step 1: Validating and generating memory traces
Step 2: Evaluating performance (s=5, E=1, b=5)
func 0 (Transpose submission): hits:6546, misses:1651, evictions:1619

Function 1 (2 total)
Step 1: Validating and generating memory traces
Step 2: Evaluating performance (s=5, E=1, b=5)
func 1 (Simple row-wise scan transpose): hits:3474, misses:4723, evictions:4691
```

### 64×64进阶
思路：1）对于8×8窗口，先反转A左上4×4元素到B，同时保存A右上4×4元素到B右上；

2）反转A左下4×4元素到B右上，同时将保存在B右上的4×4元素反转到B右下；

3）反转A右下4×4元素到B。注意执行第2步时，每次循环内对A按列读取，对B按行读取：

```
void transpose_submit(int M, int N, int A[N][M], int B[M][N]){
    int i, j, t1, t2, t3, t4;
    for(i = 0; i < N-7; i += 8)
        for(int k = 0; k < M-7; k += 8){
            for(j = 0; j < 4; ++j){ // 对8×8小窗口，先反转A左上到B，再保存右上到B
                t1 = A[i+j][k+0];
                t2 = A[i+j][k+1];
                t3 = A[i+j][k+2];
                t4 = A[i+j][k+3];
                t5 = A[i+j][k+4];
                t6 = A[i+j][k+5];
                t7 = A[i+j][k+6];
                t8 = A[i+j][k+7];

                B[k+0][i+j] = t1; // 保存左上4×4窗口对应值
                B[k+1][i+j] = t2;
                B[k+2][i+j] = t3;
                B[k+3][i+j] = t4;
                B[k+0][i+j+4] = t5; // 后4列缓存右上4×4窗口对应值
                B[k+1][i+j+4] = t6;
                B[k+2][i+j+4] = t7;
                B[k+3][i+j+4] = t8;
            }
            /* 反转A左下4×4窗口的值到B，同时将上一步保存的右上4×4窗口对应值反转到B左下
                    注意对矩阵A每次循环按行读取，B按列读取 */
            for(j = 0; j < 4; ++j){ 
                t1 = A[i+4][k+j];
                t2 = A[i+5][k+j];
                t3 = A[i+6][k+j];
                t4 = A[i+7][k+j];
                t5 = B[k+j][i+4]; // 上一步保存在B中的值
                t6 = B[k+j][i+5];
                t7 = B[k+j][i+6];
                t8 = B[k+j][i+7];

                B[k+j][i+4] = t1; // 保存反转的左下4×4窗口对应值
                B[k+j][i+5] = t2;
                B[k+j][i+6] = t3;
                B[k+j][i+7] = t4;
                B[k+4+j][i+0] = t5; // 保存反转的右上4×4窗口对应值
                B[k+4+j][i+1] = t6;
                B[k+4+j][i+2] = t7;
                B[k+4+j][i+3] = t8;
            }
            for(j = 4; j < 8; ++j){ // 反转A右下4×4窗口的值到B
                t1 = A[i+j][k+4];
                t2 = A[i+j][k+5];
                t3 = A[i+j][k+6];
                t4 = A[i+j][k+7];

                B[k+4][i+j] = t1;
                B[k+5][i+j] = t2;
                B[k+6][i+j] = t3;
                B[k+7][i+j] = t4;
            }
        }
} 
```

测试结果，miss数减小到1179：
```
Function 0 (2 total)
Step 1: Validating and generating memory traces
Step 2: Evaluating performance (s=5, E=1, b=5)
func 0 (Transpose submission): hits:9066, misses:1179, evictions:1147
```
