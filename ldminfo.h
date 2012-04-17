#ifndef __LDMINFO_H_
#define __LDMINFO_H_

#if 0
#include <linux/version.h>
#include <linux/kdev_t.h>
#include <linux/blkdev.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/list.h>
#include <linux/buffer_head.h>
#include <features.h>
#endif

//#define __ssize_t_defined
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ldm.h"

#define KERN_EMERG      "<0>"   /* system is unusable                   */
#define KERN_ALERT      "<1>"   /* action must be taken immediately     */
#define KERN_CRIT       "<2>"   /* critical conditions                  */
#define KERN_ERR        "<3>"   /* error conditions                     */
#define KERN_WARNING    "<4>"   /* warning conditions                   */
#define KERN_NOTICE     "<5>"   /* normal but significant condition     */
#define KERN_INFO       "<6>"   /* informational                        */
#define KERN_DEBUG      "<7>"   /* debug-level messages                 */

#define LDM_CRIT	KERN_CRIT
#define LDM_ERR		KERN_ERR
#define LDM_INFO	KERN_INFO
#define LDM_DEBUG	KERN_DEBUG

extern int device;
extern int debug;

extern int ldm_mem_alloc;
extern int ldm_mem_free;
extern int ldm_mem_size;
extern int ldm_mem_maxa;
extern int ldm_mem_count;
extern int ldm_mem_maxc;

struct stat64;

void dump_database (char *name, struct ldmdb *ldb);
void copy_database (char *file, int fd, long long size);
void ldm_free_vblks (struct list_head *vb);

int		open64	(const char *file, int oflag, ...);
long long	lseek64 (int fd, long long offset, int whence);
int		stat64  (const char *file, struct stat64 *buf);
char *		basename(const char *filename);

#endif // __LDMINFO_H_

