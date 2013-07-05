#ifndef _nvram_h_
#define _nvram_h_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <linux/limits.h>

#define xstr(s) str(s)
#define str(s)  #s 

#define EZP_PROD_VERSION "0.0.0_ALPHA"
#define EZP_PROD_CAT "2"
#define EZP_PROD_SUBCAT "1"
#define EZP_PROD_SUBSUBCAT "1"
#define EZP_PROD_FW_VERSION "V1.00(BWQ.1)C0"

/* TRACE */
#define TRACE(msg) \
	printf("%s(%i) in %s(): %s\n", \
		__FILE__, __LINE__, __FUNCTION__, msg ? msg : "?")

/* rule buf len for NVRAM */
#define NVRAM_TMP_LEN 256

/* Staging file for NVRAM */
#define NVRAM_MTD_NAME		"nvram"
#define NVRAM_STAGING		"/tmp/.nvram"
#define NVRAM_RO			1
#define NVRAM_RW			0

/* Helper macros */
#define NVRAM_ARRAYSIZE(a)	sizeof(a)/sizeof(a[0])
#define	NVRAM_ROUNDUP(x, y)	((((x)+((y)-1))/(y))*(y))

/* NVRAM constants */
#define NVRAM_OFFSET		0
#define NVRAM_SPACE			0x8000
#define NVRAM_MAGIC			0x48534C46	/* 'FLSH' */
#define NVRAM_VERSION		1

/* magic, len, crc8 to be skipped */
#define NVRAM_CRC_START_POSITION	9 

/* None of specific NVRAM options. */
#define NVRAM_NONE 0x00
/* NOT allowed to be disclosed (e.g. a config file). */
#define NVRAM_PRIVATE 0x01
/* NOT allowed to be overwritten (e.g. a license key). If the value is
 * empty, it could be written for once.*/
#define NVRAM_PROTECTED 0x02
/* For temporary use. */
#define NVRAM_TEMP 0x04
/* Customized by the authorized program. */
#define NVRAM_CUSTOMIZED 0x08
/* NVRAM could be empty. */
#define NVRAM_EMPTY 0x10
/* NVRAM value should be set by *_default if this value is empty . */
#define NVRAM_DEFAULT 0x20
/* NVRAM is undefined. */
#define NVRAM_UNDEFINED 0x80000000

struct nvram_header {
	uint32_t magic;
	uint32_t len;
	uint32_t crc_ver_init;	/* 0:7 crc, 8:15 ver, 16:31 sdram_init */
	uint32_t config_refresh;	/* 0:15 sdram_config, 16:31 sdram_refresh */
	uint32_t config_ncdl;	/* ncdl values for memc */
} __attribute__((__packed__));

struct nvram_tuple {
	char *name;
	char *value;
	uint32_t option;
	struct nvram_tuple *next;
};

struct nvram_handle {
	int fd;
	char *mmap;
	int access;
	unsigned int length;
	unsigned int offset;
	struct nvram_tuple *nvram_hash[257];
	struct nvram_tuple *nvram_dead;
};

typedef struct nvram_handle nvram_handle_t;
typedef struct nvram_header nvram_header_t;
typedef struct nvram_tuple  nvram_tuple_t;


/* **************** Helper functions *****************/
/* String hash */
uint32_t hash(const char *s);
/* Free all tuples. */
void _nvram_free(nvram_handle_t *h);
/* (Re)allocate NVRAM tuples. */
nvram_tuple_t * _nvram_realloc( nvram_handle_t *h, nvram_tuple_t *t,
	const char *name, const char *value );
/* (Re)initialize the hash table. */
int _nvram_rehash(nvram_handle_t *h);
/* Computes a crc8 over the input data. */
uint8_t hndcrc8 (uint8_t * pdata, uint32_t nbytes, uint8_t crc);
/* Test Count size */
void count_nvram(nvram_handle_t *h);


/** \defgroup inner_func Inner Functions 
 *@{*/ 
/* Get nvram header. */
nvram_header_t * _nvram_header(nvram_handle_t *h);

/* Copy NVRAM contents to staging file. */
int nvram_to_staging(void);
/* Copy staging file to NVRAM device. */
int staging_to_nvram(void);

/* Determine NVRAM device node. */
char * nvram_find_mtd(void);
/* Check NVRAM staging file. */
char * nvram_find_staging(void);
/* Open NVRAM and obtain a handle. */
nvram_handle_t * _nvram_open(const char *file, int access);
/* Invoke a NVRAM handle for get, getall. */
nvram_handle_t * _nvram_open_rdonly(void);
/* Invoke a NVRAM handle for set, unset & commit. */
nvram_handle_t * _nvram_open_staging(void);
/* Close NVRAM handle and free memory. */
int _nvram_close(nvram_handle_t *h);

/* Get the value of an NVRAM variable. */
char * _nvram_get(nvram_handle_t *h, const char *name);
/* Get all NVRAM variables. */
nvram_tuple_t * _nvram_getall(nvram_handle_t *h);

/* Set the value of an NVRAM variable. */
int _nvram_set(nvram_handle_t *h, const char *name, const char *value);
/* Unset the value of an NVRAM variable. */
int _nvram_unset(nvram_handle_t *h, const char *name);

/* Regenerate NVRAM. */
int _nvram_commit(nvram_handle_t *h);
/**@}*/

/** \defgroup pub_func Public Functions 
 * @{ */
/* init NVRAM flash block */
void *nvram_init();
const nvram_handle_t * get_nvram_handle();
nvram_header_t * nvram_header();

/* wrapper of above common functions */
char * nvram_get(const char *name);
#define nvram_safe_get(name) (nvram_get(name) ? : "")
int nvram_get_option(const char *name);
int nvram_set(const char *name, const char *value);
int nvram_fset(const char *name, const char *value);
int nvram_unset(const char *name);
int nvram_reset(const char *name);
nvram_tuple_t * nvram_getall();
int nvram_commit(void);

int nvram_match(char *name, char *match);
int nvram_invmatch(char *name, char *invmatch);

/* restore from the whole factory default */
int nvram_default(void);
/* restore one specific rule from factory default */
int nvram_default_rule(const char *name);
/* commit  after restore the whole factory default */
int nvram_factory(void);
void nvram_boot(void);

int nvram_export(const char *filename);
int nvram_import(const char *filename);

//TODO
int nvram_upgrade(const char *source);
int nvram_downgrade(const char *target);

//helper function
int nvram_dump(void);
/**@}*/
#endif /* _nvram_h_ */

/**
\mainpage The NVRAM module for easy settings management
\section sec_str 1.Structure:

(1)cli.c 

Command line of nvram, a demo on how to call nvram_XXX functions from libnvram.so
I use #if/#else/#endif to disable the original version MAIN function and enable the 
current EZP integrated version MAIN.

(2)cli.h 

Command line header of nvram. The original version MAIN will call the inner function
interface of nvram, while my wrapped version version will call the public function of 
nvram. Using the later public interface, you can forget the management of nvram_handle_t.

(M)Makefile 

All nvram interfaces (public or inner) are compiled into a single dynamic library called
libnvram.so.0.1. If you want to use nvram_XXX functions in your code, do as what the Makefile
do.

(3)crc.c 

CRC related functions

(4)nvram.c

The inner functions of nvram are implemented.

(5)nvram_factory.h

the factory default key-value pairs are defined. A bunch of pairs will 
be stored in the libnvram.so.0.1. In this current version, I only write 'abc' and 'abc_rule' for 
demo.

(6)nvram_fw.c (7)nvram_fw.h 

nvram_upgrade/nvram_downgrade related function will be defined here. Currently the 2 file 
are copyed from EZP-NVRAM.

(8)nvram.h

Both the inner functions and public functions of nvram are defined here.

(9)nvram_public.c

the public functions of nvram are implemented.

(10)nvram_rule.c (11)nvram_rule.h

\b Rule/SubRule/Attribute related function are defined/implemented here. You can refer to
ezp-lib.c/ezp-lib.h in EZP-NVRAM. Which we will explain it later.

\section sec_rule 2.Rule/SubRule/Attribute
The Rule/SubRule/Attribute is defined in EZP-NVRAM. You can treat them as:

\b Rule	-	\b Rule is the whole value of key-value pair.

\b SubRule	-	If the \b Rule is connected by several '|' character, then each part of it is a SubRule;
			If there is no '|' character, the whole \b Rule is a SubRule of itself.
			Each SubRule has similar sturcture. 
			SubRule can be treated like Array. The first index of the SubRule is 0. 

\b Attribute -	The Rule/SubRule is made up of short concatenated characters. And the Attr seperate is '^'.

Given the lan_main_rule as an example(Refers to nvram_ezpacket.h in EZP-NVRAM):
			lan_main_rule="LAN1^1^1500^1^1^0|GuestLAN^0^1500^1^1^0"

\b Rule	-	LAN1^1^1500^1^1^0|GuestLAN^0^1500^1^1^0	

\b SubRule	-	LAN1^1^1500^1^1^0				//0

				GuestLAN^0^1500^1^1^0			//1

\b Attribute - LAN1							//name

				1								//enable

			1500							//mtu
 */
