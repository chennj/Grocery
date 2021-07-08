struct semaphore sema;

int hello_open(struct inode* p, struct file* f)
{
	// 临界区
	// --------------------------------------------------------
	down(&sema);	// 加锁
	if (open_count >= 1){
		up(&sema);	// 失败后释放锁
		printk(KERN_INFO"device is busy, hello_open fail\r\n");
		return -EBUSY;
	}
	open_count++;
	up(&sema);		// 成功后释放锁
	// --------------------------------------------------------
	
	printk(KERN_INFO"hello_open ok\r\n");
	return 0;
}

// semaphore.h
struct semaphore {
	raw_spinlock_t		lock;
	unsigned int		count;
	struct list_head	wait_list;
};

#define __SEMAPHORE_INITIALIZER(name, n)				\
{									\
	.lock		= __RAW_SPIN_LOCK_UNLOCKED((name).lock),	\
	.count		= n,						\
	.wait_list	= LIST_HEAD_INIT((name).wait_list),		\
}

#define DEFINE_SEMAPHORE(name)	\
	struct semaphore name = __SEMAPHORE_INITIALIZER(name, 1)

static inline void sema_init(struct semaphore *sem, int val)
{
	static struct lock_class_key __key;
	*sem = (struct semaphore) __SEMAPHORE_INITIALIZER(*sem, val);
	lockdep_init_map(&sem->lock.dep_map, "semaphore->lock", &__key, 0);
}

// semaphore.c
void down(struct semaphore *sem)
{
	unsigned long flags;

	raw_spin_lock_irqsave(&sem->lock, flags);	//自旋锁，系统开销大，不可被打断
	if (likely(sem->count > 0))
		sem->count--;
	else
		__down(sem);
	raw_spin_unlock_irqrestore(&sem->lock, flags);
}

static noinline void __sched __down(struct semaphore *sem)
{
	__down_common(sem, TASK_UNINTERRUPTIBLE, MAX_SCHEDULE_TIMEOUT);
}

static inline int __sched __down_common(struct semaphore *sem, long state,
								long timeout)
{
	struct task_struct *task = current;
	struct semaphore_waiter waiter;

	list_add_tail(&waiter.list, &sem->wait_list);
	waiter.task = task;
	waiter.up = false;

	for (;;) {
		if (signal_pending_state(state, task))
			goto interrupted;
		if (unlikely(timeout <= 0))
			goto timed_out;
		__set_task_state(task, state);
		raw_spin_unlock_irq(&sem->lock);
		timeout = schedule_timeout(timeout);	//让出cpu控制权
		raw_spin_lock_irq(&sem->lock);
		if (waiter.up)
			return 0;
	}

 timed_out:
	list_del(&waiter.list);
	return -ETIME;

 interrupted:
	list_del(&waiter.list);
	return -EINTR;
}

struct semaphore_waiter {
	struct list_head list;
	struct task_struct *task;
	bool up;
};

void up(struct semaphore *sem)
{
	unsigned long flags;

	raw_spin_lock_irqsave(&sem->lock, flags);
	if (likely(list_empty(&sem->wait_list)))
		sem->count++;
	else
		__up(sem);
	raw_spin_unlock_irqrestore(&sem->lock, flags);
}

static noinline void __sched __up(struct semaphore *sem)
{
	struct semaphore_waiter *waiter = list_first_entry(&sem->wait_list,
						struct semaphore_waiter, list);
	list_del(&waiter->list);
	waiter->up = true;
	wake_up_process(waiter->task);
}
