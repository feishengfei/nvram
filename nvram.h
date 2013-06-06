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


struct nvram_header {
	uint32_t magic;
	uint32_t len;
	uint32_t crc_ver_init;	/* 0:7 crc, 8:15 ver, 16:31 sdram_init */
	uint32_t config_refresh;	/* 0:15 sdram_config, 16:31 sdram_refresh */
	uint32_t config_ncdl;	/* ncdl values for memc */
} __attribute__((__packed__));

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


/* Get nvram header. */
nvram_header_t * nvram_header(nvram_handle_t *h);

/* Set the value of an NVRAM variable. The name and value strings are
 * copied into private storage. Pointers to previously set values
 * may become invalid. The new value may be immediately
 * retrieved but will not be permanently stored until a commit.
 * @param	h	nvram handle
 * @param	name	name of variable to set
 * @param	value	value of variable
 * @return	0 on success and errno on failure */
int nvram_set(nvram_handle_t *h, const char *name, const char *value);

/* Get the value of an NVRAM variable. The pointer returned may be
 * invalid after a set.
 * @param	h	nvram handle
 * @param	name	name of variable to get
 * @return	value of variable or NULL if undefined */
char * nvram_get(nvram_handle_t *h, const char *name);

/* Get the value of an NVRAM variable.
 * @param	h	nvram handle
 * @param	name	name of variable to get
 * @return	value of variable or "" if undefined */
#define nvram_safe_get(h, name) (nvram_get(h, name) ? : "")

/* Unset an NVRAM variable. Pointers to previously set values
 * remain valid until a set.
 * @param	h	nvram handle
 * @param	name	name of variable to unset
 * @return	0 on success and errno on failure
 * NOTE: use nvram_commit to commit this change to flash. */
int nvram_unset(nvram_handle_t *h, const char *name);

/*
 * Get all NVRAM variables one by one
 * @param	h	nvram handle
 * @return	name-value tuple
 */
nvram_tuple_t * nvram_getall(nvram_handle_t *h);

/*
 * Commit NVRAM variables to permanent storage. All pointers to values
 * may be invalid after a commit.
 * NVRAM values are undefined after a commit.
 * @param	h	nvram handle
 * @return	0 on success and errno on failure
 */
int nvram_commit(nvram_handle_t *h);


/********************************************************/

extern char * nvram_get(const char *name);
extern int nvram_set(const char *name, const char *value);
extern int nvram_fset(const char *name, const char *value);
extern int nvram_unset(const char *name);
extern int nvram_getall(char *buf, int count);
extern int nvram_commit(void);

extern void nvram_default(void);
extern void nvram_default_rule(char *rulename);
extern void nvram_factory(void);

/********************************************************/









/ Open NVRAM and obtain a handle. */
nvram_handle_t * nvram_open(const char *file, int rdonly);

/* Close NVRAM and free memory. */
int nvram_close(nvram_handle_t *h);

/* Computes a crc8 over the input data. */
uint8_t hndcrc8 (uint8_t * pdata, uint32_t nbytes, uint8_t crc);

/* Returns the crc value of the nvram. */
uint8_t nvram_calc_crc(nvram_header_t * nvh);

/* Determine NVRAM device node. */
char * nvram_find_mtd(void);

/* Copy NVRAM contents to staging file. */
int nvram_to_staging(void);

/* Copy staging file to NVRAM device. */
int staging_to_nvram(void);

/* Check NVRAM staging file. */
char * nvram_find_staging(void);


/* Staging file for NVRAM */
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

#define NVRAM_CRC_START_POSITION	9 /* magic, len, crc8 to be skipped */


#endif /* _nvram_h_ */
