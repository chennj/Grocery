struct thread_info {
	...
	int			preempt_count;				/* 0 => preemptable, <0 => bug */
	...
}

如 1.jpg 图所示
hardirq count		：描述当前这个中断handler嵌套的深度，当他为嵌套的深度，当他为 1 的时候，表示有一个
					  irq handler 正在执行，即处于硬件中断上下文。在新的内核里面，因为不允许中断嵌套，
					  所以它不是 0， 就是 1
softirq count		：第 8 bit，因为软中断在单个cpu上不会嵌套执行，所以类似 hardirq，它不是 0， 就是 1，
					  如果是 1，表示有软中断的 handler 在处理。
					  bit:8 = 1 表示软中断上下文。bit:9~15 > 0 也表示处于软中断上下文。
preemption count	: 用来判断当前进程是否可以被抢占。如果 !=0 就表示当前进程不能够被抢占。

如 2.jpg 图所示 - 内核中各类上下文的定义

当我们调用了 lock_bh_disable();就表示进入了软件中断上下文了