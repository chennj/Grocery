//amd64xx.h

extern "C" int __stdcall func1();

extern "C" void __stdcall func2();

//amd64xx.asm

EXTERN  print2:PROC;�����ⲿ����

EXTERN  g_iValue:DQ;�����ⲿ����

.DATA

val1 DQ ?;�Լ��������

.CODE

func1 PROC

mov r10, g_iValue;ʹ��func.h�е��ⲿ����

mov val1,r10;ʹ���Զ������

mov rax,val1

ret;��������أ���ô�����ִ��func2

FUNC1 ENDP

func2 PROC

CALL print2 ;����func.cpp�е��ⲿ����

ret

FUNC2 ENDP

END