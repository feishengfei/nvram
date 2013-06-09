#include "nvram.h"
#include "nvram_fw.h"
#include "nvram_factory.h"
/* Global */
nvram_handle_t *nvram_h = NULL;

/* **************** public functions **************** */
nvram_handle_t* get_nvram_handle() 
{
	return nvram_h;
}

nvram_header_t * nvram_header()
{
	if(NULL == nvram_h) {
		nvram_h = _nvram_open_rdonly();
		if(NULL == nvram_h) {
			_nvram_close(nvram_h);
		}
	}
	return _nvram_header(nvram_h);
}

char * nvram_get(const char *name)
{
	char *ret = NULL;
	if(NULL == nvram_h) {
		nvram_h = _nvram_open_rdonly();
		if(NULL == nvram_h) {
			_nvram_close(nvram_h);
		}
	}

	ret = _nvram_get(nvram_h, name);
	return ret;
}

int nvram_get_option(const char *name)
{   
	struct nvram_tuple *v;
	for (v = &nvram_factory_default[0]; v->name ; v++) {
		if (!strcmp(v->name, name))
			return v->option;
	}

	/* No option is found. */
	return NVRAM_UNDEFINED;
}

int nvram_set(const char *name, const char *value)
{
	int ret = 0;
	if (NULL == nvram_h) {
		nvram_h = _nvram_open_staging();
		if(NULL == nvram_h) {
			_nvram_close(nvram_h);
		}
	}
	else if(NVRAM_RO == nvram_h->access) {
		_nvram_close(nvram_h);
		nvram_h = _nvram_open_staging();
		if(NULL == nvram_h) {
			_nvram_close(nvram_h);
		}
	}

	uint32_t opt = nvram_get_option(name);

	/* If anything exists, return permission denied. */
	if (opt & NVRAM_PROTECTED) {
		char *exist = nvram_get(name);
		if (exist && *exist) {
			return EACCES; 
		}
	}

	ret = _nvram_set(nvram_h, name, value);
	ret = _nvram_commit(nvram_h);
	return ret;
}

int nvram_fset(const char *name, const char *value)
{
	int ret;
	if (NULL == nvram_h) {
		nvram_h = _nvram_open_staging();
		if(NULL == nvram_h) {
			_nvram_close(nvram_h);
		}
	}
	else if(NVRAM_RO == nvram_h->access) {
		_nvram_close(nvram_h);
		nvram_h = _nvram_open_staging();
		if(NULL == nvram_h) {
			_nvram_close(nvram_h);
		}
	}

	ret = _nvram_set(nvram_h, name, value);
	ret = _nvram_commit(nvram_h);
	return ret;
}

int nvram_unset(const char *name)
{
	return nvram_set(name, "");
}

nvram_tuple_t * nvram_getall()
{
	char *ret = NULL;
	if(NULL == nvram_h) {
		nvram_h = _nvram_open_rdonly();
		if(NULL == nvram_h) {
			_nvram_close(nvram_h);
		}
	}
	
	return _nvram_getall(nvram_h);
}

int nvram_commit(void)
{
	int stat = 1;

	if(NULL == nvram_h) {
		nvram_h = _nvram_open_staging();
	}
	else if(NVRAM_RO == nvram_h->access) {
		_nvram_close(nvram_h);
		nvram_h = _nvram_open_staging();
		if(NULL == nvram_h) {
			_nvram_close(nvram_h);
		}
	}

	stat = _nvram_commit(nvram_h);

	_nvram_close(nvram_h);

	stat = staging_to_nvram();

	return stat;
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

int nvram_export(char *filename)
{   
	FILE *fp;
	struct nvram_tuple *v;
	char *value;

	if ( !(fp = fopen(filename, "wb") ))
		return EACCES;

	//HEADER of export
	fprintf(fp, 
		"[EZP_LOG v1.1] %s %s [EZP_%s%s] " xstr(EZP_PROD_VERSION) "\n",
		nvram_safe_get("brand"), nvram_safe_get("model"),
		nvram_safe_get("prod_cat"), nvram_safe_get("prod_subcat"));

	for (v = &nvram_factory_default[0]; v->name ; v++) {
		if ((v->option & NVRAM_PROTECTED) ||
				(v->option & NVRAM_TEMP)) {
			continue;
		}
		value = nvram_safe_get(v->name);
		printf("%s=%s\n", v->name, value);
		fprintf(fp, "%s=%s\n", v->name, value);
	}
	fclose(fp);
	return 0;
}

int nvram_import(char *filename)
{   
	FILE *fp;
	char *p, *q;
	char buf[NVRAM_TMP_LEN];
	char old_str[32], new_str[32];
	int old, new = 0;
	int i;

	struct nvram_tuple *v;
	struct nvram_fw_tuple *w;

	if ( !(fp = fopen(filename, "r") ))
		return 1;

	/* First line should begin with "EZP_LOG". */
	fgets(buf, sizeof(buf), fp);
	if ((p = strstr(buf, "EZP_LOG")) == NULL) {
		printf("log file format error\n");
		return 1;
	}
	if ((p = strstr(p + strlen("EZP_LOG"), "EZP_")) == NULL) {
		printf("log file format error: product\n");
		return 1;
	}
	p += strlen("EZP_");
	/* prod_cat */
	q = nvram_safe_get("prod_cat");
	if (p[0] != q[0]) {
		printf("log file format error: prod_cat\n");
		return 1;
	}
	/* prod_subcat */
	q = nvram_safe_get("prod_subcat");
	if (p[1] != q[0]) {
		printf("log file format error: prod_subcat\n");
		return 1;
	}
	p = strchr(p, ']');
	p += 1;

	if (*p == '\n' || *p == '\0') {
		strcpy(old_str, "0.0.0");
	} else {
		strncpy(old_str, p + 1, strlen(p + 1));
	}

	/* XXX:We don't accept any thing higher than our current version. */
	strcpy(new_str, xstr(EZP_PROD_VERSION));
	/* Purify new_str. e.g. 1.6.5-RC1 => 1.6.5 */
/*
	for (i = 0; old_str[i] == '.' || isdigit(old_str[i]) ; i++);
	old_str[i]='\0';
	for (i = 0; new_str[i] == '.' || isdigit(new_str[i]) ; i++);
	new_str[i]='\0';
*/

	/* Very likely we cannot find the matched version since our firmware might
	 * be older than the config file. */
	old = 0x0FFFFFFF;
/*
	for (w = &nvram_fw_table[0]; w->fw_str ; w++) {
		if (!strcmp(w->fw_str, old_str)) {
			old = w->fw_version;
		}
		if (!strcmp(w->fw_str, new_str)) {
			new = w->fw_version;
		}
	}

	if (old > new) {
		printf("log file format error: newer version configuration format\n");
		return 1;
	}
*/
	while (fgets(buf, sizeof(buf), fp)) {
		if ((p = strchr(buf, '=')) == NULL)
			continue;

		/* Please the end of the string to replace "=". */
		*p = 0;

		for (v = &nvram_factory_default[0]; v->name ; v++) {
			if (!strcmp(v->name, buf)) {
				break;
			}
		}

		if (!v->name || (v->option & NVRAM_PROTECTED) ||
				(v->option & NVRAM_TEMP)) {
			/* No match or NVRAM_PROTECTED or NVRAM_TEMP. */
			printf("invalid: %s=%s\n", buf, p + 1);
			continue;
		}

		if (*(p + 1) == '\n') {
			/* "key=\n" */
			nvram_set(buf, "");
		} else {
			/* "key=value\n" */
			p++;
			/* Replace \n with \0 */
			if (*(p + strlen(p) - 1) == '\n') {
				*(p + strlen(p) - 1) = '\0';
			}
			nvram_set(buf, p);
		}
	}
	fclose(fp);

	nvram_upgrade(old_str);

	return 0;
}

//TODO
int nvram_upgrade(char *source)
{
	int change = 0;
	return change;
}

//TODO
int nvram_downgrade(char *target)
{
	int change = 0;
	return change;
}

