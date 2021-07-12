//linux-4.9.229/include/linux/slab.h

/**
 * kzalloc - allocate memory. The memory is set to zero.
 * @size: how many bytes of memory are required.
 * @flags: the type of memory to allocate (see kmalloc).
 */
static inline void *kzalloc(size_t size, gfp_t flags)
{
	return kmalloc(size, flags | __GFP_ZERO);
}

/**
 * kmalloc - allocate memory
 * @size: how many bytes of memory are required.
 * @flags: the type of memory to allocate.
 *
 * kmalloc is the normal method of allocating memory
 * for objects smaller than page size in the kernel.
 *
 * The @flags argument may be one of:
 *
 * %GFP_USER - Allocate memory on behalf of user.  May sleep.
 *
 * %GFP_KERNEL - Allocate normal kernel ram.  May sleep.
 *
 * %GFP_ATOMIC - Allocation will not sleep.  May use emergency pools.
 *   For example, use this inside interrupt handlers.
 *
 * %GFP_HIGHUSER - Allocate pages from high memory.
 *
 * %GFP_NOIO - Do not do any I/O at all while trying to get memory.
 *
 * %GFP_NOFS - Do not make any fs calls while trying to get memory.
 *
 * %GFP_NOWAIT - Allocation will not sleep.
 *
 * %__GFP_THISNODE - Allocate node-local memory only.
 *
 * %GFP_DMA - Allocation suitable for DMA.
 *   Should only be used for kmalloc() caches. Otherwise, use a
 *   slab created with SLAB_DMA.
 *
 * Also it is possible to set different flags by OR'ing
 * in one or more of the following additional @flags:
 *
 * %__GFP_COLD - Request cache-cold pages instead of
 *   trying to return cache-warm pages.
 *
 * %__GFP_HIGH - This allocation has high priority and may use emergency pools.
 *
 * %__GFP_NOFAIL - Indicate that this allocation is in no way allowed to fail
 *   (think twice before using).
 *
 * %__GFP_NORETRY - If memory is not immediately available,
 *   then give up at once.
 *
 * %__GFP_NOWARN - If allocation fails, don't issue any warnings.
 *
 * %__GFP_REPEAT - If allocation fails initially, try once more before failing.
 *
 * There are other flags available as well, but these are not intended
 * for general use, and so are not documented here. For a full list of
 * potential flags, always refer to linux/gfp.h.
 */
static __always_inline void *kmalloc(size_t size, gfp_t flags)
{
	// 判断 size 是不是编译时就已经初始化了，即是常量;
	// 如果是，进入 if
	if (__builtin_constant_p(size)) {
		if (size > KMALLOC_MAX_CACHE_SIZE)
			return kmalloc_large(size, flags);
#ifndef CONFIG_SLOB
		if (!(flags & GFP_DMA)) {
			int index = kmalloc_index(size);

			if (!index)
				return ZERO_SIZE_PTR;

			return kmem_cache_alloc_trace(kmalloc_caches[index],
					flags, size);
		}
#endif
	}
	// 如果不是则进入 __kmalloc
	return __kmalloc(size, flags);
}

//linux-4.9.229/mm/slab.c
void *__kmalloc(size_t size, gfp_t flags)
{
	return __do_kmalloc(size, flags, _RET_IP_);
}

/**
 * __do_kmalloc - allocate memory
 * @size: how many bytes of memory are required.
 * @flags: the type of memory to allocate (see kmalloc).
 * @caller: function caller for debug tracking of the caller
 */
static __always_inline void *__do_kmalloc(size_t size, gfp_t flags,
					  unsigned long caller)
{
	// 一个 kmem_cache，就表示一个高速缓存
	struct kmem_cache *cachep;
	void *ret;

	if (unlikely(size > KMALLOC_MAX_CACHE_SIZE))
		return NULL;
	// 根据 size，flags 得到高速缓存
	// 重点：	如何根据 size 和 flags 找到对应的高速缓存
	//			以及这个高速缓存链表是什么时候建立起来的
	cachep = kmalloc_slab(size, flags);
	if (unlikely(ZERO_OR_NULL_PTR(cachep)))
		return cachep;
	// 通过 cachep，从高速缓存中查到一个空闲的块，将其指针返回给 ret 
	ret = slab_alloc(cachep, flags, caller);

	// 一些检查
	kasan_kmalloc(cachep, ret, size, flags);
	trace_kmalloc(caller, ret,
		      size, cachep->size, flags);

	return ret;
}

//linux-4.9.229/mm/slab—_common.c
/*
 * Find the kmem_cache structure that serves a given size of
 * allocation
 */
struct kmem_cache *kmalloc_slab(size_t size, gfp_t flags)
{
	int index;

	if (size <= 192) {
		if (!size)
			return ZERO_SIZE_PTR;

		index = size_index[size_index_elem(size)];
	} else {
		if (unlikely(size > KMALLOC_MAX_CACHE_SIZE)) {
			WARN_ON(1);
			return NULL;
		}
		index = fls(size - 1);
	}

#ifdef CONFIG_ZONE_DMA
	if (unlikely((flags & GFP_DMA)))
		return kmalloc_dma_caches[index];

#endif
	// 关键点：这个数组是如何建立起来的
	// 看后面的 create_kmalloc_caches 函数
	return kmalloc_caches[index];
}

struct kmem_cache *kmalloc_caches[KMALLOC_SHIFT_HIGH + 1];

static inline int size_index_elem(size_t bytes)
{
	// 8 字节的倍数
	return (bytes - 1) / 8;
}

// linux-4.9.229/arch/arm/include/asm/bitops.h
/*
 * fls() returns zero if the input is zero, otherwise returns the bit
 * position of the last set bit, where the LSB is 1 and MSB is 32.
 */
static inline int fls(int x)
{
	if (__builtin_constant_p(x))
	       return constant_fls(x);

	return 32 - __clz(x);
}
/*
 * On ARMv5 and above those functions can be implemented around the
 * clz instruction for much better code efficiency.  __clz returns
 * the number of leading zeros, zero input will return 32, and
 * 0x80000000 will return 0.
 */
static inline unsigned int __clz(unsigned int x)
{
	// 返回最高位以上 0 的个数。
	unsigned int ret;

	asm("clz\t%0, %1" : "=r" (ret) : "r" (x));

	return ret;
}

// 建立告诉缓存
// ----------------------------------------------------------------------
/*
 * 建立高速缓存的索引和数组中的内存对象大小的对应关系
 * Create the kmalloc array. Some of the regular kmalloc arrays
 * may already have been created because they were needed to
 * enable allocations for slab creation.
 */
void __init create_kmalloc_caches(unsigned long flags)
{
	int i;

	// KMALLOC_SHIFT_LOW = 5，KMALLOC_SHIFT_HIGH = 22
	for (i = KMALLOC_SHIFT_LOW; i <= KMALLOC_SHIFT_HIGH; i++) {
		if (!kmalloc_caches[i])
			new_kmalloc_cache(i, flags);

		/*
		 * Caches that are not of the two-to-the-power-of size.
		 * These have to be created immediately after the
		 * earlier power of two caches
		 */
		if (KMALLOC_MIN_SIZE <= 32 && !kmalloc_caches[1] && i == 6)
			new_kmalloc_cache(1, flags);
		if (KMALLOC_MIN_SIZE <= 64 && !kmalloc_caches[2] && i == 7)
			new_kmalloc_cache(2, flags);
	}

	/* Kmalloc array is now usable */
	slab_state = UP;

#ifdef CONFIG_ZONE_DMA
	for (i = 0; i <= KMALLOC_SHIFT_HIGH; i++) {
		struct kmem_cache *s = kmalloc_caches[i];

		if (s) {
			int size = kmalloc_size(i);
			char *n = kasprintf(GFP_NOWAIT,
				 "dma-kmalloc-%d", size);

			BUG_ON(!n);
			kmalloc_dma_caches[i] = create_kmalloc_cache(n,
				size, SLAB_CACHE_DMA | flags);
		}
	}
#endif
}

// linux-4.9.229/mm/slab_common.c
/*
 * 除去第一个 {NULL,                      0}，linux 内核一共有 25 个高速缓存。
 * 
 * kmalloc_info[] is to make slub_debug=,kmalloc-xx option work at boot time.
 * kmalloc_index() supports up to 2^26=64MB, so the final entry of the table is
 * kmalloc-67108864.
 */
static struct {
	const char *name;
	unsigned long size;
} const kmalloc_info[] __initconst = {
	/*对象名*/				/*对象大小*/
	{NULL,                      0},		{"kmalloc-96",             96},
	{"kmalloc-192",           192},		{"kmalloc-8",               8},
	{"kmalloc-16",             16},		{"kmalloc-32",             32},
	{"kmalloc-64",             64},		{"kmalloc-128",           128},
	{"kmalloc-256",           256},		{"kmalloc-512",           512},
	{"kmalloc-1024",         1024},		{"kmalloc-2048",         2048},
	{"kmalloc-4096",         4096},		{"kmalloc-8192",         8192},
	{"kmalloc-16384",       16384},		{"kmalloc-32768",       32768},
	{"kmalloc-65536",       65536},		{"kmalloc-131072",     131072},
	{"kmalloc-262144",     262144},		{"kmalloc-524288",     524288},
	{"kmalloc-1048576",   1048576},		{"kmalloc-2097152",   2097152},
	{"kmalloc-4194304",   4194304},		{"kmalloc-8388608",   8388608},
	{"kmalloc-16777216", 16777216},		{"kmalloc-33554432", 33554432},
	{"kmalloc-67108864", 67108864}
};

static void __init new_kmalloc_cache(int idx, unsigned long flags)
{
	kmalloc_caches[idx] = create_kmalloc_cache(kmalloc_info[idx].name,
					kmalloc_info[idx].size, flags);
}

struct kmem_cache *__init create_kmalloc_cache(const char *name, size_t size,
				unsigned long flags)
{
	struct kmem_cache *s = kmem_cache_zalloc(kmem_cache, GFP_NOWAIT);

	if (!s)
		panic("Out of memory when creating slab %s\n", name);

	create_boot_cache(s, name, size, flags);
	list_add(&s->list, &slab_caches);
	s->refcount = 1;
	return s;
}

/* Create a cache during boot when no slab services are available yet */
void __init create_boot_cache(struct kmem_cache *s, const char *name, size_t size,
		unsigned long flags)
{
	int err;

	s->name = name;
	s->size = s->object_size = size;
	s->align = calculate_alignment(flags, ARCH_KMALLOC_MINALIGN, size);

	slab_init_memcg_params(s);

	err = __kmem_cache_create(s, flags);

	if (err)
		panic("Creation of kmalloc slab %s size=%zu failed. Reason %d\n",
					name, size, err);

	s->refcount = -1;	/* Exempt from merging for now */
}

// ----------------------------------------------------------------------
