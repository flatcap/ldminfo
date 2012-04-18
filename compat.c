/**
 * ldminfo - Part of the Linux-NTFS project.
 *
 * Copyright (C) 2001-2012 Richard Russon <ldm@flatcap.org>
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
#include "common.h"

int device = 0;
int debug  = 0;

typedef size_t kdev_t;

int ldm_mem_alloc = 0;	/* Number of allocations */
int ldm_mem_free  = 0;	/* Number of frees */
int ldm_mem_size  = 0;	/* Memory allocated */
int ldm_mem_maxa  = 0;	/* Max memory allocated */
int ldm_mem_count = 0;	/* Number of memory blocks */
int ldm_mem_maxc  = 0;	/* Max memory blocks */

#define max(a, b)  (((a) > (b)) ? (a) : (b))

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
	//printf ("free   %p        in %s\n", objp - sizeof (int), fn);
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

void put_partition(struct parsed_partitions *p, int n, int from, int size)
{
	if (n < p->limit) {
		p->parts[n].from = from;
		p->parts[n].size = size;
	}
}

void put_dev_sector(Sector p)
{
	kfree (p.data);
	p.data = NULL;
}

void * read_part_sector(struct parsed_partitions *state, size_t n, Sector *sect)
{
	if (n >= state->size)
		return NULL;

	int size = 512;
	n *= size;

	sect->data = kmalloc (size, 0);

	if (lseek (device, n, SEEK_SET) < 0) {
		printk ("[CRIT] lseek to %lld failed\n", n);
	} else if (read (device, sect->data, size) < size) {
		printk ("[CRIT] read failed\n");
	}

	return sect->data;
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

size_t strlcat(char *dest, const char *src, size_t count)
{
	size_t dsize = strlen(dest);
	size_t len = strlen(src);
	size_t res = dsize + len;

	if (!dest)
		return -1;

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

