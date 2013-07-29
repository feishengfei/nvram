#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <assert.h>

#include "nvram.h"

char libnvram_debug = 0;
#define LIBNV_PRINT(x, ...) do { if (libnvram_debug) printf("%s %d: " x, __FILE__, __LINE__, ## __VA_ARGS__); } while(0)
#define LIBNV_ERROR(x, ...) do { printf("%s %d: ERROR! " x, __FILE__, __LINE__, ## __VA_ARGS__); } while(0)

static block_t fb[FLASH_BLOCK_NUM] =
{
	{
		.name = "2860",
		.flash_offset =  0x0,
		//.flash_max_len = 0x10000,
		.flash_max_len = 0x8000,
		.valid = 0
	}
};

//x is the value returned if the check failed
#define LIBNV_CHECK_INDEX(x) do { \
	if (index < 0 || index >= FLASH_BLOCK_NUM) { \
		LIBNV_PRINT("index(%d) is out of range\n", index); \
		return x; \
	} \
} while (0)

#define LIBNV_CHECK_VALID(nvramfd_addr) do { \
	if (!fb[index].valid ) { \
		LIBNV_PRINT("fb[%d] invalid, init again\n", index); \
		nvram_init(nvramfd_addr); \
	} \
} while (0)

#define FREE(x) do { if (x != NULL) {free(x); x=NULL;} } while(0)

/*
 * 1. read env from flash
 * 2. parse entries
 * 3. save the entries to cache
 */
int nvram_init(int * nvram_fd) 
{
	unsigned long from;
	int i, len;
	char *p, *q;
	int fd;
	nvram_ioctl_t nvr;
	int ret;

	int index = 0;
	LIBNV_PRINT("--> nvram_init %d\n", index);
	LIBNV_CHECK_INDEX();
	if (fb[index].valid)
		return 0;
	
	/*
	 * read data from flash
	 * skip crc checking which is done by Kernel NVRAM module 
	 */
	from = fb[index].flash_offset + sizeof(fb[index].env.crc);
	len = fb[index].flash_max_len - sizeof(fb[index].env.crc);
	fb[index].env.data = (char *)malloc(len);
	nvr.index = index;
	nvr.value = fb[index].env.data;

	//printf("======from %lx len %x index %d=====\n",from, len, nvr.index);
       
	*nvram_fd = fd = open(NV_DEV, O_RDONLY);//here maybe not useful because write is use ioctl in the below set also open the NV_DEV with 	O_RDONLY ,commented by daniel
	//printf("=========*nvram_fd %d= fd=%d===========\n", *nvram_fd,fd);
	if (fd < 0) {
		perror(NV_DEV);
		ret=fd;
		goto out;
	}
	if (ioctl(fd, NVRAM_IOCTL_GETALL, &nvr) < 0) {
		perror("ioctl");
		close(fd);
		ret=fd;
		goto out;
	}
	close(fd);

	//parse env to cache
	p = fb[index].env.data;
	for (i = 0; i < MAX_CACHE_ENTRY; i++) {
		if (NULL == (q = strchr(p, '='))) {
			LIBNV_PRINT("parsed failed - cannot find '='\n");
			break;
		}
		*q = '\0'; //strip '='
		fb[index].cache[i].name = strdup(p);
		//printf("  %d '%s'->", i, p);

		p = q + 1; //value
		if (NULL == (q = strchr(p, '\0'))) {
			LIBNV_PRINT("parsed failed - cannot find '\\0'\n");
			break;
		}
		fb[index].cache[i].value = strdup(p);
		//printf("'%s'\n", p);

		p = q + 1; //next entry
		if (p - fb[index].env.data + 1 >= len) //end of block
			break;
		if (*p == '\0') //end of env
			break;
	}
	if (i == MAX_CACHE_ENTRY)
		LIBNV_PRINT("run out of env cache, please increase MAX_CACHE_ENTRY\n");

	fb[index].valid = 1;
	fb[index].dirty = 0;
	return 0;

out:
	FREE(fb[index].env.data); //free it to save momery
	return ret;
}

void nvram_close()
{
	int i;
	int index = 0;
	LIBNV_PRINT("--> nvram_close %d\n", index);
	LIBNV_CHECK_INDEX();

	if (!fb[index].valid)
		return;

	//free env
	FREE(fb[index].env.data);

	//free cache
	for (i = 0; i < MAX_CACHE_ENTRY; i++) {
		FREE(fb[index].cache[i].name);
		FREE(fb[index].cache[i].value);
	}

	fb[index].valid = 0;
}

int nvram_set(const char *name, const char *value)
{
	if (-1 == nvram_bufset(name, value))
		return -1;

	return 0;
}

int nvram_fset(const char *name, const char *value)
{
	return nvram_set(name, value);
}

int nvram_unset(const char *name)
{
	return nvram_set(name, "");
}


char * nvram_get(const char *name)
{
	return nvram_bufget(name);
}


char * nvram_safe_get(const char *name)                                       
{      
	char *ret = nvram_get(name);                                           
	return ret ? ret : "";                                                 
} 

int nvram_getall(char *buf,int count)
{

	unsigned long from;
	int len;
	int fd;
	nvram_ioctl_t nvr;
	int ret;
	int index = 0;
	LIBNV_PRINT("--> nvram_init %d\n", index);
	LIBNV_CHECK_INDEX(-1);
	
	/*
	 * read data from flash
	 * skip crc checking which is done by Kernel NVRAM module 
	 */
	from = fb[index].flash_offset ;
	len = fb[index].flash_max_len;
	
	if (count <=len)
		len =count;
	
	
	fb[index].env.data = (char *)malloc(len);
	nvr.index = index;
	nvr.value = fb[index].env.data;

	fd = open(NV_DEV, O_RDONLY);

	if (fd < 0) {
		perror(NV_DEV);
		ret=fd;
		goto out;
	}
	if (ioctl(fd, NVRAM_IOCTL_GETALL, &nvr) < 0) {
		perror("ioctl");
		close(fd);
		ret=fd;
		goto out;
	}
	close(fd);

	
	memcpy(buf,nvr.value,len);

out:
	FREE(fb[index].env.data); //free it to save momery
	return len;

}
/*
 *  * return idx (0 ~ iMAX_CACHE_ENTRY)
 *   * return -1 if no such value or empty cache
 *    */
int cache_idx(const char *name)
{
	int i;
	int index = 0;
	for (i = 0; i < MAX_CACHE_ENTRY; i++) {
		if (!fb[index].cache[i].name)
			return -1;
		if (!strcmp(name, fb[index].cache[i].name))
			return i;
	}
	return -1;
}

int nvram_bufset(const char *name, const char *value)
{
	int idx;
	int fd;
	nvram_ioctl_t nvr;

	//LIBNV_PRINT("--> nvram_bufset\n");
	int tmpfd=-1;
	int index = 0;
	LIBNV_CHECK_INDEX(-1);
	LIBNV_CHECK_VALID(&tmpfd);

	nvr.index = index;
	nvr.name = name;
	nvr.value = value;
	fd = open(NV_DEV, O_RDONLY);
	if (fd < 0) {
		perror(NV_DEV);
		return -1;
	}
	if (ioctl(fd, NVRAM_IOCTL_SET, &nvr) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);

	idx = cache_idx(name);

	if (-1 == idx) {
		//find the first empty room
		for (idx = 0; idx < MAX_CACHE_ENTRY; idx++) {
			if (!fb[index].cache[idx].name)
				break;
		}
		//no any empty room
		if (idx == MAX_CACHE_ENTRY) {
			LIBNV_ERROR("run out of env cache, please increase MAX_CACHE_ENTRY\n");
			return -1;
		}
		fb[index].cache[idx].name = strdup(name);
		fb[index].cache[idx].value = strdup(value);
	}
	else {
		//abandon the previous value
		FREE(fb[index].cache[idx].value);
		fb[index].cache[idx].value = strdup(value);
	}
	LIBNV_PRINT("bufset %d '%s'->'%s'\n", index, name, value);
	fb[index].dirty = 1;

	return 0;
}

char  *nvram_bufget(const char *name)
{
	int idx;
	static char  *ret;
	int fd;
	nvram_ioctl_t nvr;
	//add by daniel
	int tmpfd=-1;
	//LIBNV_PRINT("--> nvram_bufget %d\n", index);
	int index = 0;
	LIBNV_CHECK_INDEX("");
	LIBNV_CHECK_VALID(&tmpfd);

	nvr.index = index;
	nvr.name = name;
	nvr.value = malloc(MAX_VALUE_LEN);
	fd = open(NV_DEV, O_RDONLY);
	if (fd < 0) {
		perror(NV_DEV);
		FREE(nvr.value);
		return "";
	}
	if (ioctl(fd, NVRAM_IOCTL_GET, &nvr) < 0) {
		perror("ioctl");
		close(fd);
		FREE(nvr.value);
		return "";
	}
	close(fd);

	idx = cache_idx(name);

	if (-1 != idx) {
		if (fb[index].cache[idx].value) {
			//duplicate the value in case caller modify it
			//ret = strdup(fb[index].cache[idx].value);
			FREE(fb[index].cache[idx].value);
			fb[index].cache[idx].value = strdup(nvr.value);
			FREE(nvr.value);
			ret = fb[index].cache[idx].value;
			LIBNV_PRINT("bufget %d '%s'->'%s'\n", index, name, ret);
			return ret;
		}
	}

	//no default value set?
	//btw, we don't return NULL anymore!
	LIBNV_PRINT("bufget %d '%s'->''(empty) Warning!\n", index, name);

	FREE(nvr.value);
     
	return "";
}

void nvram_buflist(void)
{
	int i;

	int  tmpfd=-1;
	int index = 0;	
	//LIBNV_PRINT("--> nvram_buflist %d\n", index);
	LIBNV_CHECK_INDEX();
	LIBNV_CHECK_VALID(&tmpfd);

	for (i = 0; i < MAX_CACHE_ENTRY; i++) {
		if (!fb[index].cache[i].name)
			break;
		printf("  '%s'='%s'\n", fb[index].cache[i].name, fb[index].cache[i].value);
	}
}

/*
 * write flash from cache
 */
int nvram_commit()
{
	int fd;
	nvram_ioctl_t nvr;
	//add by daniel 
	int tmpfd=-1;
	
	int index = 0;
	//LIBNV_PRINT("--> nvram_commit %d\n", index);
	LIBNV_CHECK_INDEX(-1);
	//LIBNV_PRINT("==commit====fb %p  value addr %pvalid=%d=====\n", fb, &(fb[index].valid),fb[index].valid);
	LIBNV_CHECK_VALID(&tmpfd);


	nvr.index = index;
	fd = open(NV_DEV, O_RDWR);
	if (fd < 0) {
		perror(NV_DEV);
		return -1;
	}
	if (ioctl(fd, NVRAM_IOCTL_COMMIT, &nvr) < 0) { 
		perror("ioctl"); 
		close(fd); 
		return -1;
	}
	close(fd);

	fb[index].dirty = 0;

	return 0;
}

/*
 * clear flash by writing all 1's value
 */
int nvram_clear()
{
	int fd;
	nvram_ioctl_t nvr;
	
	int index = 0;
	LIBNV_PRINT("--> nvram_clear %d\n", index);
	LIBNV_CHECK_INDEX(-1);
	nvram_close();

	nvr.index = index;
	fd = open(NV_DEV, O_RDONLY);
	if (fd < 0) {
		perror(NV_DEV);
		return -1;
	}
	if (ioctl(fd, NVRAM_IOCTL_CLEAR, &nvr) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);

	fb[index].dirty = 0;
	return 0;
}
