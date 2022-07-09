查看汇编代码：
```
0000000000400efc <phase_2>:
  400efc:	55                   	push   %rbp
  400efd:	53                   	push   %rbx
  400efe:	48 83 ec 28          	sub    $0x28,%rsp 栈指针减40字节
  400f02:	48 89 e6             	mov    %rsp,%rsi 数组指针传入第二个参数
  400f05:	e8 52 05 00 00       	callq  40145c <read_six_numbers> 输入6个4字节数保存进数组，0x4025c3地址保存sscanf格式
  400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp) 数组第一个元素要为1
  400f0e:	74 20                	je     400f30 <phase_2+0x34>
  400f10:	e8 25 05 00 00       	callq  40143a <explode_bomb>
  400f15:	eb 19                	jmp    400f30 <phase_2+0x34>
  400f17:	8b 43 fc             	mov    -0x4(%rbx),%eax 开始循环 保存数组上个元素 
  400f1a:	01 c0                	add    %eax,%eax 上个元素乘2
  400f1c:	39 03                	cmp    %eax,(%rbx) 与当前元素比较要相等，说明是等比数组,q为2
  400f1e:	74 05                	je     400f25 <phase_2+0x29>
  400f20:	e8 15 05 00 00       	callq  40143a <explode_bomb>
  400f25:	48 83 c3 04          	add    $0x4,%rbx 保存数组第3个元素
  400f29:	48 39 eb             	cmp    %rbp,%rbx
  400f2c:	75 e9                	jne    400f17 <phase_2+0x1b> 非空，则进入循环，否则结束返回
  400f2e:	eb 0c                	jmp    400f3c <phase_2+0x40>
  400f30:	48 8d 5c 24 04       	lea    0x4(%rsp),%rbx 保存数组第2个元素
  400f35:	48 8d 6c 24 18       	lea    0x18(%rsp),%rbp 保存数组第7个元素（空）
  400f3a:	eb db                	jmp    400f17 <phase_2+0x1b>
  400f3c:	48 83 c4 28          	add    $0x28,%rsp
  400f40:	5b                   	pop    %rbx
  400f41:	5d                   	pop    %rbp
  400f42:	c3                   	retq  
```

该函数初始化40字节缓冲区数组，read_six_numbers函数读入6个4字节整数保存进数组，
第一个数为1，根据代码注释得知该数组需为公比为2的等比数列。
所以答案为 1 2 4 8 16 32

read_six_numbers函数解读：
```
000000000040145c <read_six_numbers>:
  40145c:	48 83 ec 18          	sub    $0x18,%rsp 栈指针减24个字节
  401460:	48 89 f2             	mov    %rsi,%rdx 保存数组起始地址 0
  401463:	48 8d 4e 04          	lea    0x4(%rsi),%rcx 保存数组+4字节地址 1
  401467:	48 8d 46 14          	lea    0x14(%rsi),%rax 
  40146b:	48 89 44 24 08       	mov    %rax,0x8(%rsp) 保存数组+20字节地址 5 到 0x8(%rsp)
  401470:	48 8d 46 10          	lea    0x10(%rsi),%rax 
  401474:	48 89 04 24          	mov    %rax,(%rsp)    保存数组+16字节地址 4 到 (%rsp)
  401478:	4c 8d 4e 0c          	lea    0xc(%rsi),%r9 保存数组+12字节地址 3
  40147c:	4c 8d 46 08          	lea    0x8(%rsi),%r8 保存数组+8字节地址 2
  401480:	be c3 25 40 00       	mov    $0x4025c3,%esi sscanf格式保存在该地址
  401485:	b8 00 00 00 00       	mov    $0x0,%eax
  40148a:	e8 61 f7 ff ff       	callq  400bf0 <__isoc99_sscanf@plt> 参数为 rdi, rsi, rdx, rcx, r8, r9, (%rsp),8(%rsp)
  40148f:	83 f8 05             	cmp    $0x5,%eax
  401492:	7f 05                	jg     401499 <read_six_numbers+0x3d>
  401494:	e8 a1 ff ff ff       	callq  40143a <explode_bomb>
  401499:	48 83 c4 18          	add    $0x18,%rsp
  40149d:	c3                   	retq  
```
该函数调用sscanf，第一个参数为输入字符串，第二个参数为输入格式，剩余参数为保存地址