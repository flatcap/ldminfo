# Copyright (C) 2001-2012 Richard Russon

CC	= gcc
LD	= ld
LN	= ln -sf
RM	= rm -fr
MKDIR	= mkdir -p

SRC	= compat.c copy.c dump.c ldm.c ldminfo.c
HDR	= common.h ldm.h ldminfo.h ldm_req.h

DEPDIR	= .dep
OBJDIR	= .obj

OBJ	= $(SRC:%.c=$(OBJDIR)/%.o)

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

all:	$(DEPDIR) $(OBJDIR) $(OBJ) $(OUT) tags

$(DEPDIR) $(OBJDIR):
	$(MKDIR) $@

# strip the statics and forceably include ldm_req.h
$(OBJDIR)/ldm.o: ldm.c
	$(CC) $(CFLAGS) -DCONFIG_LDM_DEBUG -Dstatic="" -include ldm_req.h -c $< -o $@ && (					\
	$(CC) -MM $(CFLAGS) -c $< | sed 's/.*:/'$(OBJDIR)'\/\0/' > $(DEPDIR)/ldm.d;						\
	cp -f $(DEPDIR)/ldm.d $(DEPDIR)/ldm.d.tmp;										\
	sed -e 's/.*://' -e 's/\\$$//' < $(DEPDIR)/ldm.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(DEPDIR)/ldm.d;		\
	rm -f $(DEPDIR)/ldm.d.tmp)

$(OBJDIR)/%.o:	%.c
	$(CC) $(CFLAGS) -c $< -o $@ && (											\
	$(CC) -MM $(CFLAGS) -c $< | sed 's/.*:/'$(OBJDIR)'\/\0/' > $(DEPDIR)/$*.d;						\
	cp -f $(DEPDIR)/$*.d $(DEPDIR)/$*.d.tmp;										\
	sed -e 's/.*://' -e 's/\\$$//' < $(DEPDIR)/$*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(DEPDIR)/$*.d;		\
	rm -f $(DEPDIR)/$*.d.tmp)

#cmd_LD	= $(CC) -o $@ $(OBJ) $(LDFLAGS)

$(OUT): $(OBJ)
	$(CC) -o $(OUT) $(OBJ)

clean:
	$(RM) $(OUT) $(OBJDIR)/*

distclean: clean
	$(RM) $(DEPDIR) $(OBJDIR) tags *.img.data *.img.part

tags:	force
	ctags *.[ch]

force:

-include $(SRC:%.c=$(DEPDIR)/%.d)

