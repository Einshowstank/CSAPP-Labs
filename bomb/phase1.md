首先 objdump -d ./bomb >> bomb.s 反汇编得到汇编代码
查看phase1函数汇编代码：
```
0000000000400ee0 <phase_1>:
  400ee0:	48 83 ec 08          	sub    $0x8,%rsp
  400ee4:	be 00 24 40 00       	mov    $0x402400,%esi 找出该寄存器保存的值就是答案
  400ee9:	e8 4a 04 00 00       	callq  401338 <strings_not_equal> 比较字符串是否相等
  400eee:	85 c0                	test   %eax,%eax
  400ef0:	74 05                	je     400ef7 <phase_1+0x17>
  400ef2:	e8 43 05 00 00       	callq  40143a <explode_bomb>
  400ef7:	48 83 c4 08          	add    $0x8,%rsp
  400efb:	c3                   	retq   
```
strings_not_equal函数比较两个字符串是否相等，寄存器esi作为第二个参数，使用gdb查看地址0x402400保存的值即为答案；
gdb设置断点：
`break *0x400ee9`
查案地址保存值：
```
(gdb) x/s 0x402400
0x402400:       "Border relations with Canada have never been better."
```
