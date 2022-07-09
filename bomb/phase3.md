知识点：汇编语句函数传参；gdb查看地址的值；
查看汇编带码：
```
0000000000400f43 <phase_3>:
  400f43:	48 83 ec 18          	sub    $0x18,%rsp 栈指针减24个字节
  400f47:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx 保存数组第4个元素地址，sscanf第4个参数
  400f4c:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx 保存数组第3个元素地址，sscanf第3个参数
  400f51:	be cf 25 40 00       	mov    $0x4025cf,%esi sscanf 格式 "%d %d"
  400f56:	b8 00 00 00 00       	mov    $0x0,%eax
  400f5b:	e8 90 fc ff ff       	callq  400bf0 <__isoc99_sscanf@plt>
  400f60:	83 f8 01             	cmp    $0x1,%eax 返回填充变量数大于1才行
  400f63:	7f 05                	jg     400f6a <phase_3+0x27>
  400f65:	e8 d0 04 00 00       	callq  40143a <explode_bomb>
  400f6a:	83 7c 24 08 07       	cmpl   $0x7,0x8(%rsp)
  400f6f:	77 3c                	ja     400fad <phase_3+0x6a> 如果数组第3个元素（输入1）大于7，爆炸
  400f71:	8b 44 24 08          	mov    0x8(%rsp),%eax 保存数组第3个元素地址
  400f75:	ff 24 c5 70 24 40 00 	jmpq   *0x402470(,%rax,8) 使用gdb查找 发现输入第一个数为1时跳转到400fb9
  400f7c:	b8 cf 00 00 00       	mov    $0xcf,%eax
  400f81:	eb 3b                	jmp    400fbe <phase_3+0x7b>
 ... 中间略过 ...
  400fb2:	b8 00 00 00 00       	mov    $0x0,%eax
  400fb7:	eb 05                	jmp    400fbe <phase_3+0x7b>
  400fb9:	b8 37 01 00 00       	mov    $0x137,%eax
  400fbe:	3b 44 24 0c          	cmp    0xc(%rsp),%eax  数组第4个元素和%eax值比较
  400fc2:	74 05                	je     400fc9 <phase_3+0x86> 不相等则爆炸
  400fc4:	e8 71 04 00 00       	callq  40143a <explode_bomb>
  400fc9:	48 83 c4 18          	add    $0x18,%rsp
  400fcd:	c3                   	retq   
```
该函数调用sscanf，gdb查看地址0x4025cf值为"%d %d"，说明读入2个整数，放入数组第3，第4个元素中。
当读入第一个数为1时，gdb查看地址0x402478值为400fb9，%eax保存值为10进制311.
所以答案可以为 1 311