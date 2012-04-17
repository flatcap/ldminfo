#ifndef __LDMINFO_H_
#define __LDMINFO_H_

#include "ldm.h"

struct list_head;

extern int device;
extern int debug;

extern int ldm_mem_alloc;
extern int ldm_mem_free;
extern int ldm_mem_size;
extern int ldm_mem_maxa;
extern int ldm_mem_count;
extern int ldm_mem_maxc;

void dump_database (char *name, struct ldmdb *ldb);
void copy_database (char *file, int fd, long long size);
void ldm_free_vblks (struct list_head *vb);

#endif // __LDMINFO_H_

