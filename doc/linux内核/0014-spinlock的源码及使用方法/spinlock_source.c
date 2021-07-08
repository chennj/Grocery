// spinlock 主要用于多核cpu下的同步
// 单核cpu下不会多做什么，只是关闭一下进程的抢占而已

// linux-4.9.229/include/linux/spinlock_types.h
typedef struct spinlock {
	union {
		struct raw_spinlock rlock;

#ifdef CONFIG_DEBUG_LOCK_ALLOC
#define LOCK_PADSIZE (offsetof(struct raw_spinlock, dep_map))
		struct {
			u8 __padding[LOCK_PADSIZE];
			struct lockdep_map dep_map;
		};
#endif
	};
} spinlock_t;

typedef struct raw_spinlock {
	arch_spinlock_t raw_lock;
#ifdef CONFIG_GENERIC_LOCKBREAK
	unsigned int break_lock;
#endif
#ifdef CONFIG_DEBUG_SPINLOCK
	unsigned int magic, owner_cpu;
	void *owner;
#endif
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map dep_map;
#endif
} raw_spinlock_t;

// linux-4.9.229/arch/arm/include/asm/spinlock_types.h
typedef struct {
	union {
		u32 slock;
		struct __raw_tickets {
#ifdef __ARMEB__
			u16 next;
			u16 owner;
#else
			u16 owner;
			u16 next;
#endif
		} tickets;
	};
} arch_spinlock_t;

///////////////////////////////////////////////////////////////////////////
// spin_lock_init(&count_lock);
///////////////////////////////////////////////////////////////////////////
// linux-4.9.229/include/linux/spinlock.h
#define spin_lock_init(_lock)				\
do {							\
	spinlock_check(_lock);				\
	raw_spin_lock_init(&(_lock)->rlock);		\
} while (0)
	
#define raw_spin_lock_init(lock)				\
	do { *(lock) = __RAW_SPIN_LOCK_UNLOCKED(lock); } while (0)

// linux-4.9.229/include/linux/spinlock_types.h
#define __RAW_SPIN_LOCK_UNLOCKED(lockname)	\
	(raw_spinlock_t) __RAW_SPIN_LOCK_INITIALIZER(lockname)
	
#define __RAW_SPIN_LOCK_INITIALIZER(lockname)	\
	{					\
	.raw_lock = __ARCH_SPIN_LOCK_UNLOCKED,	\
	SPIN_DEBUG_INIT(lockname)		\
	SPIN_DEP_MAP_INIT(lockname) }

// linux-4.9.229/arch/arm/include/asm/spinlock_types.h
#define __ARCH_SPIN_LOCK_UNLOCKED	{ { 0 } }

///////////////////////////////////////////////////////////////////////////
// spin_lock(&count_lock)
///////////////////////////////////////////////////////////////////////////
// linux-4.9.229/include/linux/spinlock.h
static __always_inline void spin_lock(spinlock_t *lock)
{
	raw_spin_lock(&lock->rlock);
}

#define raw_spin_lock(lock)	_raw_spin_lock(lock)

// 在单核cpu下的实现
//~/linux-4.9.229/include/linux/spinlock_api_up.h 
#define _raw_spin_lock(lock)			__LOCK(lock)
/*
 * In the UP-nondebug case there's no real locking going on, so the
 * only thing we have to do is to keep the preempt counts and irq
 * flags straight, to suppress compiler warnings of unused lock
 * variables, and to add the proper checker annotations:
 */
#define __LOCK(lock) \
  do { preempt_disable();/*关闭独占*/ ___LOCK(lock); } while (0)
#define ___LOCK(lock) \
  do { __acquire(lock);/*返回一个0，啥也没干*/ (void)(lock); } while (0)
	  
// 在多核cpu下的实现
// ~/linux-4.9.229/kernel/locking/spinlock.c
void __lockfunc _raw_spin_lock(raw_spinlock_t *lock)
{
	__raw_spin_lock(lock);
}

//~/linux-4.9.229/include/linux/spinlock_api_smp.h
static inline void __raw_spin_lock(raw_spinlock_t *lock)
{
	preempt_disable();	//关闭进程抢占
	spin_acquire(&lock->dep_map, 0, 0, _RET_IP_);
	LOCK_CONTENDED(lock, do_raw_spin_trylock, do_raw_spin_lock);
}

//~/linux-4.9.229/include/linux/lockdep.h
#define LOCK_CONTENDED(_lock, try, lock) \
	lock(_lock)

//~/linux-4.9.229/kernel/locking/spinlock_debug.c
void do_raw_spin_lock(raw_spinlock_t *lock)
{
	debug_spin_lock_before(lock);
	arch_spin_lock(&lock->raw_lock);
	debug_spin_lock_after(lock);
}

//~/linux-4.9.229/arch/arm/include/asm/spinlock.h
static inline void arch_spin_lock(arch_spinlock_t *lock)
{
	unsigned long tmp;
	u32 newval;
	arch_spinlock_t lockval;

	prefetchw(&lock->slock);
	__asm__ __volatile__(
"1:	ldrex	%0, [%3]\n"
"	add	%1, %0, %4\n"
"	strex	%2, %1, [%3]\n"
"	teq	%2, #0\n"
"	bne	1b"
	: "=&r" (lockval), "=&r" (newval), "=&r" (tmp)
	: "r" (&lock->slock), "I" (1 << TICKET_SHIFT)
	: "cc");
	/*
	%0 		= lockval
	%1		= newval
	%2		= tmp
	%3		= &lock->slock
	%4		= 1 << TICKET_SHIFT
	
	上面的嵌入式汇编的作用翻译一下就是：
	1: 	lockval = *(&lock->slock);
		newval = lockval + (1 << TICKET_SHIFT);		// TICKET_SHIFT = 16; (1 << TICKET_SHIFT) = 1
		if ((*(&lock->slock) = newval) == TRUE){	// 即将 newval 写入 *(&lock->slock)是否成功
			tmp = 0;
		} else {
			tmp = x;
		}
		if (tmp != 0){
			goto 1;
		}
	翻译成傻瓜语就是 lock->tickets.next+1 是否更新成功，保证这个指令能够原子执行。
	注：lock->tickets 就是 lock->slock，因为他们是 union 结构。
	*/
	
	// 第一个进来的线程 lockval.tickets.next 一定是等于 lockval.tickets.owner 的
	// 因为 lockval = *lock;
	// 而 lock->tickets.next == lock->tickets.owner == 0
	// 后来的线程会进入 while 循环等待
	// 因为现在的 lock->tickets.next = 1 lock->tickets.owner = 0
	while (lockval.tickets.next != lockval.tickets.owner) {
		wfe();
		// 获取lock->tickets.owner的值
		// 如果上一个线程离开临界区，会将 lock->tickets.owner + 1
		// 那么这个等待线程的 owner 就会等于 它的 next，然后跳出循环
		lockval.tickets.owner = ACCESS_ONCE(lock->tickets.owner);
	}

	// 表示成功获取到自旋锁，进入临界区
	smp_mb();
}

///////////////////////////////////////////////////////////////////////////
// spin_unlock(&count_lock)
///////////////////////////////////////////////////////////////////////////
//~/linux-4.9.229/arch/arm/include/asm/spinlock.h
static inline void arch_spin_unlock(arch_spinlock_t *lock)
{
	// 非常简单，仅仅是将 owner + 1;
	smp_mb();
	lock->tickets.owner++;
	dsb_sev();
}