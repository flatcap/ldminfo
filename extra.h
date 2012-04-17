#ifndef _LDM_EXTRA_H_
#define _LDM_EXTRA_H_

#define CONFIG_LDM_DEBUG 1

#define PARTITION_META_INFO_VOLNAMELTH	64
#define PARTITION_META_INFO_UUIDLTH	16

struct partition_meta_info {
	u8 uuid[PARTITION_META_INFO_UUIDLTH];	/* always big endian */
	u8 volname[PARTITION_META_INFO_VOLNAMELTH];
};

#define DISK_MAX_PARTS 256
#define BDEVNAME_SIZE 32
typedef size_t sector_t;
struct parsed_partitions {
	struct block_device *bdev;
	char name[BDEVNAME_SIZE];
	struct {
		sector_t from;
		sector_t size;
		int flags;
		bool has_info;
		struct partition_meta_info info;
	} parts[DISK_MAX_PARTS];
	int next;
	int limit;
	bool access_beyond_eod;
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

//#include <linux/list.h>

#undef list_for_each
#define list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)

typedef unsigned short		umode_t;
struct inode {
	umode_t			i_mode;
	unsigned short		i_opflags;
	uid_t			i_uid;
	gid_t			i_gid;
	unsigned int		i_flags;

	const struct inode_operations	*i_op;
	struct super_block	*i_sb;
	struct address_space	*i_mapping;

	/* Stat data, not accessed from path walking */
	unsigned long		i_ino;
	/*
	 * Filesystems may only read i_nlink directly.  They shall use the
	 * following functions for modification:
	 *
	 *    (set|clear|inc|drop)_nlink
	 *    inode_(inc|dec)_link_count
	 */
	union {
		const unsigned int i_nlink;
		unsigned int __i_nlink;
	};
	dev_t			i_rdev;
	struct timespec		i_atime;
	struct timespec		i_mtime;
	struct timespec		i_ctime;
	unsigned short          i_bytes;
	blkcnt_t		i_blocks;
	loff_t			i_size;

	/* Misc */
	unsigned long		i_state;

	unsigned long		dirtied_when;	/* jiffies of first dirtying */

	struct list_head	i_wb_list;	/* backing dev IO list */
	struct list_head	i_lru;		/* inode LRU list */
	struct list_head	i_sb_list;
	unsigned int		i_blkbits;
	u64			i_version;
	const struct file_operations	*i_fop;	/* former ->i_op->default_file_ops */
	struct file_lock	*i_flock;
	struct list_head	i_devices;
	union {
		struct pipe_inode_info	*i_pipe;
		struct block_device	*i_bdev;
		struct cdev		*i_cdev;
	};
};

struct block_device {
	dev_t			bd_dev;  /* not a kdev_t - it's a search key */
	int			bd_openers;
	struct inode *		bd_inode;	/* will die */
	struct super_block *	bd_super;
	struct list_head	bd_inodes;
	void *			bd_claiming;
	void *			bd_holder;
	int			bd_holders;
	bool			bd_write_holder;
	struct block_device *	bd_contains;
	unsigned		bd_block_size;
	struct hd_struct *	bd_part;
	/* number of times partitions within this device have been opened. */
	unsigned		bd_part_count;
	int			bd_invalidated;
	struct gendisk *	bd_disk;
	struct request_queue *  bd_queue;
	struct list_head	bd_list;
	/*
	 * Private data.  You must have bd_claim'ed the block_device
	 * to use this.  NOTE:  bd_claim allows an owner to claim
	 * the same device multiple times, the owner must take special
	 * care to not mess up bd_private for that case.
	 */
	unsigned long		bd_private;

	/* The counter of freeze processes */
	int			bd_fsfreeze_count;
};

#endif /* _LDM_EXTRA_H_ */

