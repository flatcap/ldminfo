#ifndef _B_H_
#define _B_H_

#include <stddef.h>
#include <sys/types.h>

typedef unsigned char       u8;
typedef unsigned short int  u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;
typedef short int           __le16;
typedef int                 __le32;

typedef int bool;
#define true  1
#define false 0

#define GFP_KERNEL	1
#define kmalloc(X,Y)	__kmalloc(X,Y,__func__)
#define kfree(X)	__kfree(X,__func__)

void * __kmalloc (size_t size, int flags, const char *fn);
void   __kfree   (const void *objp, const char *fn);

struct inode {
	loff_t			i_size;
};

struct inode;

struct block_device {
	dev_t			bd_dev;
	struct inode *		bd_inode;
};

#define container_of(ptr, type, member) ({ const typeof( ((type *)0)->member ) *__mptr = (ptr); (type *)( (char *)__mptr - offsetof(type,member) );})
#define list_entry(ptr, type, member) container_of(ptr, type, member)
#define list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)
#define list_for_each_safe(pos, n, head) for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

struct list_head {
	struct list_head *next, *prev;
};

struct parsed_partitions {
	struct block_device *bdev;
	char name[32];
	struct {
		size_t from;
		size_t size;
		int flags;
		bool has_info;
	} parts[256];
	int limit;
	char *pp_buf;
	struct ldmdb *ldb;
	size_t rich_size;
};

#endif // _B_H_

