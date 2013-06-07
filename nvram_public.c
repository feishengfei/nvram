#include "nvram.h"
nvram_handle_t *nvram_h = NULL;

/* **************** public functions **************** */
char * nvram_get(const char *name)
{
	char *ret = NULL;
	if(nvram_h<0) {
		nvram_h = _nvram_open_rdonly();
		if(NULL == nvram_h) {
			_nvram_close(nvram_h);
		}
	}
	ret = _nvram_get(nvram_h, name);
	return ret;
}

int nvram_set(const char *name, const char *value)
{
	return 0;
}

int nvram_fset(const char *name, const char *value)
{
	return 0;
}

int nvram_unset(const char *name)
{
	return 0;
}

int nvram_getall(char *buf, int count)
{
	return 0;
}

int nvram_commit(void)
{

	return 0;
}

void nvram_default(void)
{

}

void nvram_default_rule(char *rulename)
{

}

void nvram_factory(void)
{

}
