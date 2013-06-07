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

#include "sdinitvals.h"

/* Staging file for NVRAM */
#define NVRAM_MTD_NAME		"nvram"
#define NVRAM_STAGING		"/tmp/.nvram"
#define NVRAM_RO			1
#define NVRAM_RW			0

/* Helper macros */
#define NVRAM_ARRAYSIZE(a)	sizeof(a)/sizeof(a[0])
#define	NVRAM_ROUNDUP(x, y)	((((x)+((y)-1))/(y))*(y))

/* NVRAM constants */
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

/* **************** inner functions **************** */
/* Get nvram header. */
nvram_header_t * _nvram_header(nvram_handle_t *h);

/* Open NVRAM and obtain a handle. */
nvram_handle_t * _nvram_open(const char *file, int rdonly);
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

/* **************** public functions **************** */
char * nvram_get(const char *name);
int nvram_set(const char *name, const char *value);
int nvram_fset(const char *name, const char *value);
int nvram_unset(const char *name);
int nvram_getall(char *buf, int count);
int nvram_commit(void);

void nvram_default(void);
void nvram_default_rule(char *rulename);
void nvram_factory(void);
/********************************************************/

/* Computes a crc8 over the input data. */
uint8_t hndcrc8 (uint8_t * pdata, uint32_t nbytes, uint8_t crc);

/* Returns the crc value of the nvram. */
uint8_t nvram_calc_crc(nvram_header_t * nvh);

/* Determine NVRAM device node. */
char * nvram_find_mtd(void);

/* Check NVRAM staging file. */
char * nvram_find_staging(void);

/* Copy NVRAM contents to staging file. */
int nvram_to_staging(void);

/* Copy staging file to NVRAM device. */
int staging_to_nvram(void);


#endif /* _nvram_h_ */
