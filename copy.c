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


#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ldminfo.h"

#define BUFSIZE_BITS	12
#define BUFSIZE		(1 << BUFSIZE_BITS)

#define kmalloc(S,F)	malloc(S)
#define kfree(S)	free(S)

u64 get_unaligned_be64 (const u8 *ptr);

/**
 * copy_database - Save the LDM Database to a file
 */
void copy_database (char *file, int device, long long size)
{
	unsigned char *buffer;
	int fpart = 0;	/* Partition table + primary PRIVHEAD */
	int fdata = 0;	/* LDM Database */

	if (!file)
		return;

	buffer = kmalloc (BUFSIZE, GFP_KERNEL);
	if (!buffer)
		return;

	sprintf ((char*) buffer, "%.58s.part", basename (file));
	fpart = open ((char*)buffer, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fpart < 0) {
		printf ("Couldn't open output file: %s\n", buffer);
		goto out;
	}

	sprintf ((char*) buffer, "%.58s.data", basename (file));
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
	if (read (device, buffer, BUFSIZE) < BUFSIZE) {
		printf ("Couldn't read the partition table and primary PRIVHEAD\n");
		goto out;
	}
	if (write (fpart, buffer, BUFSIZE) < BUFSIZE) {
		printf ("Couldn't write the partition table and primary PRIVHEAD\n");
		goto out;
	}

	if (strncmp ((char*) (buffer + 3072), "PRIVHEAD", 8) == 0) {
		long long ph;
		ph  = get_unaligned_be64 (buffer + 3072 + 0x12B);
		ph += get_unaligned_be64 (buffer + 3072 + 0x133);
		ph <<= 9;
		if (ph != size) {
			printf ("The device appears to be %llu bytes long, but the PRIVHEAD reckons it's %llu bytes.\n", size, ph);
		}
	}

	size -= 1048576;	/* 1 MiB */

	if (lseek (device, size, SEEK_SET) < 0) {
		printf ("lseek to %lld failed\n", size);
		goto out;
	}

	while (read (device, buffer, BUFSIZE) > 0) {
		if (write (fdata, buffer, BUFSIZE) < 0) {
			printf ("Couldn't write to data file\n");
			goto out;
		}
	}

	printf ("Successfully copied the LDM data\n");
out:
	kfree (buffer);
	close (fpart);
	close (fdata);
}

