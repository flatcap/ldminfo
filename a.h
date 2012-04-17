#ifndef _A_H_
#define _A_H_

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <endian.h>

#include "b.h"

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define be16_to_cpu(s) bswap_16(s)
#define be32_to_cpu(s) bswap_32(s)
#define be64_to_cpu(s) bswap_64(s)
#define cpu_to_le16
#define cpu_to_le32
#define cpu_to_le64
#else
#define be16_to_cpu
#define be32_to_cpu
#define be64_to_cpu
#define cpu_to_le16(s) bswap_16(s)
#define cpu_to_le32(s) bswap_32(s)
#define cpu_to_le64(s) bswap_64(s)
#endif

#define BUG() do { \
	printf("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __func__); \
	exit(1); \
} while (0)

#define BUG_ON(condition) do { if (condition) BUG(); } while(0)

#define KERN_CRIT       "<2>"   /* critical conditions                  */
#define KERN_ERR        "<3>"   /* error conditions                     */
#define KERN_INFO       "<6>"   /* informational                        */
#define KERN_DEBUG      "<7>"   /* debug-level messages                 */

#define get_unaligned(X) (*(X))

struct page {};
typedef struct {struct page *v;} Sector;

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

#define __printf(a, b) __attribute__((format(printf, a, b)))

#define MSDOS_LABEL_MAGIC		0xAA55
#define PAGE_SIZE			4096

struct partition {
	unsigned char boot_ind;		/* 0x80 - active */
	unsigned char head;		/* starting head */
	unsigned char sector;		/* starting sector */
	unsigned char cyl;		/* starting cylinder */
	unsigned char sys_ind;		/* What partition type */
	unsigned char end_head;		/* end head */
	unsigned char end_sector;	/* end sector */
	unsigned char end_cyl;		/* end cylinder */
	__le32 start_sect;	/* starting sector counting from 0 */
	__le32 nr_sects;		/* nr of sectors in partition */
} __attribute__((packed));

struct va_format {
	const char *fmt;
	va_list *va;
};

int printk (const char *format, ...);
unsigned char *read_dev_sector (struct block_device *bdev, unsigned long n, Sector *sect);
int hex_to_bin(char ch);
void * __kmalloc (size_t size, int flags, const char *fn);
void __kfree (const void *objp, const char *fn);
u64 get_unaligned_be64 (const u8 *ptr);
u32 get_unaligned_be32 (const u8 *ptr);
u16 get_unaligned_be16 (const u8 *ptr);
void *read_part_sector(struct parsed_partitions *state, size_t n, Sector *p);
void put_dev_sector(Sector p);
size_t strlcat(char *dest, const char *src, size_t count);
void put_partition(struct parsed_partitions *pp, int part_num, long long a, long long b);
void list_add(struct list_head *new, struct list_head *head);
void list_add_tail(struct list_head *new, struct list_head *head);
void INIT_LIST_HEAD(struct list_head *list);

#endif // _A_H_

