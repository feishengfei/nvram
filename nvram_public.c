#include <ctype.h>

#include "nvram.h"
#include "nvram_fw.h"
#include "nvram_factory.h"

/* Global */
nvram_handle_t *nvram_h = NULL;
char nvram_get_buf[NVRAM_TMP_LEN] = {0};
extern struct nvram_fw_tuple nvram_fw_table[];
extern size_t nvram_erase_size;



/****************** public functions **************** */
/**
 *\brief Getter of NVRAM handler
 *\return Pointer to NVRAM handler 
 **/
const nvram_handle_t* get_nvram_handle() 
{
	return nvram_h;
}

/**
 *\brief Get header from NVRAM handler, open the ReadOnly NVRAM handler if necessary
 *\return Pointer to NVRAM header
 **/
nvram_header_t * nvram_header()
{
	if(NULL == nvram_h) {
		nvram_h = _nvram_open_rdonly();
		if(NULL == nvram_h) {
			_nvram_close(nvram_h);
			return NULL;
		}
	}
	return _nvram_header(nvram_h);
}

/**
 *\brief Get the Value of an NVRAM variable. 
 *\return	Return the value of the name. 
			Retrun NULL if name does not exist or flash non-initialized
 *\param[in] name The specified name
 **/
char * nvram_get(const char *name)
{
	char *ret = NULL;
	if(NULL == nvram_h) {
		nvram_h = _nvram_open_rdonly();
		if(NULL == nvram_h) {
			_nvram_close(nvram_h);
			return NULL;
		}
	}


	ret = _nvram_get(nvram_h, name);
	if(ret){
		strcpy(nvram_get_buf, ret);
		{_nvram_close(nvram_h); nvram_h = NULL;}	//RESOLVE_II
		return nvram_get_buf;
	}
	else {
		{_nvram_close(nvram_h); nvram_h = NULL;}	//RESOLVE_II
		return NULL;
	}
}

/**
 *\brief Get the Option of an NVRAM variable. 
 *\return Return the option of the name accroding to Factory Default, NVRAM_UNDEFINED if the name doesn't exist.
 *\param[in] name The specified name
 **/
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

/**
 * \brief Set the value of an NVRAM variable. 
 * \return Return 0 on success, errno on fail
 * \param[in] name The specified name 
 * \param[in] value The specified value 
 * \note This won't set setting which is NVRAM_PROTECTED.
 **/
int nvram_set(const char *name, const char *value)
{
	int ret = 0;
	if (NULL == nvram_h) {
		nvram_h = _nvram_open_staging();
		if(NULL == nvram_h) {
			return _nvram_close(nvram_h);
		}
	}
	else if(NVRAM_RO == nvram_h->access) {
		_nvram_close(nvram_h);
		nvram_h = _nvram_open_staging();
		if(NULL == nvram_h) {
			return _nvram_close(nvram_h);
		}
	}


	uint32_t opt = nvram_get_option(name);

	/* If anything exists, return permission denied. */
	if (opt & NVRAM_PROTECTED) {
		char *exist = _nvram_get(nvram_h, name);
		if (exist && *exist) {
			return EACCES; 
		}
	}

	ret = _nvram_set(nvram_h, name, value);
	ret = _nvram_commit(nvram_h);
	{_nvram_close(nvram_h); nvram_h = NULL;}	//RESOLVE_II
	return ret;
}

/**
 * \brief Force to set the value of an NVRAM variable. 
 * \return Return 0 on success, errno on fail
 * \param[in] name The specified name 
 * \param[in] value The specified value 
 **/
int nvram_fset(const char *name, const char *value)
{
	int ret = -1;
	if (NULL == nvram_h) {
		nvram_h = _nvram_open_staging();
		if(NULL == nvram_h) {
			return _nvram_close(nvram_h);
		}
	}
	else if(NVRAM_RO == nvram_h->access) {
		_nvram_close(nvram_h);
		nvram_h = _nvram_open_staging();
		if(NULL == nvram_h) {
			return _nvram_close(nvram_h);
		}
	}

	ret = _nvram_set(nvram_h, name, value);
	ret = _nvram_commit(nvram_h);
	{_nvram_close(nvram_h); nvram_h = NULL;}	//RESOLVE_II
	return ret;
}

/**
 *\brief Unset the value of an NVRAM variable.
 *\return Return 0 on success //FIXME
 *\param[in] name The specifed name
 */
int nvram_unset(const char *name)
{
	return nvram_set(name, "");
}

/**
 *\brief Reset the value of an NVRAM variable.
 *\return Return 0 on success //FIXME
 *\param[in] name The specifed name
 */
int nvram_reset(const char *name)
{
	int ret = -1;
	if (NULL == nvram_h) {
		nvram_h = _nvram_open_staging();
		if(NULL == nvram_h) {
			return _nvram_close(nvram_h);
		}
	}
	else if(NVRAM_RO == nvram_h->access) {
		_nvram_close(nvram_h);
		nvram_h = _nvram_open_staging();
		if(NULL == nvram_h) {
			return _nvram_close(nvram_h);
		}
	}

	ret = _nvram_unset(nvram_h, name);
	{_nvram_close(nvram_h); nvram_h = NULL;}	//RESOLVE_II
	return ret;
}

/**
 *\brief Get all NVRAM variables. 
 *\return The iterator of all NVRAM settings.
 **/
nvram_tuple_t * nvram_getall()
{
	if(NULL == nvram_h) {
		nvram_h = _nvram_open_rdonly();
		if(NULL == nvram_h) {
			_nvram_close(nvram_h);
			return NULL;
		}
	}

	return _nvram_getall(nvram_h);
}

/**
 * \brief Regenerate NVRAM. 
 * \return Return 0 on success
 * \note This will clear staging file and all data will be saved into flash block
 **/
int nvram_commit(void)
{
	int stat = 0;

	if(NULL == nvram_h) {
		nvram_h = _nvram_open_staging();
		if(NULL == nvram_h) {
			return _nvram_close(nvram_h);
		}
	}
	else if(NVRAM_RO == nvram_h->access) {
		_nvram_close(nvram_h);
		nvram_h = _nvram_open_staging();
		if(NULL == nvram_h) {
			return _nvram_close(nvram_h);
		}
	}


	stat |= _nvram_commit(nvram_h);

	stat |= _nvram_close(nvram_h);
	nvram_h = NULL;	
	stat |= staging_to_nvram();

	return stat;
}

/**
 * \brief Restore settings to Factory Default if it belongs to.
 * \return Return 0 on success
 * \test Test Case for Performance:
 *
 * count=1;while(true); do echo count=$count;
 * dd if=/dev/zero of=/dev/mtdblock* bs=1k count=*;
 * nvram init; nvram default;nvram commit;
 * count=$(($count+1)); done;
 * \sa nvram_default nvram_factory
 **/
int nvram_default(void)
{
	int stat = 0;
	struct nvram_tuple *v;

	if(NULL == nvram_h) {
		nvram_h = _nvram_open_rdonly();
		if(NULL == nvram_h) {
			return _nvram_close(nvram_h);
		}
	}

	for (v = &nvram_factory_default[0]; v->name ; v++) {
		stat += nvram_set(v->name, v->value);
	}
	return stat;
}

/**
 * \brief Restore a specified setting to Factory Default.
 * \return Return 0 on success
 * \param[in] rule
 **/
int nvram_default_rule(const char *name)
{       
	int stat = 1;
	struct nvram_tuple *v;
	for (v = &nvram_factory_default[0]; v->name ; v++) {
		if(!strcmp(v->name, name)) {
			stat = nvram_set(v->name, v->value);
			return stat;	
		}
	}   
	return stat;
}

/**
 * \brief Restore settings to Factory Default if it belongs to, followed by commit process.
 * \return Return 0 on success
 * \test Test Case for Performance:
 *
 * count=1;while(true); do echo count=$count;
 * dd if=/dev/zero of=/dev/mtdblock* bs=1k count=*;
 * nvram init; nvram default;nvram commit;
 * count=$(($count+1)); done;
 * \sa nvram_default nvram_factory
 **/
int nvram_factory(void)
{
	int stat = 1;
	stat = nvram_default();
	stat = nvram_commit();
	return stat;
}


/**
 * \brief 	Update the \ref NVRAM_DEFAULT rule if it's empty.
 *			This may be effect only once it revert to factory default.
 *			1.Empty \ref NVRAM_DEFAULT will be updated by it's _default brother.
 *			2.Valid \ref NVRAM_TEMP will be reverted to it's factory default.
 */
void nvram_boot(void)
{
	struct nvram_tuple *v;
    char *value;
	nvram_handle_t *h_r = NULL;
	nvram_handle_t *h_w = NULL;

	h_r = _nvram_open_rdonly();
	if(NULL == h_r) {
		_nvram_close(h_r);
		return NULL;
	}

	h_w = _nvram_open_staging();
	if(NULL == h_w) {
		return _nvram_close(h_w);
	}

	for (v = &nvram_factory_default[0]; v->name ; v++) {
		value = _nvram_get(h_r, v->name);
        if (!value || !*value) {
            /* NULL or "\0" */
            if (v->option & NVRAM_EMPTY)
                continue; /* NULL or "\0" is allowed. */

            if (v->option & NVRAM_DEFAULT) {
                char default_name[64];
                /* Get the default value. */
                sprintf(default_name, "%s_default", v->name);
                v->value = _nvram_get(h_r, default_name);
            }

            _nvram_set(h_w, v->name, v->value);
			_nvram_commit(h_w);
        } else {
            /* Some value exist. */
            if (v->option & NVRAM_TEMP)
				_nvram_set(h_w, v->name, v->value);
				_nvram_commit(h_w);
        }
	}

	_nvram_close(h_w);
	_nvram_close(h_r);
}

/**
 * \brief Export settings with their current values to a specified file.
 * \return Return 0 on success
 * \param[in] filename The file to export settings.
 * \note Only NON NVRAM_PROTECTED and NON NVRAM_TEMP settings are allowed to be exported.
 **/
int nvram_export(const char *filename)
{   
	FILE *fp;
	struct nvram_tuple *v;
	char *value;

	if(NULL == nvram_h) {
		nvram_h = _nvram_open_rdonly();
		if(NULL == nvram_h) {
			return _nvram_close(nvram_h);
		}
	}

	if ( !(fp = fopen(filename, "wb") ))
		return EACCES;

	//HEADER of export
	fprintf(fp, 
		"[EZP_LOG v1.1] %s %s [EZP_%s%s] " EZP_PROD_VERSION "\n",
		nvram_safe_get("brand"), 
		nvram_safe_get("model"),
		nvram_safe_get("prod_cat"), 
		nvram_safe_get("prod_subcat")
	);

	for (v = &nvram_factory_default[0]; v->name ; v++) {
		if ((v->option & NVRAM_PROTECTED) ||
				(v->option & NVRAM_TEMP)) {
			continue;
		}
		value = nvram_safe_get(v->name);
		fprintf(fp, "%s=%s\n", v->name, value);
	}
	fclose(fp);
	return 0;
}

/**
 * \brief Import settings from specified file.
 * \return Return 0 on success
 * \param[in] filename The file to import settings.
 * \note	Only NON NVRAM_PROTECTED and NON NVRAM_TEMP settings are allowed to be imported. 
			The header line of the file will be checked.
 **/
int nvram_import(const char *filename)
{   
	FILE *fp;
	char *p, *q;
	char buf[NVRAM_TMP_LEN];
	char old_str[32], new_str[32];
	int old, new = 0;
	int i=0;

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
	strcpy(new_str, EZP_PROD_VERSION);
	/* Purify new_str. e.g. 1.6.5-RC1 => 1.6.5 */
	for (i = 0; old_str[i] == '.' || isdigit(old_str[i]) ; i++);
	old_str[i]='\0';
	for (i = 0; new_str[i] == '.' || isdigit(new_str[i]) ; i++);
	new_str[i]='\0';
	printf("Firmware:%s\r\nConfiguration:%s\r\n", new_str, old_str);

	/* Very likely we cannot find the matched version since our firmware might
	 * be older than the config file. */
	old = 0x0FFFFFFF;

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
/**
 *\brief Upgrade configurations to specified source version
 *\return Return 0 on success
 *\param[in] source The specified version to upgrade
 *\todo Need to be implemented
 */
int nvram_upgrade(const char *source)
{
	int change = 0;
	return change;
}

/**
 *\brief Downgrade configurations to specified target version
 *\return Return 0 on success
 *\param[in] source The specified version to downgrade
 *\todo Need to be implemented
 */
int nvram_downgrade(const char *target)
{
	int change = 0;
	return change;
}

/**
 * \brief Dump whole settings to standard output
 * \return Return 0 on success
 **/
int nvram_dump(void)
{
	int ret = -1;
	nvram_tuple_t * t = nvram_getall();
	while (NULL != t ){
		ret = 0;	
		printf("nvram set \"%s=%s\"\n", t->name, t->value);
		t = t->next;
	}
	return ret;
}


/**
 * \brief Init NVRAM flash block 
 * \return Return 0 on success
 * \test Test Case for Performance:
 *
 * count=1;while(true); do echo count=$count;
 * dd if=/dev/zero of=/dev/mtdblock* bs=1k count=*;
 * nvram init; nvram default;nvram commit;
 * count=$(($count+1)); done;
 * \sa nvram_default nvram_factory
 * \bug nvram init may get SIGSEGV if the last commit runs uncomplete.
 **/
void *nvram_init()
{
	nvram_to_staging();
	char *file = NVRAM_STAGING;
	int i;
	int fd;
	char *mtd = NULL;
	nvram_handle_t *h;
	int offset = -1;
	/* If erase size or file are undefined then try to define them */
	if( (nvram_erase_size == 0) || (file == NULL) )
	{
		/* Finding the mtd will set the appropriate erase size */
		if( (mtd = nvram_find_mtd()) == NULL || nvram_erase_size == 0 )
		{
			free(mtd);
			return NULL;
		}
	}

	if( (fd = open(file ? file : mtd, O_RDWR)) > -1 )
	{
		char *mmap_area = (char *) mmap(
			NULL, nvram_erase_size, PROT_READ | PROT_WRITE,
			 MAP_SHARED | MAP_LOCKED, fd, 0);

		if( mmap_area != MAP_FAILED )
		{
			for( i = 0; i <= ((nvram_erase_size - NVRAM_SPACE) / sizeof(uint32_t)); i++ )
			{
				if( ((uint32_t *)mmap_area)[i] == NVRAM_MAGIC )
				{
					offset = i * sizeof(uint32_t);
					break;
				}
			}
			if( offset >= 0 )
			{
				free(mtd);
				return NULL; 
			}

			if( (h = malloc(sizeof(nvram_handle_t))) != NULL )
			{
				memset(h, 0, sizeof(nvram_handle_t));

				h->fd     = fd;
				h->mmap   = mmap_area;
				h->length = nvram_erase_size;
				h->offset = NVRAM_OFFSET;
				h->access = NVRAM_RW;


				_nvram_rehash(h);
				_nvram_commit(h);
				_nvram_close(h);
				staging_to_nvram();
				free(mtd);
				return h;
			}
		}
	}

	free(mtd);
	return NULL;
}

/**
 * \brief Match an NVRAM variable.
 * \return  TRUE if variable is defined and its value is string equal
 *      to match or FALSE otherwise
 * \param   name    name of variable to match
 * \param   match   value to compare against value of variable
 */
int nvram_match(char *name, char *match) 
{
	const char *value = nvram_get(name);
	return (value && !strcmp(value, match));
}

/**
 * \brief Inversely match an NVRAM variable.
 * \return  TRUE if variable is defined and its value is not string
 *      equal to invmatch or FALSE otherwise
 * \param   name    name of variable to match
 * \param   match   value to compare against value of variable
 */
int nvram_invmatch(char *name, char *invmatch) {
	const char *value = nvram_get(name);
	return (value && strcmp(value, invmatch));
}


