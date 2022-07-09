
/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "leij",
    /* First member's email address */
    "jianglei888899@mail.nwpu.edu.cn",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t))) // 8

/* Basic constants and macros */
#define WSIZE 4             /* word size (bytes) */
#define DSIZE 8             /* doubleword size (bytes) */
#define CHUNKSIZE (1 << 12) /* extend heap size (bytes) */
#define OVERHEAD 8          /* overhead of header and footer (bytes) */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(size_t *)(p))
#define PUT(p, val) (*(size_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

static char* heap_listp; // 堆头
static long long list;
static long long t;
static void *head = &list;
static void *tail = &t;
static char *find_fit_ptr;

#define PREV(bp) ((char *)(bp) )  // prev 在第一个位置
#define SUCC(bp) ((char *)(bp)+ WSIZE)    // succ 在第二个位置
#define GETP(p) (*(char **)p)
#define GET_PREV(bp) (GETP(PREV(bp)))
#define GET_NEXT(bp) (GETP(SUCC(bp)))
#define PUTP(p, val) (*(char **)(p) = (char *)(val))

/* 在pos 之前 插入 node节点 */
#define insert_node(pos, node){ \
    GET_NEXT(GET_PREV(pos)) = (char *)(node); \
    GET_PREV(node) =  GET_PREV(pos); \
    GET_PREV(pos) = (char *)(node); \
    GET_NEXT(node) = (char *)(pos); \
}

/* 从空闲链表删除节点 */
#define del_node(pos){ \
    GET_NEXT(GET_PREV(pos)) = GET_NEXT(pos); \
    GET_PREV(GET_NEXT(pos)) = GET_PREV(pos); \
}

/* 加入空闲节点到空闲链表 */
#define add_node_to_head(anode) {\
    void * tmp = GET_NEXT(head);\
    insert_node(tmp, anode);\
}

/* $end mallocmacros */

/* 没有空闲空间时扩展约 words * DSIZE 个字节的堆空间 */
static void* extend_heap(size_t);
/* 回收空闲块时合并左右空闲块 */
static void* coalesce(void *);
static void* find_fit(size_t);
static void place(void* , size_t);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    printf("init...\n");
    GET_NEXT(head) = (char *)tail; //建立链表
    GET_PREV(tail) = (char *)head;
    if((heap_listp=mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                          /* alignment padding */
    PUT(heap_listp + WSIZE, PACK(OVERHEAD, 1));  /* prologue header */
    PUT(heap_listp + DSIZE, PACK(OVERHEAD, 1));  /* prologue footer */
    PUT(heap_listp + WSIZE + DSIZE, PACK(0, 1)); /* epilogue header */
    heap_listp += DSIZE;

    if (extend_heap(CHUNKSIZE / DSIZE) == NULL) return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;  //调整块大小
    size_t extendsize; // 没有适合的空闲块时，额外分配的堆空间大小
    char *bp;
    if (size == 0) return NULL;

    /* 调整块大小满足对其要求 */
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = ((size / DSIZE) % 2) ? ((size / DSIZE) + 1) * DSIZE : ((size / DSIZE)+2) * DSIZE;
    
    /* 查找空闲链表 */
    if ((bp = find_fit(asize)) != NULL){
        //printf("place01\n");
        place(bp, asize);
        return bp;
    }

    /* 没找到的话，申请额外空间 */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / DSIZE)) == NULL) return NULL;
    //printf("place02\n");
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr)); //获取当前块长度
    PUT(HDRP(ptr), PACK(size, 0));     //将头尾部 分配位设置为空闲
    PUT(FTRP(ptr), PACK(size, 0));

    coalesce(ptr); //处理合并左右空闲块
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

/* 扩展约 words * DSIZE 个字节的堆空间 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* 分配偶数单词以保持对齐方式 最小size是 16字节 即 2 * DSIZE*/
    size = (words % 2) ? (words + 1) * DSIZE : words * DSIZE;
    if ((bp = mem_sbrk(size)) == (void *)-1) return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* new epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/* 根据四种情况处理左右空闲块的合并 */
static void *coalesce(void *bp)
{
    if (GET_SIZE(HDRP(bp)) != GET_SIZE(FTRP(bp)))
        printf("Error: header does not match footer01\n");
    size_t prev_alloc = GET_ALLOC((char *)bp - DSIZE);
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    // 相邻前后块都已分配，将该块加入空闲链表
    if (prev_alloc && next_alloc) { /* Case 1 */
        add_node_to_head(bp);
    }
    // alloc, unallocated，右相邻块空闲，在空闲链表中删除右块，合并当前块后再加入空闲链表头部（头插法）
    else if (prev_alloc && !next_alloc) { /* Case 2 */
        del_node(NEXT_BLKP(bp));
        if (GET_SIZE(HDRP(NEXT_BLKP(bp))) != GET_SIZE(FTRP(NEXT_BLKP(bp))))
            printf("Error: header does not match footer02\n");
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        add_node_to_head(bp);
    }
    // unallocated, alloc，左相邻块是空闲，合并修改左块的大小，并重置指针
    else if (!prev_alloc && next_alloc) { /* Case 3 */
        if (GET_SIZE(HDRP(PREV_BLKP(bp))) != GET_SIZE(FTRP(PREV_BLKP(bp))))
            printf("Error: header does not match footer03\n");
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    // unallocated, unallocated, 左右相邻块都是空闲，合并修改左块的大小，并重置指针
    else { /* Case 4 */
        del_node(NEXT_BLKP(bp));
        if (GET_SIZE(HDRP(NEXT_BLKP(bp))) != GET_SIZE(FTRP(NEXT_BLKP(bp))))
            printf("Error: header does not match footer04\n");
        if (GET_SIZE(HDRP(PREV_BLKP(bp))) != GET_SIZE(FTRP(PREV_BLKP(bp))))
            printf("Error: header does not match footer05\n");
        size += GET_SIZE(FTRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    if (GET_SIZE(HDRP(bp)) != GET_SIZE(FTRP(bp)))
        printf("Error: header does not match footer after coalesce\n");

    return bp;
}

// 首次适应
void* find_fit(size_t size){ 
    find_fit_ptr = GET_NEXT(head);
    while(find_fit_ptr != tail ){
        if (GET_SIZE(HDRP(find_fit_ptr)) >= size)
            break;
        find_fit_ptr = GET_NEXT(find_fit_ptr);
        if (find_fit_ptr == tail)
            break;
        if (GET_SIZE(HDRP(find_fit_ptr)) != GET_SIZE(FTRP(find_fit_ptr)))
            printf("Error: header does not match footer in find_fit\n");
        else
            printf("gggggoooooooodddd\n");
    }
    if (find_fit_ptr == tail)
        return NULL;
    else
        return find_fit_ptr;
}

// 在ptr所指空闲块中分配size大小空间
void place(void* ptr, size_t size){
    if (GET_SIZE(HDRP(ptr)) != GET_SIZE(FTRP(ptr)))
        printf("Error: header does not match footer06\n");
    size_t asize = GET_SIZE(HDRP(ptr));
    if ((asize - size) < 4 * WSIZE){ // 不足以抽出其它空闲块
        PUT(HDRP(ptr), PACK(asize, 1)); // 设置分配位
        PUT(FTRP(ptr), PACK(asize, 1));
        del_node(ptr);               // 从空闲链表删除该节点
    }

    else{   // 可以抽出其它空闲块
        del_node(ptr);
        PUT(HDRP(ptr), PACK(size, 1)); 
        PUT(FTRP(ptr), PACK(size, 1));
        PUT(HDRP(NEXT_BLKP(ptr)), PACK(asize - size, 0)); 
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(asize - size, 0));
        add_node_to_head(NEXT_BLKP(ptr));
    }
    return ;
}


