# $Id: Makefile 4726 2006-09-02 18:37:11Z nbd $
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

EXTRA_CFLAGS := -c -I. -I../include
LIBSHARED_OBJS := shutils.o wl.o wl_linux.o linux_timer.o base64.o strings.o files.o process.o crc32.o
LIBNVRAM_OBJS := crc32.o nvram_env.o flash_api.o nvram_linux.o ezp-lib.o

#NVDIR = $(BUILD_DIR)/ralink-nvram
SPECIAL_CFLAGS+=-I $(LINUX_DIR)/include
all: libnvram.so nvram

#%.o: %.c
#	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^

shutils.o: shutils.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^
wl.o: wl.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^
wl_linux.o: wl_linux.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^
linux_timer.o: linux_timer.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^
base64.o: base64.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^
strings.o: strings.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^
files.o: files.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^
process.o: process.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^
crc32.o: crc32.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^	
nvram_env.o: nvram_env.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(SPECIAL_CFLAGS) -o $@ $^	
flash_api.o: flash_api.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^	
nvram_linux.o: nvram_linux.c 
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^	
ezp-lib.o: ezp-lib.c 
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^	


libshared.so: $(LIBSHARED_OBJS)
	$(CC) -shared -o $@ $^

libnvram.so: $(LIBNVRAM_OBJS)
	$(CC) -shared -o $@ $^

nvram: main.o
	$(CC) -o $@ $^ -L. -lnvram

clean:
	rm -f *.o *.so nvram
