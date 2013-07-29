#ifndef _nvram_h_
#define _nvram_h_

typedef struct environment_s {
	unsigned long crc;		//CRC32 over data bytes
	char *data;
} env_t;

typedef struct cache_environment_s {
	char *name;
	char *value;
} cache_t;

#define ENV_BLK_SIZE 0x1000
#define MAX_CACHE_ENTRY 500
typedef struct block_s {
	char *name;
	env_t env;			//env block
	cache_t	cache[MAX_CACHE_ENTRY];	//env cache entry by entry
	unsigned long flash_offset;
	unsigned long flash_max_len;	//ENV_BLK_SIZE
	char valid;
	char dirty;
} block_t;

#define MAX_NAME_LEN 128
#define MAX_VALUE_LEN (ENV_BLK_SIZE * 5)
typedef struct nvram_ioctl_s {
	int index;
	int ret;
	char *name;
	char *value;
} nvram_ioctl_t;


#define FLASH_BLOCK_NUM 1
#define NV_DEV "/dev/nvram"
#define NV_DEV "/dev/nvram"

#define NVRAM_IOCTL_GET		0x01
#define NVRAM_IOCTL_GETALL	0x02
#define NVRAM_IOCTL_SET		0x03
#define NVRAM_IOCTL_COMMIT	0x04
#define NVRAM_IOCTL_CLEAR	0x05

int		nvram_init(int * nvram_fd);
void 	nvram_close();

int 	nvram_set(const char *name, const char *value);
char *	nvram_get(const char *name);
int 	nvram_getall(char *buf,int count);
int 	nvram_commit();

int 	nvram_fset(const char *name, const char *value);
int 	nvram_unset(const char *name);
char *	nvram_safe_get(const char *name);


int 	cache_idx(const char *name);
int 	nvram_bufset(const char *name, const char *value);
char  	*nvram_bufget(const char *name);
void 	nvram_buflist(void);


int 	nvram_clear();
//int 	nvram_erase();//TODO

void 	toggleNvramDebug(void);


int nvram_export(const char *filename);
int nvram_import(const char *filename);

int nvram_upgrade(const char *source);
int nvram_downgrade(const char *target);

int nvram_factory(void);
int nvram_default(void);
int nvram_default_rule(const char *name);
void nvram_boot(void);
int nvram_dump(void);
#endif /* _nvram_h_ */
