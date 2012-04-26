/*
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


#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ldminfo.h"
#include "common.h"

/**
 * copy_database - Save the LDM Database to a file
 */
void copy_database (int device, char *name, struct ldmdb *ldb)
{
	unsigned char *buffer;
	int fpart = 0;	/* Partition table + primary PRIVHEAD */
	int fdata = 0;	/* LDM Database */

	long long start = ldb->ph.config_start * 512;
	long long size  = ldb->ph.config_size  * 512;

	if (!name)
		return;

	buffer = kmalloc (size, 0);
	if (!buffer)
		return;

	sprintf ((char*) buffer, "%.58s.part", basename (name));
	fpart = open ((char*)buffer, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fpart < 0) {
		printf ("Couldn't open output file: %s\n", buffer);
		goto out;
	}

	sprintf ((char*) buffer, "%.58s.data", basename (name));
	fdata = open ((char*)buffer, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fdata < 0) {
		printf ("Couldn't open output file: %s\n", buffer);
		goto out;
	}

	/* First copy the partition table and primary PRIVHEAD to fpart */
	if (lseek (device, 0, SEEK_SET) < 0) {
		printf ("lseek to %d failed\n", 0);
		goto out;
	}
	if (read (device, buffer, 4096) < 4096) {
		printf ("Couldn't read the partition table and primary PRIVHEAD\n");
		goto out;
	}
	if (write (fpart, buffer, 4096) < 4096) {
		printf ("Couldn't write the partition table and primary PRIVHEAD\n");
		goto out;
	}

	if (lseek (device, start, SEEK_SET) < 0) {
		printf ("lseek to %lld failed\n", size);
		goto out;
	}
	if (read (device, buffer, size) < size) {
		printf ("Couldn't read the ldm database\n");
		goto out;
	}
	if (write (fdata, buffer, size) < size) {
		printf ("Couldn't write the ldm database\n");
		goto out;
	}

	printf ("Successfully copied the LDM data\n");
out:
	kfree (buffer);
	close (fpart);
	close (fdata);
}

