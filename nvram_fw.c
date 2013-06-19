/*
 * NVRAM variable manipulation (Linux user mode half)
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: nvram_linux.c 4726 2006-09-02 18:37:11Z nbd $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "nvram.h"
#include "nvram_rule.h"
#include "nvram_fw.h"

#define WAN_NUM 1
#define LAN_NUM 1
#define WL_NUM 1


/* Globals */
#if defined(EZPLIB_MMAP) && EZPLIB_MMAP > 0
static char *nvram_buf = NULL;
#endif
int check_action(void);
int file_to_buf(char *path, char *buf, int len);

/* Firmware. */
int fw_func_1_6_5_to_0_0_0(void);
int fw_func_0_0_0_to_1_6_5(void);

struct nvram_fw_tuple nvram_fw_table[] = {
    { "0.0.0", EZPLIB_FW_0_0_0, NULL, NULL },   
    { "1.6.5", EZPLIB_FW_1_6_5, fw_func_0_0_0_to_1_6_5, fw_func_1_6_5_to_0_0_0 },
    { NULL, 0, NULL, NULL }
};


int fw_func_0_0_0_to_1_6_5(void)
{
    int i, j, len;
    char new[1024];
    char tmp[64];
    char *str = &new[0];
    char *rule_set = "wan_bw_rule";
    char *value = nvram_safe_get(rule_set);

    printf("fw_func_0_0_0_to_1_6_5\n");
    /* wanX_bw_mode will be added at "nvram boot" if it doesn't exist. */

    /* If empty, nvram boot will create it. */ 
    if (value && *value) {
        /* 
         * wan_bw_rule is changed in the format. 
         *
         * Old format: type^dl^ul^link_percent^global_percent^user_percent^
         * dbm_upmax^dbm_upmin^dbm_downmax^dbm_downmin 
         * New format: type^dl^ul^expert_dl^expert_ul^link_percent^
         * global_percent^user_percent^dbm_upmax^dbm_upmin^dbm_downmax^
         * dbm_downmin 
         */
		for (i = 0, j = 0, len = 0; i < WAN_NUM; i++) {
            /* Removed expert_dl/expert_ul. */
            ezplib_get_attr_val(rule_set, i, "type", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "dl", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "ul", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            /* expert_dl */
            len = snprintf(str, 1024 - j, "0^", tmp);
            j += len;
            str += len;
            /* expert_ul */
            len = snprintf(str, 1024 - j, "0^", tmp);
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "expert_dl", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "expert_ul", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "link_percent", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "global_percent", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "user_percent", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "dbm_upmax", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            /* No ending "^". */ 
            ezplib_get_attr_val(rule_set, i, "dbm_upmin", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s", tmp);
            j += len;
            str += len;

            ezplib_replace_rule(rule_set, i, new);
        }
    }
    return 0;
}

int fw_func_1_6_5_to_0_0_0(void)
{
    int i, j, len;
    char new[1024];
    char tmp[128];
    char *str = &new[0];
    char *rule_set = "wan_bw_rule";
    char *value = nvram_safe_get(rule_set);

    printf("fw_func_1_6_5_to_0_0_0\n");
    /* wanX_bw_mode */
    for (i = 0; i < WAN_NUM; i++) {
        snprintf(tmp, 64, "wan%d_bw_mode", i);
        nvram_unset(tmp);    
    }
    /* Check the existence of the value. */
    if (value && *value) {
        for (i = 0, j = 0, len = 0; i < WAN_NUM; i++) {
            /* Removed expert_dl/expert_ul. */
            ezplib_get_attr_val(rule_set, i, "type", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "dl", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "ul", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "link_percent", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "global_percent", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "user_percent", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "dbm_upmax", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "dbm_upmin", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "dbm_downmax", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            /* No ending "^". */ 
            ezplib_get_attr_val(rule_set, i, "dbm_downmin", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s", tmp);
            j += len;
            str += len;

            ezplib_replace_rule(rule_set, i, new);
        }
    }
    return 0;
}

