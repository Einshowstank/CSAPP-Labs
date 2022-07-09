知识点：汇编语言函数传参、递归；汇编语言算数运算；
阅读phase4代码：
```
000000000040100c <phase_4>:
  40100c:	48 83 ec 18          	sub    $0x18,%rsp 栈指针减24个字节
  401010:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx 保存数组A第4个元素地址，sscanf第4个参数
  401015:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx 保存数组A第3个元素地址，sscanf第3个参数
  40101a:	be cf 25 40 00       	mov    $0x4025cf,%esi sscanf第2个参数，输入格式"%d %d"
  40101f:	b8 00 00 00 00       	mov    $0x0,%eax
  401024:	e8 c7 fb ff ff       	callq  400bf0 <__isoc99_sscanf@plt>
  401029:	83 f8 02             	cmp    $0x2,%eax
  40102c:	75 07                	jne    401035 <phase_4+0x29> 填充变量不等于2则爆炸
  40102e:	83 7c 24 08 0e       	cmpl   $0xe,0x8(%rsp)
  401033:	76 05                	jbe    40103a <phase_4+0x2e> A[2]小于等于14
  401035:	e8 00 04 00 00       	callq  40143a <explode_bomb>
  40103a:	ba 0e 00 00 00       	mov    $0xe,%edx
  40103f:	be 00 00 00 00       	mov    $0x0,%esi
  401044:	8b 7c 24 08          	mov    0x8(%rsp),%edi
  401048:	e8 81 ff ff ff       	callq  400fce <func4> 调用func4(&A[2], 0, 14, &A[3])
  40104d:	85 c0                	test   %eax,%eax
  40104f:	75 07                	jne    401058 <phase_4+0x4c> 返回值不等于0则爆炸
  401051:	83 7c 24 0c 00       	cmpl   $0x0,0xc(%rsp)
  401056:	74 05                	je     40105d <phase_4+0x51> A[3]要等于0
  401058:	e8 dd 03 00 00       	callq  40143a <explode_bomb>
  40105d:	48 83 c4 18          	add    $0x18,%rsp
  401061:	c3                   	retq   
```

同样，通过sscanf函数向栈空间数组A的A[2], A[3]写入两个整数。A[2]小于等于14。
调用函数 func4(&A[2], 0, 14, &A[3])，返回值要为0，且A[3] == 0。
进一步阅读func4代码：
```
0000000000400fce <func4>: 参数(&A[2], 0, 14, &A[3]])
  400fce:	48 83 ec 08          	sub    $0x8,%rsp 栈指针减8字节
  400fd2:	89 d0                	mov    %edx,%eax // int val = 14;
  400fd4:	29 f0                	sub    %esi,%eax // val -= 0;
  400fd6:	89 c1                	mov    %eax,%ecx // int val2 = val;
  400fd8:	c1 e9 1f             	shr    $0x1f,%ecx // val2 = val2 >> 31;// (逻辑右移)，值为0
  400fdb:	01 c8                	add    %ecx,%eax // val += val2;  值为14
  400fdd:	d1 f8                	sar    %eax      // val = val >> 1;  值为7
  400fdf:	8d 0c 30             	lea    (%rax,%rsi,1),%ecx // val2 = 7 + 0;
  400fe2:	39 f9                	cmp    %edi,%ecx
  400fe4:	7e 0c                	jle    400ff2 <func4+0x24> 如果7 小于等于 A[2]跳转
  400fe6:	8d 51 ff             	lea    -0x1(%rcx),%edx
  400fe9:	e8 e0 ff ff ff       	callq  400fce <func4>
  400fee:	01 c0                	add    %eax,%eax
  400ff0:	eb 15                	jmp    401007 <func4+0x39>
  400ff2:	b8 00 00 00 00       	mov    $0x0,%eax
  400ff7:	39 f9                	cmp    %edi,%ecx
  400ff9:	7d 0c                	jge    401007 <func4+0x39> 7大于等于A[2],所以A[2]可以为7
  400ffb:	8d 71 01             	lea    0x1(%rcx),%esi
  400ffe:	e8 cb ff ff ff       	callq  400fce <func4>
  401003:	8d 44 00 01          	lea    0x1(%rax,%rax,1),%eax
  401007:	48 83 c4 08          	add    $0x8,%rsp
  40100b:	c3                   	retq  
```
func4 并没有借助第四个参数A[3]的地址来修改A[3]的值，可以知道A[3]值为0。
经过一些列算数运算，由400fe4跳转到400ff2的话可以返回0，且A[2]等于7为边界条件，可以不进入递归，
所以答案为 7 0