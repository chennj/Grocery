//amd64xx.h

extern "C" int __stdcall func1();

extern "C" void __stdcall func2();

//amd64xx.asm

EXTERN  print2:PROC;引用外部函数

EXTERN  g_iValue:DQ;引用外部变量

.DATA

val1 DQ ?;自己定义变量

.CODE

func1 PROC

mov r10, g_iValue;使用func.h中的外部变量

mov val1,r10;使用自定义变量

mov rax,val1

ret;如果不返回，那么会继续执行func2

FUNC1 ENDP

func2 PROC

CALL print2 ;调用func.cpp中的外部函数

ret

FUNC2 ENDP

END