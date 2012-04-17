/**
 * ldminfo - Part of the Linux-NTFS project.
 *
 * Copyright (C) 2001 Richard Russon <ldm@flatcap.org>
 *
 * Documentation is available at http://linux-ntfs.sourceforge.net/ldm
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the Linux-NTFS source
 * in the file COPYING); if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>

#include "ldminfo.h"

int device = 0;
int debug  = 0;

struct page {
	int count;
	unsigned long private;
};
typedef struct {struct page *v;} Sector;
typedef size_t kdev_t;
#define atomic_read(X)	(X)
typedef size_t sector_t;

struct buffer_head {
	unsigned long b_state;		/* buffer state bitmap (see above) */
	struct buffer_head *b_this_page;/* circular list of page's buffers */
	struct page *b_page;		/* the page this bh is mapped to */

	//sector_t b_blocknr;		/* start block number */
	size_t b_size;			/* size of mapping */
	char *b_data;			/* pointer to data within the page */

	struct block_device *b_bdev;
	//bh_end_io_t *b_end_io;		/* I/O completion */
	void *b_private;		/* reserved for b_end_io */
	struct list_head b_assoc_buffers; /* associated with another mapping */
	struct address_space *b_assoc_map;	/* mapping this buffer is
						   associated with */
	//atomic_t b_count;		/* users using this buffer_head */
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

typedef size_t sector_t;
struct parsed_partitions {
	struct block_device *bdev;
	char name[32];
	struct {
		sector_t from;
		sector_t size;
		int flags;
	} parts[256];
	bool access_beyond_eod;
	char *pp_buf;
	struct ldmdb *ldb;
};

/* external dependencies */
void *	malloc	(size_t size);
void	free	(void *ptr);

/* general kernel functions */
int printk (const char *fmt, ...);
void __brelse (struct buffer_head *buf);

int ldm_mem_alloc = 0;	/* Number of allocations */
int ldm_mem_free  = 0;	/* Number of frees */
int ldm_mem_size  = 0;	/* Memory allocated */
int ldm_mem_maxa  = 0;	/* Max memory allocated */
int ldm_mem_count = 0;	/* Number of memory blocks */
int ldm_mem_maxc  = 0;	/* Max memory blocks */

#define max(a, b)  (((a) > (b)) ? (a) : (b))

#define kmalloc(X,Y)	__kmalloc(X,Y,__FUNCTION__)
#define kfree(X)	__kfree(X,__FUNCTION__)

void put_page (struct page *p)
{
	printf ("NOTIMPL: %s\n", __func__);
}

void * __kmalloc (size_t size, int flags, const char *fn)
{
	void *ptr = malloc (size + sizeof (int));
	//printf ("malloc %p %6zu in %s\n", ptr, size, fn);
	ldm_mem_alloc++;
	ldm_mem_size += size;
	ldm_mem_maxa = max (ldm_mem_maxa, ldm_mem_size);
	ldm_mem_count++;
	ldm_mem_maxc = max (ldm_mem_maxc, ldm_mem_count);
	*((int *)ptr) = size;
	return (ptr + sizeof (int));
}

void __kfree (const void *objp, const char *fn)
{
	//printf ("free   %p        in %s\n", objp, fn);
	if (!objp)
		return;
	ldm_mem_free++;
	ldm_mem_count--;
	ldm_mem_size -= *((int *)(objp - sizeof (int)));
	free ((void *)(objp - sizeof (int)));
}

int printk (const char *fmt, ...)
{
	static int ignore;
	char buf[1024];
	va_list args;

	va_start (args, fmt);
	vsnprintf (buf, sizeof (buf), fmt, args);
	va_end (args);

	if ((!debug) && (!strcmp (buf, " [LDM]")))
		return 0;

	if ((!debug) && (buf[0] == ' ') && isdigit(buf[1]))
		return 0;

	if (ignore) {
		ignore = (strchr (buf, '\n') == NULL);
		return 0;
	}

	if ((buf[0] == '<') && (buf[2] == '>'))
		if (debug || buf[1] != '7')
			printf ("%s", buf+3);
		else
			ignore = (strchr (buf, '\n') == NULL);
	else
		printf ("%s", buf);

	return 0;
}

void __brelse (struct buffer_head * buf)
{
	if (buf) {
		kfree (buf->b_data);
		kfree (buf);
	}
}

struct buffer_head * ldm_bread (kdev_t dev, int block, int size)
{
	struct buffer_head *bh;
	long long offset;

	offset = (((long long) block) * size);

	bh = kmalloc (sizeof (*bh), 0);
	if (bh) {
		memset (bh, 0, sizeof (*bh));

		bh->b_data = kmalloc (size, 0);

		if (lseek (device, offset, SEEK_SET) < 0)
			printk ("[CRIT] lseek to %lld failed\n", offset);
		else if (read (device, bh->b_data, size) < size)
			printk ("[CRIT] read failed\n");
		else
			goto bread_end;

		kfree (bh);
		bh = NULL;
	}
bread_end:
	return bh;
}

unsigned char *read_dev_sector (struct block_device *bdev, unsigned long n, Sector *sect)
{
	struct page        *pg = NULL;
	struct buffer_head *bh = NULL;

	if (!bdev || !sect)
		return NULL;

	pg = (struct page *) kmalloc (sizeof (*pg), 0);
	if (!pg)
		return NULL;

	memset (pg, 0, sizeof (*pg));
	//atomic_inc (&pg->count);
	pg->count++;

	bh = ldm_bread ((*((kdev_t*) (&bdev->bd_dev))), n, 512);
	if (!bh) {
		put_page (pg);
		return NULL;
	}

	sect->v = pg;
	pg->private = (unsigned long) bh;
	return (unsigned char *) bh->b_data;
}

void __free_pages(struct page *page, unsigned int order)
{
	//atomic_dec (&page->count);
	page->count--;
	//if (atomic_read (&page->count) < 1) {
	if (page->count < 1) {
		if ((page->private) && (((struct buffer_head*)(page->private))->b_data))
			kfree (((struct buffer_head*)(page->private))->b_data);
		if (page->private);
			kfree ((void*)(page->private));
		kfree (page);
	}
}

void page_cache_release (struct page *page)
{
	__free_pages (page, 0);
}

void __free_page (struct page *page)
{
	__free_pages((page), 0);
}

#if 0
void put_partition(struct parsed_partitions *p, int n, int from, int size)
{
	if (n < p->limit) {
		p->parts[n].from = from;
		p->parts[n].size = size;
	}
}
#endif

void put_partition(struct parsed_partitions *pp, int part_num, long long a, long long b)
{
	printf ("NOTIMPL: %s\n", __func__);
}

void put_dev_sector(Sector p)
{
	printf ("NOTIMPL: %s\n", __func__);
}

struct hd_struct {
	sector_t nr_sects;
};

struct gendisk {
	struct hd_struct part0;
};

sector_t get_capacity(struct gendisk *disk)
{
	return disk->part0.nr_sects;
}

void *read_part_sector(struct parsed_partitions *state, sector_t n, Sector *p)
{
	if (n >= get_capacity(state->bdev->bd_disk)) {
		state->access_beyond_eod = 1;
		return NULL;
	}
	return read_dev_sector(state->bdev, n, p);
}

int hex_to_bin(char ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return ch - '0';
	ch = tolower(ch);
	if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	return -1;
}

#define BUG() do { \
	printf("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __func__); \
	exit(1); \
} while (0)

#define BUG_ON(condition) do { if (condition) BUG(); } while(0)

size_t strlcat(char *dest, const char *src, size_t count)
{
	size_t dsize = strlen(dest);
	size_t len = strlen(src);
	size_t res = dsize + len;

	/* This would be a bug */
	BUG_ON(dsize >= count);

	dest += dsize;
	count -= dsize;
	if (len >= count)
		len = count-1;
	memcpy(dest, src, len);
	dest[len] = 0;
	return res;
}

u64 get_unaligned_be64 (const u8 *ptr)
{
	return be64toh(*(u64*)ptr);
}

u32 get_unaligned_be32 (const u8 *ptr)
{
	return be32toh(*(u32*)ptr);
}

u16 get_unaligned_be16 (const u8 *ptr)
{
	return be16toh(*(u16*)ptr);
}

void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}
void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}
void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}
// why two different inits?

