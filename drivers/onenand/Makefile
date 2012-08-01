include $(TOPDIR)/config.mk

LIB := libonenand.a

OBJS := onenand.o generic.o

ifeq ($(BOARD),smdk6400) 
OBJS += s3c_onenand.o
else ifeq ($(BOARD),smdk6410)
OBJS += s3c_onenand.o
else ifeq ($(BOARD),smdk6430)
OBJS += s3c_onenand.o
else
OBJS += onenand_base.o onenand_bbt.o
endif

all:	$(LIB)

$(LIB):	$(OBJS)
	$(AR) crv $@ $(OBJS)

#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(OBJS:.o=.c) > $@

sinclude .depend
