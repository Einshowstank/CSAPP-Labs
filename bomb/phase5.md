知识点：金丝雀；字符ASCII码值；汇编循环语句；字符数组通过偏置取地址；
阅读完整代码：
```
0000000000401062 <phase_5>: 设输入第一个参数为input1
  401062:	53                   	push   %rbx
  401063:	48 83 ec 20          	sub    $0x20,%rsp 栈指针减32字节
  401067:	48 89 fb             	mov    %rdi,%rbx  保存input1起始地址到%rbx
  40106a:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax 获得金丝雀（栈破坏检测）
  401071:	00 00 
  401073:	48 89 44 24 18       	mov    %rax,0x18(%rsp) 保存金丝雀值到栈+24字节位置
  401078:	31 c0                	xor    %eax,%eax
  40107a:	e8 9c 02 00 00       	callq  40131b <string_length>
  40107f:	83 f8 06             	cmp    $0x6,%eax  input1为字符串，长度为6 否则爆炸
  401082:	74 4e                	je     4010d2 <phase_5+0x70>
  401084:	e8 b1 03 00 00       	callq  40143a <explode_bomb>
  401089:	eb 47                	jmp    4010d2 <phase_5+0x70>
  40108b:	0f b6 0c 03          	movzbl (%rbx,%rax,1),%ecx 进入循环 %ecx 保存 input1 的1~6个字符
  40108f:	88 0c 24             	mov    %cl,(%rsp)    栈+0字节保存 input1字符
  401092:	48 8b 14 24          	mov    (%rsp),%rdx   %rdx 保存input1字符
  401096:	83 e2 0f             	and    $0xf,%edx     %rdx 只保存最后半字节，比如a的ascii码0x61，得到0x1
  401099:	0f b6 92 b0 24 40 00 	movzbl 0x4024b0(%rdx),%edx  对以下字符串取偏置得到单个字符
          0x4024b0: "maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?"
  4010a0:	88 54 04 10          	mov    %dl,0x10(%rsp,%rax,1) 保存%rdx的最后1字节到栈+16~+21字节位置
  4010a4:	48 83 c0 01          	add    $0x1,%rax    %rax初始为0，不断加1
  4010a8:	48 83 f8 06          	cmp    $0x6,%rax
  4010ac:	75 dd                	jne    40108b <phase_5+0x29> 进入循环，%rax 从0 ~5
  4010ae:	c6 44 24 16 00       	movb   $0x0,0x16(%rsp) 栈+22字节保存0
  4010b3:	be 5e 24 40 00       	mov    $0x40245e,%esi  gdb查询知值为 "flyers"
  4010b8:	48 8d 7c 24 10       	lea    0x10(%rsp),%rdi
  4010bd:	e8 76 02 00 00       	callq  401338 <strings_not_equal> 比较栈+16~+21字节保存字符串 和"flyers"
  4010c2:	85 c0                	test   %eax,%eax
  4010c4:	74 13                	je     4010d9 <phase_5+0x77>
  4010c6:	e8 6f 03 00 00       	callq  40143a <explode_bomb>
  4010cb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
  4010d0:	eb 07                	jmp    4010d9 <phase_5+0x77>
  4010d2:	b8 00 00 00 00       	mov    $0x0,%eax
  4010d7:	eb b2                	jmp    40108b <phase_5+0x29>
  4010d9:	48 8b 44 24 18       	mov    0x18(%rsp),%rax 取回金丝雀
  4010de:	64 48 33 04 25 28 00 	xor    %fs:0x28,%rax  栈破坏检测
  4010e5:	00 00 
  4010e7:	74 05                	je     4010ee <phase_5+0x8c>
  4010e9:	e8 42 fa ff ff       	callq  400b30 <__stack_chk_fail@plt>
  4010ee:	48 83 c4 20          	add    $0x20,%rsp
  4010f2:	5b                   	pop    %rbx
  4010f3:	c3                   	retq 
```

%fs:0x28是一个金丝雀值，用于栈破坏检测。该值只读，放入栈中某一位置，可在程序结束时对比原值判断缓冲区是否溢出：
```
  40106a:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax 获得金丝雀（栈破坏检测）
  401071:	00 00 
  401073:	48 89 44 24 18       	mov    %rax,0x18(%rsp) 保存金丝雀值到栈+24字节位置
  ...其他代码...
  4010d9:	48 8b 44 24 18       	mov    0x18(%rsp),%rax 取回金丝雀
  4010de:	64 48 33 04 25 28 00 	xor    %fs:0x28,%rax  栈破坏检测
  4010e5:	00 00 
  4010e7:	74 05                	je     4010ee <phase_5+0x8c>
  4010e9:	e8 42 fa ff ff       	callq  400b30 <__stack_chk_fail@plt>
```

阅读函数知要求输入一长度为6的字符串，之后进入循环，每次取一个字符，获取其ASCII码值，截断为半字节，
比如字符a的ASCII码值为0x61，阶段后得0x01，即为一个偏置。
通过gdb查询地址0x4024b0保存字符串（假设输入abcdef）:
```
(gdb) x/s 0x4024b0
0x4024b0 <array.3449>:  "maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?"
```

movzbl 0x4024b0(%rdx),%edx  即为对以上字符串取偏置得到单个字符，并把字符保存到栈+16~+21字节位置。
至于保存的这6个字符应该是啥，继续阅读代码。循环结束后：
```
  4010ae:	c6 44 24 16 00       	movb   $0x0,0x16(%rsp) 栈+22字节保存0
  4010b3:	be 5e 24 40 00       	mov    $0x40245e,%esi  gdb查询知值为 "flyers"
  4010b8:	48 8d 7c 24 10       	lea    0x10(%rsp),%rdi
  4010bd:	e8 76 02 00 00       	callq  401338 <strings_not_equal> 比较栈+16~+21字节保存字符串 和"flyers"
```

可知这6个字符要等于"flyers"，所以对应字符串
"maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?"
偏置分别为：
9 15 14 5 6 7
可以对应取末尾半字节的ASCII码：
0x69 0x6F 0x6E 0x65 0x66 0x67
即对应字符：
i o n e f g