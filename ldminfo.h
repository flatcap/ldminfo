/*
 * Copyright (c) 2001-2012 Richard Russon (FlatCap)
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __LDMINFO_H_
#define __LDMINFO_H_

#include "common.h"
#include "ldm.h"

struct list_head;

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

