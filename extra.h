#ifndef _LDM_EXTRA_H_
#define _LDM_EXTRA_H_

#define CONFIG_LDM_DEBUG 1

typedef size_t sector_t;
struct parsed_partitions {
	struct block_device *bdev;
	char name[32];
	struct {
		sector_t from;
		sector_t size;
		int flags;
		bool has_info;
	} parts[256];
	int limit;
	char *pp_buf;
	struct ldmdb *ldb;
};


#define KDEV_MINOR_BITS		8
#define __mkdev(major,minor)	(((major) << KDEV_MINOR_BITS) + (minor))
#define mk_kdev(major, minor)	((kdev_t) { __mkdev(major,minor) } )

void * __kmalloc (size_t size, int flags, const char *fn);
void   __kfree   (const void *objp, const char *fn);

#define kmalloc(X,Y)	__kmalloc(X,Y,__FUNCTION__)
#define kfree(X)	__kfree(X,__FUNCTION__)

#ifdef page_cache_release
#undef page_cache_release
#endif

#ifdef __free_page
#undef __free_page
#endif

struct page;
void page_cache_release (struct page *page);
void __free_page (struct page *page);

struct inode {
	loff_t			i_size;
};

struct block_device {
	dev_t			bd_dev;  /* not a kdev_t - it's a search key */
	struct inode *		bd_inode;	/* will die */
	struct gendisk *	bd_disk;
};

#endif /* _LDM_EXTRA_H_ */

