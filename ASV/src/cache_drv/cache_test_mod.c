#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/version.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/io.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/dma-buf.h>
#include <asm/cacheflush.h>


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define preempt_get_count preempt_count
#endif

/*
 * #> insmod cache_test_mod.ko param_ca_test_kb=1024 param_debug_on=0 param_ca_test_cnt=10  ...
 */

static long param_ca_test_kb = 1024;	/* kbyte*/
static long param_ca_size_kb = 1024;	/* byte*/
static int param_ca_test_cnt = 1;
static int param_debug_on = 0;
static int param_ca_non_op = 0;

module_param(param_ca_test_kb, long, 0);
module_param(param_ca_size_kb, long, 0);
module_param(param_ca_test_cnt, int, 0);
module_param(param_debug_on, int, 0);
module_param(param_ca_non_op, int, 0);

#define	CA_DEBUG(cond, msg...) 	do { if (cond) printk(msg); } while (0)

struct cache_buffer {
	unsigned long caddr;
	unsigned long ncaddr;
	unsigned long paddr;
	long length;
	struct device *dev;
};

static struct miscdevice cache_dev;
static struct cache_buffer *ca_buffer = NULL;

static void *phys_to_noncache(unsigned long phys, unsigned long length)
{
	unsigned long num_pages = length >> PAGE_SHIFT;
	struct page **pages;
	void *virt = NULL;
	int i = 0;

	pages = kzalloc(num_pages * sizeof(struct page *), GFP_KERNEL);
	if (!pages)
		return NULL;

	for (i = 0; i < num_pages; i++)
		pages[i] = pfn_to_page((phys >> PAGE_SHIFT) + i);

	/*
	 * pgprot_writecombine(PAGE_KERNEL)
	 * pgprot_dmacoherent (PAGE_KERNEL)
	 * pgprot_noncached(PAGE_KERNEL)
	*/
	virt = vmap(pages, num_pages, VM_MAP, pgprot_noncached(PAGE_KERNEL));
	kfree(pages);

	return virt;
}

static void release_noncache(void *addr)
{
	vunmap(addr);
}

static int cache_alloc_continuous(struct cache_buffer **ca_buffer, int length)
{
	struct cache_buffer *cab = NULL;
	int debug = param_debug_on;

 	if (0 == length) {
		pr_err("Fail, request cache buffer size is zero ...\n");
 		return -1;
	}

	cab = kzalloc(sizeof(struct cache_buffer), GFP_KERNEL);
	if (NULL == cab) {
		pr_err("Fail, alloc cache buffer info struct size %d ...\n", sizeof(struct cache_buffer));
		return -1;
	}

	CA_DEBUG(debug, "[page offeset : 0x%08lx, phys offset   : 0x%08lx]\n",
		PAGE_OFFSET, PHYS_OFFSET);
	CA_DEBUG(debug, "[high memory  : 0x%08lx, vmalloc start : 0x%08lx]\n",
		(ulong)high_memory, VMALLOC_START);

	cab->caddr = (unsigned int)kzalloc(length, GFP_KERNEL);
	if (!((unsigned long)(cab->caddr) >= PAGE_OFFSET &&
		  (unsigned long)(cab->caddr) < (unsigned long)high_memory) ) {
		pr_err("Fail, kzalloc 0x%lx is high memory(0x%p) or non continuous(vmalloc:0x%08lx)\n",
			cab->caddr, high_memory, VMALLOC_START);
		kfree(cab);
		return -1;
	}

	cab->paddr  = (unsigned long)virt_to_phys((void*)cab->caddr);
	cab->ncaddr = (unsigned long)phys_to_noncache(cab->paddr, length);
	cab->dev = cache_dev.this_device;
	cab->length = length;

	if (!cab->ncaddr) {
		kfree((void*)cab->caddr);
		kfree(cab);
		return -1;
	}

	CA_DEBUG(debug, "memory = ca:0x%08lx, nc:0x%08lx, phys:0x%08lx, len:%d\n",
		cab->caddr, cab->ncaddr, cab->paddr, length);

	*ca_buffer = cab;
	return 0;
}

static void cache_free_continuous(struct cache_buffer *ca_buffer)
{
	struct cache_buffer *cab = ca_buffer;
	if (cab) {
		if (cab->caddr)
			kfree((void*)cab->caddr);
		if (cab->ncaddr)
			release_noncache((void*)cab->ncaddr);
		kfree(cab);
	}
}

/*
 * Non Cached (DMA) -> Cached
 *			invalidate and compare cache memory with noncache memory
 * return:
 *			-1	: Envalid cache status
 *			 0	: Success
 */
static long cache_test_invalidate(struct device *dev,
				unsigned long *cached, unsigned long *nocache,
				long cache_size, long total_size, bool invalidate)
{
	int debug = param_debug_on;
	volatile char temp = 0;
	char *ca = (char*)cached, *nc = (char*)nocache;
	long i = 0, ret = 0;

	CA_DEBUG(debug, "Invalidate: CPU.%d", raw_smp_processor_id());

	/*
	 * step 1. load cache : total length
	 */
	for(i = 0; total_size > i; i++)
		temp = (volatile char)ca[i];

	/*
	 * step 2. clear cache data : total length
	 * note> if set cached memory, that memory is dirty
	 */
	memset((void*)ca, 0xaa, total_size);

	/*
	 * step 3. cache clean(flush): total length
	 */
	dma_map_single(dev, (void*)ca, total_size, DMA_TO_DEVICE);

	/*
	 * setp 4. set non_cache: total length
	 * assume DMA write to memory
	 */
	for(i = 0; total_size > i; i++)
		nc[i] = (i+1)%256;

	/*
	 * setp 5. cache invalidate: cache length
	 * invalidate cache before read cache.
	 */
	if (invalidate)
		dma_map_single(dev, (void*)ca, cache_size, DMA_FROM_DEVICE);


	/*
	 * verify. check cached data [Non Cached (DMA) -> Cached]
	 * 		   maybe only equal cache size
	 */
	for(i = 0; total_size > i; i++) {
		if ((i+1)%256 != ca[i]) {
			if (cache_size > i)
				ret = -1;
			break;
		}
	}

	CA_DEBUG(debug, "[%s] %s - (%ld:%ld)\n", invalidate?"ON ":"OFF",
		ret?(invalidate?"Fail":"Done"):"Done", cache_size, i);

	return ret;
}

/*
 * Cached -> Non Cached (DMA)
 *			flush and compare cache memory with noncache memory
 * return:
 *			-1	: Envalid cache status
 *			 0	: Success
 */
static long cache_test_flush (struct device *dev,
				unsigned long *cached, unsigned long *nocache,
				long cache_size, long total_size, bool flush)
{
	int debug = param_debug_on;
	char *ca = (char*)cached, *nc = (char*)nocache;
	volatile char temp = 0;
	long i = 0, ret = 0;
	unsigned long flags;

	CA_DEBUG(debug, "Flushcache: CPU.%d", raw_smp_processor_id());

	local_irq_save(flags);

	/*
	 * step 1. invalidate and load to cache from memory
	 */
	dma_map_single(dev, (void*)ca, total_size, DMA_FROM_DEVICE);

	for(i = 0; total_size > i; i++)
		temp = (volatile char)ca[i];

	/* step 2. check cache vs memory */
	for(i = 0; total_size > i; i++) {
		if (ca[i] != nc[i]) {
			pr_err("Fail: invalidate cache to test flush [total %ld: step %ld]\n",
				total_size, i);
			return -1;
		}
	}

	/*
	 * step 3. clear non_cache data: total length
	 * NOTE
	 * 	 	set non cache area before set cache area.
	 * 		1. Set CA -> "audto cpu flsuh CA to NC"	-> 2. Set NC
	 *   	-> "3. CA Flush: auto flushed DATA is not flush"
	 */
	for(i = 0; total_size > i; i++)
		nc[i] = 0xaa;

	/*
	 * step 4. set cache data: total length
	 */
	for(i = 0; total_size > i; i++)
		ca[i] = (i+1)%256;

	/*
	 * step 5. cache flush
	 * flush_cache_all(), outer_flush_all()
	 */
	if (flush)
		dma_map_single(dev, (void*)ca, cache_size, DMA_TO_DEVICE);

	local_irq_restore(flags);

	/*
	 * verify. check non_cache data [Cached -> Non Cached (DMA)]
	 */
	for(i = 0; total_size > i; i++) {
		if ((i+1)%256 != nc[i]) {
			if (cache_size > i)
				ret = -1;
			break;
		}
	}

	CA_DEBUG(debug, "[%s] %s - (%ld:%ld)\n", flush?"ON ":"OFF",
		ret?(flush?"Fail":"Done"):"Done", cache_size, i);

	return ret;
}

static int cache_mod_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int cache_mod_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t cache_mod_write(struct file *filp, const char *buf, size_t count, loff_t *ppos)
{
	struct cache_buffer *cab = ca_buffer;
	int csize = (param_ca_size_kb * 1024);
	int tests = param_ca_test_cnt;
	bool op = param_ca_non_op ? false : true;

	unsigned long *caddr, *naddr;
	long length = 0;
	int ret = 0, i = 0;

	if (NULL == cab)
		return -1;

	preempt_disable();
	caddr  = (unsigned long *)cab->caddr;
	naddr  = (unsigned long *)cab->ncaddr;
	length = cab->length;

	for (i = 0; tests > i; i++) {
		ret = cache_test_flush(cab->dev, caddr, naddr, csize, length, op);
		if (0 > ret)
			break;

		ret = cache_test_invalidate(cab->dev, caddr, naddr, csize, length, op);
		if (0 > ret)
			break;
	}
	preempt_enable();
	return ret;
}

static struct file_operations cache_mod_fops = {
	.owner 			= THIS_MODULE,
	.open 			= cache_mod_open,
	.release 		= cache_mod_release,
	.write 			= cache_mod_write,
};

static struct miscdevice cache_dev = {
	.minor 	= MISC_DYNAMIC_MINOR,
	.name 	= "cache_test",
	.fops 	= &cache_mod_fops
};

static int __init cache_mod_init(void)
{
	int ret = misc_register(&cache_dev);
	if (ret) {
		pr_err(KERN_ERR "Fail: misc register ....\n");
		return ret;
	}

	if (param_ca_size_kb > param_ca_test_kb)	{
		pr_err(KERN_ERR "Fail: cache size %ld less than buffer size %ld ....\n",
			param_ca_size_kb, (param_ca_test_kb*1024));
		return ret;
	}

	ret = cache_alloc_continuous(&ca_buffer, (param_ca_test_kb*1024));
	if (0 > ret)
		return ret;

	pr_info("/dev/%s : %d.%d [%s]\n",
		cache_dev.name, MISC_MAJOR, cache_dev.minor, ret?"FAIL":"DONE");

	return ret;
}

static void __exit cache_mod_exit(void)
{
	cache_free_continuous(ca_buffer);
	misc_deregister(&cache_dev);
}

module_init(cache_mod_init);
module_exit(cache_mod_exit);

MODULE_LICENSE("GPL");
