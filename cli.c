#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include "cli.h"
#include "nvram_rule.h"

void puts_trim_cr(char *str)
{   
	int len;

	if (!str) return;

	len = strlen(str);
	if (len && (str[len-1] == '\r')) len--;
	printf("%.*s\n", len, str);
}


int do_show(void)
{
	char *name, buf[NVRAM_SPACE];
	int size, crc;

	/* nvram show */
	nvram_getall(buf, sizeof(buf));
	memcpy(&crc, buf, 4);

	/*get crc size in nvram*/
	size = strlen(buf) + 1;

	/*show contents start second string, hide crc content*/
	for (name = buf + strlen(name) + 1; *name; name += strlen(name) + 1)
	{
		puts_trim_cr(name);
	}

	size = size + (int) name - (int) buf;

	fprintf(stderr, "crc = %x\n", crc);
	fprintf(stderr, "size: %d bytes (%d left)\n",
			size, NVRAM_SPACE - size);
	return 0;
}


int do_get(const char *var)
{
	const char *val;
	int stat = 1;

	if( (val = nvram_get(var)) != NULL )
	{
		printf("%s\n", val);
		stat = 0;
	}

	return stat;
}

int do_unset(const char *var)
{
	return nvram_unset(var);
}


int do_set(const char *pair)
{
	char *val = strstr(pair, "=");
	char var[strlen(pair)];
	int stat = 1;

	if( val != NULL )
	{
		memset(var, 0, sizeof(var));
		strncpy(var, pair, (int)(val-pair));
		stat = nvram_set(var, (char *)(val + 1));
	}

	return stat;
}

int do_fset(const char *pair)
{
	char *val = strstr(pair, "=");
	char var[strlen(pair)];
	int stat = 1;

	if( val != NULL )
	{
		memset(var, 0, sizeof(var));
		strncpy(var, pair, (int)(val-pair));
		stat = nvram_fset(var, (char *)(val + 1));
	}

	return stat;
}


int do_info()
{
#if 0
	nvram_header_t *hdr = nvram_header();
	if(NULL == hdr){
		return -1;
	}

	/* CRC8 over the last 11 bytes of the header and data bytes */
	uint8_t crc = hndcrc8((unsigned char *) &hdr[0] + NVRAM_CRC_START_POSITION,
			hdr->len - NVRAM_CRC_START_POSITION, 0xff);

	/* Show info */
	printf("Magic:         0x%08X\n",   hdr->magic);
	printf("Length:        0x%08X\n",   hdr->len);
	printf("Offset:        0x%08X\n",   get_nvram_handle()->offset);

	printf("CRC8:          0x%02X (calculated: 0x%02X)\n",
			hdr->crc_ver_init & 0xFF, crc);

	printf("Version:       0x%02X\n",   (hdr->crc_ver_init >> 8) & 0xFF);
	printf("SDRAM init:    0x%04X\n",   (hdr->crc_ver_init >> 16) & 0xFFFF);
	printf("SDRAM config:  0x%04X\n",   hdr->config_refresh & 0xFFFF);
	printf("SDRAM refresh: 0x%04X\n",   (hdr->config_refresh >> 16) & 0xFFFF);
	printf("NCDL values:   0x%08X\n\n", hdr->config_ncdl);

	printf("%i bytes used / %i bytes available (%.2f%%)\n",
			hdr->len, NVRAM_SPACE - hdr->len,
			(100.00 / (double)NVRAM_SPACE) * (double)hdr->len);

#endif
	return 0;
}


int main( int argc, const char *argv[] )
{
	int stat = 1;
	int done = 0;
	char res[EZPLIB_BUF_LEN];

	if( argc > 1 )
	{
		--argc;
		++argv;
		if( !strncmp(*argv, "show", 4) ) {
			--argc;
			++argv;
			/* nvram show */
			if( 0==argc ) {
				stat = do_show();
				done++;
			}
			/* nvram show <rule-set> <nth> */
			else if (argc == 2) {
				stat = ezplib_get_rule(argv[0], atoi(argv[1]), 
						res, EZPLIB_BUF_LEN);
				puts_trim_cr(res);
				done++;
			} 
			/* nvram show <rule-set> <nth> <attr-type> */
			else if (argc == 3) {
				stat = ezplib_get_attr_val(argv[0], atoi(argv[1]), 
					argv[2], res, EZPLIB_BUF_LEN, EZPLIB_USE_CLI);
				puts_trim_cr(res);
				done++;
			} 
			/* nvram show <rule-set> <nth> subrule <start> <end> */
			else if (argc == 5 && !strncmp(argv[2], "subrule", strlen(argv[1]))) {
				stat = ezplib_get_subrule(argv[0], atoi(argv[1]), atoi(argv[3]),
						atoi(argv[4]), res, EZPLIB_BUF_LEN);
				puts_trim_cr(res);
				done++;
			}
			else {
				done = 0;
			}

		}
		/* nvram info */
		else if( !strncmp(*argv, "info", 4) ) {
			//stat = do_info();
			done++;
		}
		/* nvram get <rule>*/
		else if (!strncmp(*argv, "get", 3)) {
			if (*++argv) {
				stat = do_get(*argv);
				done++;
			} else {
				fprintf(stderr, "Command '%s' requires an argument!\n", 
						*--argv);
				done = 0;
			}
		}
		/* nvram set <rule=value>*/
		else if (!strncmp(*argv, "set", 3)) {
			if (*++argv) {
				stat = do_set(*argv);
				done++;
			} else {
				fprintf(stderr, "Command '%s' requires an argument!\n", 
						*--argv);
				done = 0;
			}
		}
		/* nvram fset <rule=value>*/
		else if (!strncmp(*argv, "fset", 4)) {
			if (*++argv) {
				stat = do_fset(*argv);
				done++;
			} else {
				fprintf(stderr, "Command '%s' requires an argument!\n", 
						*--argv);
				done = 0;
			}
		}
		/* nvram unset <rule>*/
		else if (!strncmp(*argv, "unset", 5)) {
			if (*++argv) {
				stat = do_unset(*argv);
				done++;
			} else {
				fprintf(stderr, "Command '%s' requires an argument!\n", 
						*--argv);
				done = 0;
			}
		}
		/* nvram export <backup_file>*/
		else if (!strncmp(*argv, "export", 6)) {
			if (*++argv) {
				stat = nvram_export(*argv);
				done++;
			} else {
				fprintf(stderr, "Command '%s' requires an argument!\n", 
						*--argv);
				done = 0;
			}
		}
		/* nvram import <backup_file>*/
		else if (!strncmp(*argv, "import", 6)) {
			if (*++argv) {
				stat = nvram_import(*argv);
				done++;
			} else {
				fprintf(stderr, "Command '%s' requires an argument!\n", 
						*--argv);
				done = 0;
			}
		}
		/*nvram upgrade <version> */
		else if (!strncmp(*argv, "upgrade", 7)) {
			argc -= 1;
			argv += 1;
			if (nvram_upgrade((argc >= 1)? argv[0]: NULL)) {
				nvram_commit();
				done++;
				stat = 0;
			}
			else {
				fprintf(stderr, "nvram upgrade error\n", 
						*--argv);
				done = 0;
			}
		}
		/*nvram downgrade version */
		else if (!strncmp(*argv, "downgrade", 9)) {
			argc -= 1;
			argv += 1;
			if (argc == 1) {
				if (nvram_downgrade(argv[0])) {
					nvram_commit();
					done++;
					stat = 0;
				}
			}  
			else {
				fprintf(stderr, "nvram downgrade error\n");
				done = 0;
			}
		}
		else if (!strncmp(argv[0], "replace", strlen(argv[0]))) {
			/* nvram replace rule <rule-set> <nth> <new-rule> */
			if (!strncmp(argv[1], "rule", strlen(argv[1]))) {
				argc -= 2;
				argv += 2;
				if (argc == 3) {
					int ret;

					ret =
						ezplib_replace_rule(argv[0], atoi(argv[1]),
								argv[2]);
					if (ret < 0) {
						printf("NVRAM replace rule %s failed:%d!\n", argv[0], ret);
						return 1;
					} else  {
						return 0;
					}
				}
			} 
			/* nvram replace attr <rule-set> <nth> <attr> <new-rule> */
			else if (!strncmp(argv[1], "attr", strlen(argv[1]))) {
				argc -= 2;
				argv += 2;
				if (argc == 4) {
					int ret;

					ret =
						ezplib_replace_attr(argv[0], atoi(argv[1]), argv[2],
								argv[3]);
					if (ret < 0) {
						printf("NVRAM replace attr %s failed: %d\n", argv[0], ret);
						return 1;
					} else  {
						return 0;
					}
				}
			}
		}
		/* nvram append rule <rule-set> <new-rule> */
		else if (!strncmp(argv[0], "append", strlen(argv[0])) &&
				!strncmp(argv[1], "rule", strlen(argv[1]))) {
			argc -= 2;
			argv += 2;
			if (argc == 2) {
				int ret;

				ret = ezplib_append_rule(argv[0], argv[1]);
				if (ret < 0) {
					printf("NVRAM append rule %s failed: %d\n", argv[0], ret);
					return 1;
				} else  {
					return 0;
				}
			}
		} 
		/* nvram prepend rule <rule-set> <new-rule> */
		else if (!strncmp(argv[0], "prepend", strlen(argv[0])) &&
				!strncmp(argv[1], "rule", strlen(argv[1]))) {
			argc -= 2;
			argv += 2;
			if (argc == 2) {
				int ret;

				ret = ezplib_prepend_rule(argv[0], argv[1]);
				if (ret < 0) {
					printf("NVRAM prepend rule %s failed: %d\n", argv[0], ret);
					return 1;
				} else  {
					return 0;
				}
			}
		}
		/* nvram add rule <rule-set> <nth> <new-rule> */
		else if (!strncmp(argv[0], "add", strlen(argv[0])) &&
				!strncmp(argv[1], "rule", strlen(argv[1]))) {
			argc -= 2;
			argv += 2;
			if (argc == 3) {
				int ret;

				ret = ezplib_add_rule(argv[0], atoi(argv[1]), argv[2]);
				if (ret < 0) {
					printf("NVRAM add rule %s failed: %d\n", argv[0], ret);
					return 1;
				} else  {
					return 0;
				}
			}
		} 
		/* nvram delete rule <rule-set> <nth> */
		else if (!strncmp(argv[0], "delete", strlen(argv[0])) &&
				!strncmp(argv[1], "rule", strlen(argv[1]))) {
			argc -= 2;
			argv += 2;
			if (argc == 2) {
				int ret;

				ret = ezplib_delete_rule(argv[0], atoi(argv[1]));
				if (ret < 0) {
					printf("NVRAM delete rule %s failed: %d\n", argv[0], ret);
					return 1;
				} else {
					return 0;
				}
			}
		}
		/* nvram rule num <rule-set> */
		else if (!strncmp(argv[0], "rule", strlen(argv[0])) &&
				!strncmp(argv[1], "num", strlen(argv[1]))) {
			argc -= 2;
			argv += 2;
			if (argc == 1) {
				int ret;

				ret = ezplib_get_rule_num(argv[0]);
				if (ret < 0) {
					printf("Rule %s doesn't exist: %d\n", argv[0], ret);
				}
				else {
					printf("%d\n", ret);
					return 0;
				}
			}
		}
		else if (!strncmp(*argv, "boot", 4)) {
			nvram_boot();
			stat = 0;
			done++;
		}
		/* nvram default */
		else if( !strncmp(*argv, "default", 7) )
		{
			if(argc == 1) {
				stat = nvram_default();
			}
			else {
				/* Added for single rule default */
				argv++;
				stat = nvram_default_rule(*argv);
			}
			done++;
		}
		/* nvram factory */
		else if( !strncmp(*argv, "factory", 7) )
		{
			stat = nvram_factory();
			/* send SIGTERM to init for reboot */
			if(!stat)
				kill(1, 15);
			done++;
		}
		/* nvram commit */
		else if( !strncmp(*argv, "commit", 6) )
		{
			stat = nvram_commit();
			done++;
		}
		/* nvram dump */
		else if( !strncmp(*argv, "dump", 4) )
		{
//			stat = nvram_dump();
			done++;
		}
		/* nvram mtd 
		else if( !strncmp(*argv, "init", 4) )
		{
			stat = *((int *)nvram_init());
			done++;
		}
		*/
		else
		{
			fprintf(stderr, "Unknown option '%s' !\n", *argv);
			done = 0;
		}
	}
	else if( !done )
	{
		fprintf(stderr,
				"Usage:\n"
				"	nvram show\n"
				"	nvram show <rule-set> <nth>\n"
				"	nvram show <rule-set> <nth> <attr-type>\n"
				"	nvram show <rule-set> <nth> subrule <start> <end>\n"
				"	nvram get <rule>\n"
				"	nvram set/fset <rule=value> \n"
				"	nvram unset <rule> \n"
				"	nvram export/import <backup_file>\n"
				"	nvram upgrade <version>\n"
				"	nvram downgrade version\n"
				"	nvram replace rule <rule-set> <nth> <new-rule>\n"
				"	nvram replace attr <rule-set> <nth> <attr> <new-attr> \n"
				"	nvram append rule <rule-set> <new-rule>\n"
				"	nvram prepend rule <rule-set> <new-rule>\n"
				"	nvram add rule <rule-set> <nth> <new-rule>\n"
				"	nvram delete rule <rule-set> <nth>\n"
				"	nvram rule num <rule-set>\n"
				"	nvram boot\n"
				"	nvram default [rule-set]\n"				//TODO
				"	nvram factory\n"						//TODO
				"	nvram commit\n"
			   );
	}
	return stat;
}
