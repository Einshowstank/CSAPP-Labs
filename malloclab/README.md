报错：segment fault，gdb调试信息：
```
Breakpoint 2, coalesce (bp=0xf69e0020) at mm.c:230
230             add_node_to_head(bp);
(gdb) p *(char **)((char *)head+4)
$20 = 0x804f568 <t> "`\365\004\b"
(gdb) p *(char **)((char *)tail)
$21 = 0x804f560 <list> ""
(gdb) p *(char **)((char *)bp)
$22 = 0x0
(gdb) p *(char **)((char *)bp+4)
$23 = 0x0
(gdb) step
264         return bp;
(gdb) p *(char **)((char *)bp+4)
$24 = 0xf69e0020 " "
(gdb) p *(char **)((char *)bp)
$25 = 0xf69e0020 " "
(gdb) p *(char **)((char *)tail)
$26 = 0x804f560 <list> ""
(gdb) p *(char **)((char *)head+4)
$27 = 0xf69e0020 "
```
知add_node_to_head宏代码有问题，没有正确完成空闲节点的插入。原代码：
```
/* 在pos 之前 插入 node节点 */
#define insert_node(pos, node){ \
    GET_NEXT(GET_PREV(pos)) = (char *)(node); \
    GET_PREV(node) =  GET_PREV(pos); \
    GET_PREV(pos) = (char *)(node); \
    GET_NEXT(node) = (char *)(pos); \
}
/* 加入空闲节点到空闲链表 */
#define add_node_to_head(node) insert_node(GET_NEXT(head), node);
```
