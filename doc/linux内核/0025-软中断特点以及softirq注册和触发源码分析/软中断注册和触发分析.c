//kernel/softirq.c
static struct softirq_action softirq_vec[NR_SOFTIRQS] __cacheline_aligned_in_smp;

DEFINE_PER_CPU(struct task_struct *, ksoftirqd);

// 软中断的类型
// TIMER：内核定时器
// NET_TX/NET_RX：用于网卡驱动，处理网卡数据的发送和接收
// BLOCK：用于处理块设备
const char * const softirq_to_name[NR_SOFTIRQS] = {
	"HI", "TIMER", "NET_TX", "NET_RX", "BLOCK", "IRQ_POLL",
	"TASKLET", "SCHED", "HRTIMER", "RCU"
};

//include/linux/interrupt.h
/* softirq mask and active fields moved to irq_cpustat_t in
 * asm/hardirq.h to get better cache usage.  KAO
 */
struct softirq_action
{
	void	(*action)(struct softirq_action *);
};

//软中断的注册
//---------------------------------------------------------------------------------------------------
//kernel/softirq.c
//注册软中断
void open_softirq(int nr, void (*action)(struct softirq_action *))
{
	softirq_vec[nr].action = action;
}

//例子：初始化软中断
//注册了 TASKLET_SOFTIRQ，HI_SOFTIRQ两个类型的软中断
void __init softirq_init(void)
{
	int cpu;

	for_each_possible_cpu(cpu) {
		per_cpu(tasklet_vec, cpu).tail =
			&per_cpu(tasklet_vec, cpu).head;
		per_cpu(tasklet_hi_vec, cpu).tail =
			&per_cpu(tasklet_hi_vec, cpu).head;
	}

	open_softirq(TASKLET_SOFTIRQ, tasklet_action);
	open_softirq(HI_SOFTIRQ, tasklet_hi_action);
}
//---------------------------------------------------------------------------------------------------

//软中断的触发
//---------------------------------------------------------------------------------------------------
//kernel/softirq.c
void raise_softirq(unsigned int nr)
{
	unsigned long flags;
	
	// 禁止本地中断
	local_irq_save(flags);
	raise_softirq_irqoff(nr);
	// 恢复本地中断
	local_irq_restore(flags);
}

/*
 * 调用这个函数之前，必须禁止本地中断
 * This function must run with irqs disabled!
 */
inline void raise_softirq_irqoff(unsigned int nr)
{
	__raise_softirq_irqoff(nr);

	/*
	 * If we're in an interrupt or softirq, we're done
	 * (this also catches softirq-disabled code). We will
	 * actually run the softirq once we return from
	 * the irq or softirq.
	 *
	 * Otherwise we wake up ksoftirqd to make sure we
	 * schedule the softirq soon.
	 */
	if (!in_interrupt())
		wakeup_softirqd();
}

void __raise_softirq_irqoff(unsigned int nr)
{
	trace_softirq_raise(nr);
	or_softirq_pending(1UL << nr);
}

//include/linux/interrupt.h
#ifndef __ARCH_SET_SOFTIRQ_PENDING
#define set_softirq_pending(x) (local_softirq_pending() = (x))
#define or_softirq_pending(x)  (local_softirq_pending() |= (x))
#endif

//include/linux/irq_cpustat.h
/* arch independent irq_stat fields */
#define local_softirq_pending() \
	__IRQ_STAT(smp_processor_id(), __softirq_pending)

#ifndef __ARCH_IRQ_STAT
extern irq_cpustat_t irq_stat[];		/* defined in asm/hardirq.h */
#define __IRQ_STAT(cpu, member)	(irq_stat[cpu].member)
#endif

// 重点来了哈
// 软件中断它有一个特点：谁（哪个CPU）触发，谁（哪个CPU）执行。
// 那这个是怎么来实现的呢？
// 就是通过这个结构体来实现的
// 在前面，我们触发软中断的时候，我们会执行 local_softirq_pending() |= (x)
// 
//arch/arm/include/asm/hardirq.h
typedef struct {
	// 可以看作是软件中断的寄存器
	// 它的每一个 bit 都代表不同类型的中断。
	// 当某个软件中断触发的时候，我们就给它的相应的比特位置 1。
	// 在后续软件中断的处理时，就可以通过查看它哪一个比特置位
	// 了，就知道哪一个类型的软件中断被触发了。就可以处理相关
	// 的处理函数了。
	unsigned int __softirq_pending;
#ifdef CONFIG_SMP
	unsigned int ipi_irqs[NR_IPI];
#endif
} ____cacheline_aligned irq_cpustat_t;
//---------------------------------------------------------------------------------------------------