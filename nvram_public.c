#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "nvram.h"
#include "nvram_fw.h"
#include "nvram_rule.h"

/* Global */
extern struct nvram_fw_tuple nvram_fw_table[];
extern nvram_tuple_t nvram_factory_default[];
extern int _nvram_set(const char *name, const char *value);

/**
 * **************Advanced Interface*************************
 */

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

int nvram_set_boot(const char *name, const char *value)
{
    unsigned int opt = nvram_get_option(name);
    int ret = 0;
  
    //printf("==============in nvram_set=== opt  %x,name %s,value %s===========\n",opt,name,value);
    if (opt & NVRAM_UNDEFINED) {
        return EINVAL;
    }
    if (opt & NVRAM_PROTECTED) {
        char *exist = nvram_get(name);
        if (exist && *exist) {
            return EACCES; /* If anything exists, return permission denied. */
        }
    }
	
    ret = _nvram_set(name, value);
    return ret;
}

void nvram_boot(void)
{
	struct nvram_tuple *v;
    char *value;

    nvram_init(NULL);

	for (v = &nvram_factory_default[0]; v->name ; v++) {
		value = nvram_get(v->name);
        if (!value || !*value) {
            /* NULL or "\0" */
            if (v->option & NVRAM_EMPTY)
                continue; /* NULL or "\0" is allowed. */

            if (v->option & NVRAM_DEFAULT) {
                char default_name[64];
                /* Get the default value. */
                sprintf(default_name, "%s_default", v->name);
                v->value = nvram_get( default_name);
            }

            nvram_set_boot(v->name, v->value);
        } else {
            /* Some value exist. */
            if (v->option & NVRAM_TEMP)
               nvram_set_boot(v->name, v->value);
        }
	}
    nvram_close();
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
	printf("[import]Firmware:%s\r\nConfiguration:%s\r\n", new_str, old_str);

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
			//printf("invalid: %s=%s\n", buf, p + 1);
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

int special_processing()
{
    char tmp[32];
    char *val;

    ezplib_get_attr_val("ssh_rule", 0, "enable", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
    if (tmp[0] != '0') {
        ezplib_replace_attr("ssh_rule", 0, "enable", "0");
        return 1;
    }

    val = nvram_get("turbonat_enable");
    if (!val || *val == '0') {
        nvram_fset("turbonat_enable", "1");
    }

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
    struct nvram_fw_tuple *v;
    int old, new = 0;
    char old_str[32];
    char new_str[32];
    int i, change = 0;

    /* If source is empty, get it from fw_version. */
    if (source && *source) {
        strcpy(old_str, source);
    } else {
        strcpy(old_str, nvram_safe_get("fw_version"));
    }
    strcpy(new_str, EZP_PROD_VERSION);

	//add for test
    printf("====== old_str :%s ===  new_str :%s=====\n",old_str,new_str);

    if (strcmp(old_str, new_str)) {
        /** 
         * If the versions have any difference, 
         * eg., 2.0.2-RC1 to 2.0.2-RC3, we
         * should enforce the special processing, 
         * fg., closing the backdoor.
         */
        change = special_processing();
    }

    /* Purify new_str. e.g. 1.6.5-RC1 => 1.6.5 */
    for (i = 0; old_str[i] == '.' || isdigit(old_str[i]) ; i++);
    old_str[i]='\0';
    for (i = 0; new_str[i] == '.' || isdigit(new_str[i]) ; i++);
    new_str[i]='\0';

    /* Use "0.0.0" instead of "". */
    if (!*old_str) {
        strcpy(old_str, nvram_fw_table[0].fw_str);
    }

    printf("====== old_str :%s ===  new_str :%s=====\n",old_str,new_str);

    /* We might not be able to find out the version. Skip upgrade if cannot. */
    old = 0xEFFFFFFF;
    for (v = &nvram_fw_table[0]; v->fw_str ; v++) {
        if (!strcmp(v->fw_str, old_str)) {
            old = v->fw_version;
        }
        if (!strcmp(v->fw_str, new_str)) {
            new = v->fw_version;
        }
    }
	printf("=======old:%d==new:%d=======\n",old,new);

	
    if (old < new) {
        printf("Upgrade [%s->%s], index[%d->%d]\n", old_str, new_str, old, new);
        /* Upgrade. Use the upgrade functions from (old + 1) to new. */
        for (i = old + 1; i <= new; i++) {
            if (nvram_fw_table[i].fw_upgrade_func) {
                (*nvram_fw_table[i].fw_upgrade_func)();
            }
        }
        /* nvram is the only place to set fw_version. */
        nvram_fset("fw_version", EZP_PROD_VERSION);
        change = 1;
    }
    
    if(strcmp(nvram_safe_get("prod_subsubcat"), "")) {
        /* unset prod_subsubcat to make sure prod_subsubcat the same with 
         * special firmware */
        nvram_unset("prod_subsubcat"); 
        change = 1;
    }
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
    struct nvram_fw_tuple *v;
    int old, new;
    char old_str[32];
    char new_str[32];
    int i, change = 0;

    strcpy(old_str, nvram_safe_get("fw_version"));
    strcpy(new_str, target);
	printf("old str: %s, new str: %s\n", old_str, new_str);
    if (strcmp(old_str, new_str)) {
        /* If the versions have any difference, eg., 2.0.2-RC1 to 2.0.2-RC3, we
         * should enforce the special processing, fg., closing the backdoor.
         */
        change = special_processing();
    }

    /* Purify new_str. e.g. 1.6.5-RC1 => 1.6.5 */
    for (i = 0; old_str[i] == '.' || isdigit(old_str[i]) ; i++);
    old_str[i]='\0';
    for (i = 0; new_str[i] == '.' || isdigit(new_str[i]) ; i++);
    new_str[i]='\0';

    /* Use "0.0.0" instead of "". */
    if (!*old_str) {
        strcpy(old_str, nvram_fw_table[0].fw_str);
    }
    if (!*new_str) {
        strcpy(new_str, nvram_fw_table[0].fw_str);
    }

    /* We might not find out the version. Skip downgrade if cannot. */
    old = 0;
    for (v = &nvram_fw_table[0]; v->fw_str ; v++) {
        if (!strcmp(v->fw_str, old_str)) {
            old = v->fw_version;
        }
        if (!strcmp(v->fw_str, new_str)) {
            new = v->fw_version;
        }
    }
	printf("old:%d, new:%d\n ", old, new);
    if (old > new) {
        printf("Downgrade [%s->%s], [%d->%d]\n", old_str, new_str, old, new);
        /* Downgrade. Use the upgrade functions from old to (new + 1). */
        for (i = old; i >= new + 1; i-- ) {
            if (nvram_fw_table[i].fw_downgrade_func) {
                (*nvram_fw_table[i].fw_downgrade_func)();
            }
        }
        /* nvram is the only place to set fw_version. */
        if (!strcmp(new_str, nvram_fw_table[0].fw_str)) {
            nvram_unset("fw_version"); /* 0.0.0 */
        } else {
            nvram_fset("fw_version", target);
        }
        change = 1;
    }
    if(strcmp(nvram_safe_get("prod_subsubcat"), "")) {
        /* unset prod_subsubcat to make sure prod_subsubcat the same with 
         * special firmware */
        nvram_unset("prod_subsubcat"); 
        change = 1;
    }
    return change;
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


