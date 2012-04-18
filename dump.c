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
#include <string.h>

#include "ldminfo.h"

#ifndef prefetch
#define prefetch(X)	1
#endif

#define list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)
#define container_of(ptr, type, member) ({ const typeof( ((type *)0)->member ) *__mptr = (ptr); (type *)( (char *)__mptr - offsetof(type,member) );})
#define list_entry(ptr, type, member) container_of(ptr, type, member)

/**
 * print_guid -
 */
static char * print_guid (const u8 *block)
{
	static char buffer[40];

	memset (buffer, 0, sizeof (buffer));

	if (block) {
		sprintf (buffer, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
			block[0], block[1], block[2],  block[3],  block[4],  block[5],  block[6],  block[7],
			block[8], block[9], block[10], block[11], block[12], block[13], block[14], block[15]);
	}

	return buffer;
}

/**
 * dump_component -
 */
static void dump_component (struct vblk *vb)
{
	struct vblk_comp *comp = &vb->vblk.comp;

	printf ("0x%06X: <Component>\n",      vb->sequence);
	printf ("         Name        : %s\n",       vb->name);
	printf ("         Object Id   : 0x%04llx\n", vb->obj_id);
	printf ("         Parent Id   : 0x%04llx\n", comp->parent_id);
}

/**
 * dump_partition -
 */
static void dump_partition (struct vblk *vb)
{
	struct vblk_part *part = &vb->vblk.part;;

	printf ("0x%06X: <Partition>\n",      vb->sequence);
	printf ("         Name        : %s\n",       vb->name);
	printf ("         Object Id   : 0x%04llx\n", vb->obj_id);
	printf ("         Parent Id   : 0x%04llx\n", part->parent_id);
	printf ("         Disk Id     : 0x%04llx\n", part->disk_id);
	printf ("         Start       : 0x%llX\n",   (unsigned long long) part->start);
	printf ("         Size        : 0x%llX (%llu MB)\n", (unsigned long long) part->size, (unsigned long long) part->size >> 11);
	printf ("         Volume Off  : 0x%llX (%llu MB)\n", (unsigned long long) part->volume_offset, (unsigned long long) part->volume_offset >> 11);
}

/**
 * dump_disk -
 */
static void dump_disk (struct vblk *vb)
{
	struct vblk_disk *disk = &vb->vblk.disk;

	printf ("0x%06X: <Disk>\n",           vb->sequence);
	printf ("         Name        : %s\n",       vb->name);
	printf ("         Object Id   : 0x%04llx\n", vb->obj_id);
	printf ("         Disk Id     : %s\n",       print_guid (disk->disk_id));

	if (*disk->alt_name) {
		printf ("         AltName     : %s\n", disk->alt_name);
	}
}

/**
 * dump_diskgroup -
 */
static void dump_diskgroup (struct vblk *vb)
{
	struct vblk_dgrp *dgrp = &vb->vblk.dgrp;

	printf ("0x%06X: <DiskGroup>\n",      vb->sequence);
	printf ("         Name        : %s\n",       vb->name);
	printf ("         Object Id   : 0x%04llx\n", vb->obj_id);
	printf ("         GUID        : %s\n",       dgrp->disk_id);
}

/**
 * dump_volume -
 */
static void dump_volume (struct vblk *vb)
{
	struct vblk_volu *volu = &vb->vblk.volu;

	printf ("0x%06X: <Volume>\n",         vb->sequence);
	printf ("         Name        : %s\n",       vb->name);
	printf ("         Object Id   : 0x%04llx\n", vb->obj_id);
	printf ("         Volume state: %s\n",       volu->volume_state);
	printf ("         Size        : 0x%08llX (%llu MB)\n", (unsigned long long) volu->size, (unsigned long long) volu->size >> 11);
	printf ("         GUID        : %s\n", print_guid (volu->guid));

	if (*volu->drive_hint) {
		printf ("         Drive Hint  : %s\n", volu->drive_hint);
	}

	printf ("         Partition   : ");
	switch (volu->partition_type) {
		case 1: printf ("FAT12\n"); break;
		case 6: printf ("FAT16\n"); break;
		case 7: printf ("NTFS\n"); break;
		default: printf ("%d\n", volu->partition_type);
	}
}

/**
 * dump_vmdb -
 */
static int dump_vmdb (struct ldmdb *ldb)
{
	struct list_head *item;
	struct vmdb *vm = &ldb->vm;

	printf ("VMDB DATABASE HEADER:\n");

	printf ("Version            : %d/%d\n", vm->ver_major, vm->ver_minor);
	printf ("VBLK Size          : 0x%X\n", vm->vblk_size);
	printf ("Offset to VBLKs    : 0x%X\n", vm->vblk_offset);
	printf ("Number of VBLKs    : 0x%X\n", vm->last_vblk_seq - (vm->vblk_offset / vm->vblk_size));
	printf ("\n");

	printf ("VBLK DATABASE:\n");

	list_for_each (item, &ldb->v_comp) {
		dump_component (list_entry (item, struct vblk, list));
	}
	list_for_each (item, &ldb->v_part) {
		dump_partition (list_entry (item, struct vblk, list));
	}
	list_for_each (item, &ldb->v_disk) {
		dump_disk (list_entry (item, struct vblk, list));
	}
	list_for_each (item, &ldb->v_dgrp) {
		dump_diskgroup (list_entry (item, struct vblk, list));
	}
	list_for_each (item, &ldb->v_volu) {
		dump_volume (list_entry (item, struct vblk, list));
	}

	printf ("\n");

	return 0;
}

/**
 * dump_privhead -
 */
static int dump_privhead (struct privhead *ph)
{
	printf ("PRIVATE HEADER:\n");
	printf ("Version            : %d.%d\n", ph->ver_major, ph->ver_minor);
	if (ph->disk_id);
		printf ("Disk Id            : %s\n", print_guid (ph->disk_id));

	printf ("Logical disk start : 0x%llX\n",		(unsigned long long) ph->logical_disk_start);
	printf ("Logical disk size  : 0x%llX (%llu MB)\n",	(unsigned long long) ph->logical_disk_size, (unsigned long long) ph->logical_disk_size>>11);
	printf ("Configuration start: 0x%llX\n",		(unsigned long long) ph->config_start);
	printf ("Configuration size : 0x%llX (%llu MB)\n",	(unsigned long long) ph->config_size, (unsigned long long) ph->config_size>>11);
	printf ("\n");
	return 0;
}

/**
 * dump_tocblock -
 */
static int dump_tocblock (struct tocblock *toc)
{
	printf ("TOC\n");

	printf ("Bitmap name        : \"%s\"\n", toc->bitmap1_name);
	printf ("Log bitmap start   : 0x%llX\n", (unsigned long long) toc->bitmap1_start);
	printf ("Log bitmap size    : 0x%llX\n", (unsigned long long) toc->bitmap1_size);

	printf ("Bitmap name        : \"%s\"\n", toc->bitmap2_name);
	printf ("Log bitmap start   : 0x%llX\n", (unsigned long long) toc->bitmap2_start);
	printf ("Log bitmap size    : 0x%llX\n", (unsigned long long) toc->bitmap2_size);

	return 0;
}

/**
 * dump_disks -
 */
static int dump_disks (struct ldmdb *ldb)
{
	struct list_head *disk_item, *part_item;
	struct vblk      *disk_vb,   *part_vb;

	printf ("PARTITION LAYOUT:\n");
	printf ("\n");

	list_for_each (disk_item, &ldb->v_disk) {
		disk_vb = list_entry (disk_item, struct vblk, list);
		printf ("Disk %s:\n", disk_vb->name);

		list_for_each (part_item, &ldb->v_part) {
			struct vblk_part *part;

			part_vb = list_entry (part_item, struct vblk, list);

			part = &part_vb->vblk.part;

			if (disk_vb->obj_id != part->disk_id)
				continue;

			printf ("        %s ", part_vb->name);
			printf ("Offset: 0x%08llX ", part->start);
			printf ("Length: 0x%08llX (%llu MB)\n", (unsigned long long) part->size, (unsigned long long) part->size>>11);
		}
	}

	printf ("\n");
	return 0;
}

/**
 * dump_volumes -
 */
static int dump_volumes (struct ldmdb *ldb)
{
	struct list_head *volu_item, *comp_item, *part_item;
	struct vblk      *volu_vb,   *comp_vb,   *part_vb;

	printf ("VOLUME DEFINITIONS:\n");
	printf ("\n");

	list_for_each (volu_item, &ldb->v_volu) {
		volu_vb = list_entry (volu_item, struct vblk, list);

		printf ("%s ", volu_vb->name);
		printf ("Size: 0x%08llX (%llu MB)\n", (unsigned long long) volu_vb->vblk.volu.size, (unsigned long long) volu_vb->vblk.volu.size >> 11);

		list_for_each (comp_item, &ldb->v_comp) {
			comp_vb = list_entry (comp_item, struct vblk, list);

			if (volu_vb->obj_id != comp_vb->vblk.comp.parent_id)
				continue;

			printf ("    %s\n", comp_vb->name);

			list_for_each (part_item, &ldb->v_part) {
				part_vb = list_entry (part_item, struct vblk, list);

				if (comp_vb->obj_id != part_vb->vblk.part.parent_id)
					continue;

				printf ("      %s   ",             part_vb->name);
				printf ("VolumeOffset: 0x%08llX ", part_vb->vblk.part.volume_offset);
				printf ("Offset: 0x%08llX ",       part_vb->vblk.part.start);
				printf ("Length: 0x%08llX\n",      part_vb->vblk.part.size);
			}
		}
	}

	printf ("\n");
	return 0;
}

/**
 * dump_database -
 */
void dump_database (char *name, struct ldmdb *ldb)
{
	printf ("Device: %s\n\n", name);

	dump_privhead (&ldb->ph);
	dump_tocblock (&ldb->toc);
	dump_vmdb (ldb);
	dump_disks (ldb);
	dump_volumes (ldb);
}

