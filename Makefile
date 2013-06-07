CLI_FILENAME = nvram

CLI_PATH=$(LITEON_APP_PATH)/$(CLI_FILENAME)

include $(LITEON_APP_PATH)/rules.gcc

LIB_VERMAJOR = 0
LIB_VERMINOR = 1
LIB_FILENAME = libnvram.so

LIB_CFLAGS  = $(CFLAGS) -shared -fPIC

CLI_CFLAGS  = $(CFLAGS)
CLI_LDFLAGS = $(LDFLAGS)

CLI_OBJ = cli.o
LIB_OBJ = crc.o nvram.o

all: cli libnvram

cli: libnvram
	$(CC) $(CLI_CFLAGS) -c -o cli.o cli.c
	$(CC)  $(CLI_LDFLAGS) $(CLI_OBJ) \
	$(LIB_FILENAME).$(LIB_VERMAJOR).$(LIB_VERMINOR) \
		-o $(CLI_FILENAME)

cli.o: cli.c
	$(CC) $(CLI_CFLAGS) -c -o $@ $<

libnvram:
	$(CC) $(LIB_CFLAGS) -c -o crc.o crc.c
	$(CC) $(LIB_CFLAGS) -c -o nvram.o nvram.c
	$(CC) $(LIB_CFLAGS)  \
		-o $(LIB_FILENAME).$(LIB_VERMAJOR).$(LIB_VERMINOR) $(LIB_OBJ)

clean:
	rm -f $(CLI_FILENAME) $(LIB_FILENAME)* *.o

install:
	$(call install_program,$(CLI_PATH)/$(CLI_FILENAME),$(APP_INSTALL_ROOT))
	$(call install_library,$(LIB_FILENAME).$(LIB_VERMAJOR).$(LIB_VERMINOR))
