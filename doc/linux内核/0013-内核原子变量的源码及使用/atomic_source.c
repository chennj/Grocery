static atomic_t can_open = ATOMIC_INIT(1);

arch/arm/include/asm/atomic.h

#if __LINUX_ARM_ARCH__ >= 6
/*
 * ARMv6 UP and SMP safe atomic ops.  We use load exclusive and
 * store exclusive to ensure that these are atomic.  We may loop
 * to ensure that the update happens.
 */
#define ATOMIC_OP(op, c_op, asm_op)					\
static inline void atomic_##op(int i, atomic_t *v)			\
{									\
	unsigned long tmp;						\
	int result;							\
									\
	prefetchw(&v->counter);						\
	__asm__ __volatile__("@ atomic_" #op "\n"			\
"1:	ldrex	%0, [%3]\n"						\
"	" #asm_op "	%0, %0, %4\n"					\
"	strex	%1, %0, [%3]\n"						\
"	teq	%1, #0\n"						\
"	bne	1b"							\
	: "=&r" (result), "=&r" (tmp), "+Qo" (v->counter)		\
	: "r" (&v->counter), "Ir" (i)					\
	: "cc");							\
}	
/*
&v->counter		= 原子变量的地址
%0 				= result
%1				= tmp
%3 				= &v->counter
%4				= i

[%3]			= [原子变量的地址]	= 原子变量的值
如果是 atomic_inc，则 ##op = add
那么上面这段汇编干的事就是
---------------------------------------------
result = 原子变量的值
result = result + i
原子变量的值 = result; 如果成功则 tmp = 0
判断 tmp 是否等于 0，如果是 0 则表示写入成功
否则跳转回 1: 继续执行。
---------------------------------------------
注：
---------------------------------------------
利用 LDREX 和 STREX 可在多个处理器和共享内存系统之前实现进程间通信。

LDREX 指令会设置一个独占标记，其他cpu不能修改
直到
STREX 指令执行完才会清楚这个独占标记，STREX在更新内存数值时，
会检查该段内存是否已经被标记为独占访问，并以此来决定是否更新内存中的值

例如：这条指令
STREX Rx, Ry, [Rz]
如果执行这条指令的时候发现已经被标记为独占访问了，则将寄存器Ry中的值更新到寄存器Rz指向的内存，并将寄存器Rx设置成0。
指令执行成功后，会将独占访问标记位清除。否则不设置Rx的值。

而如果执行这条指令的时候发现没有设置独占标记，则不会更新内存，且将寄存器Rx的值设置成1。
例如：锁的实现
    MOV r1, #0x1                ; load the ‘lock taken’ value
try
    LDREX r0, [LockAddr]        ; load the lock value
    CMP r0, #0                  ; is the lock free?
    STREXEQ r0, r1, [LockAddr]  ; try and claim the lock
    CMPEQ r0, #0                ; did this succeed?
    BNE try                     ; no – try again
    ....                        ; yes – we have the lock

STREX 指令中所用的地址必须要与近期执行次数最多的 LDREX 指令所用的地址相同。 如果使用不同的地址，则 STREX 指令的执行结果将不可预知。
---------------------------------------------
综上所述：
ATOMIC_OP 原子变量通过这种操作保证了操作的原子性。
*/

// 阅读linux内核，嵌入式汇编时绕不过去的
// 嵌入式汇编语法
/*
	asm("汇编语句"
	:"输出寄存器"
	:"输入寄存器"
	:"会被修改的寄存器")

其中：
"汇编语句"			：	就是x86或者arm的汇编指令代码
"输出寄存器"		：	表示当这段嵌入式汇编执行完成后，哪些寄存器用于存放输出数据。这些寄存器会分别对应一个C
						语言表达式值或一个内存地址。
"输入寄存器"		：	表示在开始执行汇编代码时，这里指定的一些寄存器中应存放的输入值，它们分别对应着一个C
						或常数值。
"会被修改的寄存器"	：	也称作clober list，描述了汇编代码对寄存器的修改情况。

为何需要 clober list？
我们的汇编代码是用 gcc 来处理的，当遇到嵌入式汇编的时候，gcc会将这些嵌入式汇编的文本送给 Assembler 进行后期处理。
这样 gcc 需要了解这些汇编代码对寄存器的修改情况，否则可能会大麻烦。比如：gcc 对 C 代码进行处理，将某些变量值保存在
寄存器中，如果嵌入式汇编修改了该寄存器的值，有没有通知 gcc，那么 gcc 会以为寄存器中仍然保存了之前的值，乱套不？
*/