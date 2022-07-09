/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]){
    int i, j, t1, t2, t3, t4, t5, t6, t7, t8;
    if (M == 32){
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
    else if (M == 64){
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
                /* 反转A左下4×4窗口的值到B，同时将上一步保存的右上4×4窗口对应值反转到B 
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
    
} 

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
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

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

