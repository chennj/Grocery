/*
 * low level task data that entry.S needs immediate access to.
 * __switch_to() assumes cpu_context follows immediately after cpu_domain.
 */
struct thread_info {
	unsigned long		flags;				/* low level flags */
	int			preempt_count;				/* 0 => preemptable, <0 => bug */
	mm_segment_t		addr_limit;			/* address limit */
	struct task_struct	*task;				/* main task structure */
	__u32			cpu;					/* cpu */
	__u32			cpu_domain;				/* cpu domain */
	struct cpu_context_save	cpu_context;	/* cpu context */
	__u32			syscall;				/* syscall number */
	__u8			used_cp[16];			/* thread used copro */
	unsigned long		tp_value[2];		/* TLS registers */
#ifdef CONFIG_CRUNCH
	struct crunch_state	crunchstate;
#endif
	union fp_state		fpstate __attribute__((aligned(8)));
	union vfp_state		vfpstate;
#ifdef CONFIG_ARM_THUMBEE
	unsigned long		thumbee_state;	/* ThumbEE Handler Base register */
#endif
};
