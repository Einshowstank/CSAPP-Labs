分段阅读代码：
```
  4010f4:	41 56                	push   %r14
  4010f6:	41 55                	push   %r13
  4010f8:	41 54                	push   %r12
  4010fa:	55                   	push   %rbp
  4010fb:	53                   	push   %rbx
  4010fc:	48 83 ec 50          	sub    $0x50,%rsp 栈指针减80字节
  401100:	49 89 e5             	mov    %rsp,%r13
  401103:	48 89 e6             	mov    %rsp,%rsi
  401106:	e8 51 03 00 00       	callq  40145c <read_six_numbers> 读6个整数到数组
```
开始读取6个整数到栈空间，地址%rsp ~ %rsp + 20

```
  40110b:	49 89 e6             	mov    %rsp,%r14  保存栈+0字节地址
  40110e:	41 bc 00 00 00 00    	mov    $0x0,%r12d 
  401114:	4c 89 ed             	mov    %r13,%rbp  保存栈+4(j-1)字节地址
  401117:	41 8b 45 00          	mov    0x0(%r13),%eax 保存栈+4(j-1)字节值
  40111b:	83 e8 01             	sub    $0x1,%eax 栈+4(j-1)字节值-1
  40111e:	83 f8 05             	cmp    $0x5,%eax
  401121:	76 05                	jbe    401128 <phase_6+0x34> 栈+4(j-1)字节值-1小于等于5
  401123:	e8 12 03 00 00       	callq  40143a <explode_bomb>
  401128:	41 83 c4 01          	add    $0x1,%r12d  设 %r12d保存 j
  40112c:	41 83 fc 06          	cmp    $0x6,%r12d  // for(int j = 1; j < 6; ++j)
  401130:	74 21                	je     401153 <phase_6+0x5f>
  401132:	44 89 e3             	mov    %r12d,%ebx  // int v1 = i;
  401135:	48 63 c3             	movslq %ebx,%rax   设 %ebx 保存变量 i
  401138:	8b 04 84             	mov    (%rsp,%rax,4),%eax  // v2 = A[i];
  40113b:	39 45 00             	cmp    %eax,0x0(%rbp)
  40113e:	75 05                	jne    401145 <phase_6+0x51> 数组后续数字不等于栈+4(j-1)字节值
  401140:	e8 f5 02 00 00       	callq  40143a <explode_bomb>
  401145:	83 c3 01             	add    $0x1,%ebx
  401148:	83 fb 05             	cmp    $0x5,%ebx
  40114b:	7e e8                	jle    401135 <phase_6+0x41>
  40114d:	49 83 c5 04          	add    $0x4,%r13    结束循环，%r13保存栈+4字节地址
  401151:	eb c1                	jmp    401114 <phase_6+0x20>
```
该段代码是一个双循环，对应简略c语言代码：
```
if (M[0] > 6)
    explode_bomb();
for(int j = 1; j < 6; ++j){
    int i = j;
    for(i; i <= 5; ++i)
        if (M[i] == M[j-1])
            explode_bomb();
    if (M[j] > 6)
        explode_bomb();
}
```
说明数组6个元素都小于等于6，且互不相等。


```
  401153:	48 8d 74 24 18       	lea    0x18(%rsp),%rsi 保存栈+24字节地址
  401158:	4c 89 f0             	mov    %r14,%rax       保存栈+0字节地址
  40115b:	b9 07 00 00 00       	mov    $0x7,%ecx
  401160:	89 ca                	mov    %ecx,%edx
  401162:	2b 10                	sub    (%rax),%edx     7 - 栈+4x字节值 (x 范围0~5)
  401164:	89 10                	mov    %edx,(%rax)     栈+4x字节值 = 7 - 栈+4x字节值
  401166:	48 83 c0 04          	add    $0x4,%rax       保存栈+4(x+1)字节地址
  40116a:	48 39 f0             	cmp    %rsi,%rax
  40116d:	75 f1                	jne    401160 <phase_6+0x6c>
```
该段代码为一个循环，M[i] = 7 - M[i];(i = 0~5)


```
  40116f:	be 00 00 00 00       	mov    $0x0,%esi
  401174:	eb 21                	jmp    401197 <phase_6+0xa3>
  401176:	48 8b 52 08          	mov    0x8(%rdx),%rdx
  40117a:	83 c0 01             	add    $0x1,%eax
  40117d:	39 c8                	cmp    %ecx,%eax   选取链表第M[i]个节点
  40117f:	75 f5                	jne    401176 <phase_6+0x82>
  401181:	eb 05                	jmp    401188 <phase_6+0x94>
  401183:	ba d0 32 60 00       	mov    $0x6032d0,%edx
  401188:	48 89 54 74 20       	mov    %rdx,0x20(%rsp,%rsi,2) 从栈指针+32字节位置开始保存8字节值
  40118d:	48 83 c6 04          	add    $0x4,%rsi
  401191:	48 83 fe 18          	cmp    $0x18,%rsi
  401195:	74 14                	je     4011ab <phase_6+0xb7>  %rsi从0增加到24跳出循环
  401197:	8b 0c 34             	mov    (%rsp,%rsi,1),%ecx  循环遍历数组6个元素,ecx保存
  40119a:	83 f9 01             	cmp    $0x1,%ecx
  40119d:	7e e4                	jle    401183 <phase_6+0x8f> 小于等于1则跳转
  40119f:	b8 01 00 00 00       	mov    $0x1,%eax
  4011a4:	ba d0 32 60 00       	mov    $0x6032d0,%edx
  4011a9:	eb cb                	jmp    401176 <phase_6+0x82>
```
0x6032d0是一个链表地址，上面代码功能是选取链表第M[i]个节点值存取在从栈指针+32字节开始的位置。


```
  4011da:	bd 05 00 00 00       	mov    $0x5,%ebp
  4011df:	48 8b 43 08          	mov    0x8(%rbx),%rax  //L[1]
  4011e3:	8b 00                	mov    (%rax),%eax
  4011e5:	39 03                	cmp    %eax,(%rbx)
  4011e7:	7d 05                	jge    4011ee <phase_6+0xfa>  //L[i] 要大于等于 L[i+1]
  4011e9:	e8 4c 02 00 00       	callq  40143a <explode_bomb>
  4011ee:	48 8b 5b 08          	mov    0x8(%rbx),%rbx  //L[1]
  4011f2:	83 ed 01             	sub    $0x1,%ebp       // 5-i
  4011f5:	75 e8                	jne    4011df <phase_6+0xeb> 循环，从5开始递减到0
```
最后一段代码说明L[i] 要大于等于 L[i+1]，即链表维持递减，
gdb查看原链表信息，
```
(gdb) print /x  *0x6032d0
$10 = 0x14c
(gdb) print /x  *0x6032d8 （链表节点指针）
$11 = 0x6032e0
(gdb) print /x  *0x6032e0
$12 = 0xa8
(gdb) print /x  *0x6032f0
$13 = 0x39c
(gdb) print /x  *0x603300
$14 = 0x2b3
(gdb) print /x  *0x603310
$15 = 0x1dd
(gdb) print /x  *0x603320
$16 = 0x1bb
```
知原链表值为：
0x14c, 0xa8, 0x39c, 0x2b3, 0x1dd, 0x1bb
要按递减重排，则选择顺序为
3，4，5，6，1，2
由于经过M[i] = 7 - M[i]，所以原值为
4，3，2，1，6，5
