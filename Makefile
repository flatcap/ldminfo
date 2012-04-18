# Copyright (C) 2001 Richard Russon

CC	= gcc
LD	= ld
LN	= ln -sf
RM	= rm -f

SRC	= ldm.c compat.c copy.c dump.c ldminfo.c
OBJ	= $(SRC:.c=.o)

INFODEP	= ldm.o compat.o copy.o dump.o ldminfo.o

OUT	= ldminfo

CFLAGS	= -Wall -g
CFLAGS += -D_FILE_OFFSET_BITS=64
CFLAGS += -I.
CFLAGS += -Ilinux
#CFLAGS += -I$(KERNEL)/include
#CFLAGS += -I$(KERNEL)/fs/partitions

#CFLAGS += -Wstrict-prototypes
#CFLAGS += -fomit-frame-pointer
#CFLAGS += -fno-strict-aliasing
#CFLAGS += -pipe
#CFLAGS += -mpreferred-stack-boundary=4
#CFLAGS += -march=$(shell uname -m)

#LFLAGS += -m elf_i386 -r

all:	$(OUT)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

ldm.o:
	# strip the statics and forceably include ldm_req.h
	$(CC) $(CFLAGS) -Dstatic="" -include ldm_req.h -c ldm.c -o $@

ldminfo: $(INFODEP)
	$(CC) -o ldminfo $(INFODEP)

clean:
	$(RM) $(OUT) $(OBJ)

distclean: clean
	$(RM) tags

tags:	force
	ctags *.[ch]

force:

