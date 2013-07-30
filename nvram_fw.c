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
#include <arpa/inet.h>

#include "nvram.h"
#include "nvram_rule.h"
#include "nvram_fw.h"
#include "nvram_factory.h"

//#define WAN_NUM 1
//#define LAN_NUM 1
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
/* 1.6.6 */
int fw_func_1_6_5_to_1_6_6(void);
int fw_func_1_6_6_to_1_6_5(void);
/* 1.6.7 */
int fw_func_1_6_6_to_1_6_7(void);
int fw_func_1_6_7_to_1_6_6(void);
/* 1.7.0 */
int fw_func_1_6_7_to_1_7_0(void);
int fw_func_1_7_0_to_1_6_7(void);
/* 1.7.1 */
int fw_func_1_7_0_to_1_7_1(void);
int fw_func_1_7_1_to_1_7_0(void);
/* 1.7.2 */
int fw_func_1_7_1_to_1_7_2(void);
int fw_func_1_7_2_to_1_7_1(void);
/* 1.7.3 */
int fw_func_1_7_2_to_1_7_3(void);
int fw_func_1_7_3_to_1_7_2(void);
/* 1.7.4 */
int fw_func_1_7_3_to_1_7_4(void);
int fw_func_1_7_4_to_1_7_3(void);
/* 1.7.5 */
int fw_func_1_7_4_to_1_7_5(void);
int fw_func_1_7_5_to_1_7_4(void);
/* 1.7.5 */
int fw_func_1_7_4_to_1_7_5(void);
int fw_func_1_7_5_to_1_7_4(void);
/* 1.7.6 */
int fw_func_1_7_5_to_1_7_6(void);
int fw_func_1_7_6_to_1_7_5(void);
/* 1.7.7 */
int fw_func_1_7_6_to_1_7_7(void);
int fw_func_1_7_7_to_1_7_6(void);
/* 1.7.8 */
int fw_func_1_7_7_to_1_7_8(void);
int fw_func_1_7_8_to_1_7_7(void);
/* 1.7.9 */
int fw_func_1_7_8_to_1_7_9(void);
int fw_func_1_7_9_to_1_7_8(void);
/* 1.7.10 */
int fw_func_1_7_9_to_1_7_10(void);
int fw_func_1_7_10_to_1_7_9(void);
/* 1.7.11 */
int fw_func_1_7_10_to_1_7_11(void);
int fw_func_1_7_11_to_1_7_10(void);
/* 2.0.0 */
int fw_func_1_7_11_to_2_0_0(void);
int fw_func_2_0_0_to_1_7_11(void);
/* 2.0.1 */
int fw_func_2_0_0_to_2_0_1(void);
int fw_func_2_0_1_to_2_0_0(void);
/* 2.0.2 */
int fw_func_2_0_1_to_2_0_2(void);
int fw_func_2_0_2_to_2_0_1(void);
/* 2.0.3 */
int fw_func_2_0_2_to_2_0_3(void);
int fw_func_2_0_3_to_2_0_2(void);
/* 2.0.4 */
int fw_func_2_0_3_to_2_0_4(void);
int fw_func_2_0_4_to_2_0_3(void);
/* 2.0.5 */
int fw_func_2_0_4_to_2_0_5(void);
int fw_func_2_0_5_to_2_0_4(void);

/* Added by Liteon, 2013-7-9 */

/* 2.0.6 for firmware from v0.1.6 to v0.1.7 */
int fw_func_2_0_6_to_2_0_5(void);
int fw_func_2_0_5_to_2_0_6(void);

/* 2.0.6 for firmware from v0.1.7 to v0.1.8 */
int fw_func_2_0_7_to_2_0_6(void);
int fw_func_2_0_6_to_2_0_7(void);

/* Ended by Liteon */

struct nvram_fw_tuple nvram_fw_table[] = {
    { "0.0.0", NVRAM_FW_0_0_0, NULL, NULL },   
#if 0
    { "1.6.5", NVRAM_FW_1_6_5, fw_func_0_0_0_to_1_6_5, fw_func_1_6_5_to_0_0_0 },
    { "1.6.6", NVRAM_FW_1_6_6, fw_func_1_6_5_to_1_6_6, fw_func_1_6_6_to_1_6_5 },
    { "1.6.7", NVRAM_FW_1_6_7, fw_func_1_6_6_to_1_6_7, fw_func_1_6_7_to_1_6_6 },
    { "1.7.0", NVRAM_FW_1_7_0, fw_func_1_6_7_to_1_7_0, fw_func_1_7_0_to_1_6_7 },
    { "1.7.1", NVRAM_FW_1_7_1, fw_func_1_7_0_to_1_7_1, fw_func_1_7_1_to_1_7_0 },
    { "1.7.2", NVRAM_FW_1_7_2, fw_func_1_7_1_to_1_7_2, fw_func_1_7_2_to_1_7_1 },
    { "1.7.3", NVRAM_FW_1_7_3, fw_func_1_7_2_to_1_7_3, fw_func_1_7_3_to_1_7_2 },
    { "1.7.4", NVRAM_FW_1_7_4, fw_func_1_7_3_to_1_7_4, fw_func_1_7_4_to_1_7_3 },
    { "1.7.5", NVRAM_FW_1_7_5, fw_func_1_7_4_to_1_7_5, fw_func_1_7_5_to_1_7_4 },
    { "1.7.6", NVRAM_FW_1_7_6, fw_func_1_7_5_to_1_7_6, fw_func_1_7_6_to_1_7_5 },
    { "1.7.7", NVRAM_FW_1_7_7, fw_func_1_7_6_to_1_7_7, fw_func_1_7_7_to_1_7_6 },
    { "1.7.8", NVRAM_FW_1_7_8, fw_func_1_7_7_to_1_7_8, fw_func_1_7_8_to_1_7_7 },
    { "1.7.9", NVRAM_FW_1_7_9, fw_func_1_7_8_to_1_7_9, fw_func_1_7_9_to_1_7_8 },
    { "1.7.10", NVRAM_FW_1_7_10, fw_func_1_7_9_to_1_7_10, fw_func_1_7_10_to_1_7_9 },
    { "1.7.11", NVRAM_FW_1_7_11, fw_func_1_7_10_to_1_7_11, fw_func_1_7_11_to_1_7_10 },
    { "2.0.0", NVRAM_FW_2_0_0, fw_func_1_7_11_to_2_0_0, fw_func_2_0_0_to_1_7_11 },
    { "2.0.1", NVRAM_FW_2_0_1, fw_func_2_0_0_to_2_0_1, fw_func_2_0_1_to_2_0_0 },
    { "2.0.2", NVRAM_FW_2_0_2, fw_func_2_0_1_to_2_0_2, fw_func_2_0_2_to_2_0_1 },
    { "2.0.3", NVRAM_FW_2_0_3, fw_func_2_0_2_to_2_0_3, fw_func_2_0_3_to_2_0_2 },
    { "2.0.4", NVRAM_FW_2_0_4, fw_func_2_0_3_to_2_0_4, fw_func_2_0_4_to_2_0_3 },
    { "2.0.5", NVRAM_FW_2_0_5, fw_func_2_0_4_to_2_0_5, fw_func_2_0_5_to_2_0_4 },
    /* Added by Liteon, 2013-7-9 */
    /* 2.0.6 for firmware from v0.1.6 to v0.1.7 */
    { "2.0.6", NVRAM_FW_2_0_6, fw_func_2_0_5_to_2_0_6, fw_func_2_0_6_to_2_0_5 },
    { "2.0.7", NVRAM_FW_2_0_7, fw_func_2_0_6_to_2_0_7, fw_func_2_0_7_to_2_0_6 },
    /* Ended Liteon*/
#endif
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

int fw_func_1_6_5_to_1_6_6(void)
{
    int i, j, len;
    char new[1024];
    char tmp[64];
    char *str = &new[0];
    char *rule_set = "upnp_rule";
    char *value = nvram_safe_get(rule_set);

    printf("fw_func_1_6_5_to_1_6_6\n");
    /* If empty, nvram boot will create it. */ 
    /* Check the existence of the value. */
    if (value && *value) {
        /* 
         * upnp_rule is changed in the format. 
         *
         * Old: name^enable[0|1]^extif^inif^port
         * New: name^enable[0|1]^pmp_enable[0|1]^extif^inif^port
         */

        i = j = 0;
        /* Added pmp_enable. */
        ezplib_get_attr_val(rule_set, i, "name", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s^", tmp);
        j += len;
        str += len;

        ezplib_get_attr_val(rule_set, i, "enable", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s^", tmp);
        j += len;
        str += len;

        /* pmp_enable */
        len = snprintf(str, 1024 - j, "0^", tmp);
        j += len;
        str += len;

        ezplib_get_attr_val(rule_set, i, "pmp_enable", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s^", tmp);
        j += len;
        str += len;

        ezplib_get_attr_val(rule_set, i, "extif", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s^", tmp);
        j += len;
        str += len;

        /* No ending "^". */ 
        ezplib_get_attr_val(rule_set, i, "inif", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s", tmp);
        j += len;
        str += len;

        ezplib_replace_rule(rule_set, i, new);
    }
    return 0;
}

int fw_func_1_6_6_to_1_6_5(void)
{
    int i, j, len;
    char new[1024];
    char tmp[128];
    char *str = &new[0];
    char *rule_set = "upnp_rule";
    char *value = nvram_safe_get(rule_set);

    printf("fw_func_1_6_6_to_1_6_5\n");

    /* Check the existence of the value. */
    if (value && *value) {
        i = 0;
        /* Removed pmp_enable. */
        ezplib_get_attr_val(rule_set, i, "name", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s^", tmp);
        j += len;
        str += len;

        ezplib_get_attr_val(rule_set, i, "enable", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s^", tmp);
        j += len;
        str += len;

        ezplib_get_attr_val(rule_set, i, "extif", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s^", tmp);
        j += len;
        str += len;

        ezplib_get_attr_val(rule_set, i, "inif", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s^", tmp);
        j += len;
        str += len;

        /* No ending "^". */ 
        ezplib_get_attr_val(rule_set, i, "port", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s", tmp);
        j += len;
        str += len;

        ezplib_replace_rule(rule_set, i, new);
    }
    return 0;
}
int fw_func_1_6_6_to_1_6_7(void)
{
    int i, j, len;
    char new[1024];
    char tmp[64];
    char *str = &new[0];
    char *rule_set = "wan_bw_rule";
    char *value = nvram_safe_get(rule_set);

    printf("fw_func_1_6_6_to_1_6_7\n");
    /* If empty, nvram boot will create it. */ 
    /* Check the existence of the value. */
    if (value && *value) {
        /* 
         * wan_bw_rule is changed in the value in the attribute "type". 
         *
         * e.g.,
         * Old: 2M256K
         * New: A2M256K
         */

        for (i = 0, j = 0, len = 0; i < WAN_NUM; i++) {
            /* Modified the value of "type". */
            ezplib_get_attr_val(rule_set, i, "type", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            /* Set the type default as ADSL */
            len = snprintf(str, 1024 - j, "A%s^", tmp);
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

int fw_func_1_6_7_to_1_6_6(void)
{
    int i, j, k, len;
    char new[1024];
    char tmp[128];
    char *str = &new[0];
    char *rule_set = "wan_bw_rule";
    char *value = nvram_safe_get(rule_set);
    char *type;
    char tmp_dl[128], tmp_ul[128];
    char *bw_type[] = {
        "1M64K", "2M256K", "2M512K", "3M640K", "4M1M", "8M640K", "10M2M", 
        "12M1M", NULL
    };

    printf("fw_func_1_6_7_to_1_6_6\n");

    /* Check the existence of the value. */
    if (value && *value) {
        for (i = 0, j = 0, len = 0; i < WAN_NUM; i++) {
            ezplib_get_attr_val(rule_set, i, "type", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            ezplib_get_attr_val(rule_set, i, "dl", tmp_dl, sizeof(tmp),
                    EZPLIB_USE_CLI);
            ezplib_get_attr_val(rule_set, i, "ul", tmp_ul, sizeof(tmp),
                    EZPLIB_USE_CLI);
            /* Ignore the bandwidth type: ADSL, Cable or VDSL */
            if (!strcmp(tmp, "Custom")) {
                type = "Custom";
            } else {
                type = &tmp[1];
                for (k = 0; bw_type[k]; k++) {
                    if (!strcmp(type, bw_type[k])) {
                        break;
                    }
                }
                if (!bw_type[k]) {
                    /* No matched type */
                    type = "2M256K";
                    strcpy(tmp_dl, "2048");
                    strcpy(tmp_ul, "256");
                }
            }
            len = snprintf(str, 1024 - j, "%s^%s^%s^", type, tmp_dl, tmp_ul);
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
int fw_func_1_6_7_to_1_7_0(void)
{
    printf("fw_func_1_6_7_to_1_7_0\n");
    return 0;
}
int fw_func_1_7_0_to_1_6_7(void)
{
    printf("fw_func_1_7_0_to_1_6_7\n");
    return 0;
}
int fw_func_1_7_0_to_1_7_1(void)
{
    printf("fw_func_1_7_0_to_1_7_1\n");
    return 0;
}
int fw_func_1_7_1_to_1_7_0(void)
{
    printf("fw_func_1_7_1_to_1_7_0\n");
    return 0;
}
int fw_func_1_7_1_to_1_7_2(void)
{
    printf("fw_func_1_7_1_to_1_7_2\n");
    return 0;
}
int fw_func_1_7_2_to_1_7_1(void)
{
    printf("fw_func_1_7_2_to_1_7_1\n");
    return 0;
}
int fw_func_1_7_2_to_1_7_3(void)
{
    printf("fw_func_1_7_2_to_1_7_3\n");
    return 0;
}
int fw_func_1_7_3_to_1_7_2(void)
{
    printf("fw_func_1_7_3_to_1_7_2\n");
    return 0;
}
int fw_func_1_7_3_to_1_7_4(void)
{
    printf("fw_func_1_7_3_to_1_7_4\n");
    return 0;
}
int fw_func_1_7_4_to_1_7_3(void)
{
    printf("fw_func_1_7_4_to_1_7_3\n");
    return 0;
}
int fw_func_1_7_4_to_1_7_5(void)
{
    char *rule_set = "upnp_rule";
    char tmp[32];

    printf("fw_func_1_7_4_to_1_7_5\n");

    /* Disable NAT-PMP by default since now! */
    ezplib_get_attr_val(rule_set, 0, "pmp_enable", tmp, sizeof(tmp),
		    EZPLIB_USE_CLI);
    if (!strcmp(tmp, "1")) {
        ezplib_replace_attr(rule_set, 0, "pmp_enable", "0");
    } 

    /* Change UPnP listen port from 5000 to 5555.  */
    ezplib_get_attr_val(rule_set, 0, "port", tmp, sizeof(tmp), EZPLIB_USE_CLI);
    if (!strcmp(tmp, "5000")) {
        ezplib_replace_attr(rule_set, 0, "port", "5555");
    } 

    return 0;
}
int fw_func_1_7_5_to_1_7_4(void)
{
    printf("fw_func_1_7_5_to_1_7_4\n");
    /* Do nothing since we perfer NAT-PMP disabled and port 5555. */
    return 0;
}
int fw_func_1_7_5_to_1_7_6(void)
{
    int i, j, len, rule_num, max;
    char new[1024], old[1024];
    char tmp[64], tmp2[64], tmp3[64];
    char *str = &new[0];
    char *rule_set = "ntp_rule";
    char *value = nvram_safe_get(rule_set), *value2;
    char *wl_enable, *ptr;

	char ModeTmpBuf[32];
	ezplib_get_attr_val("wl_mode_rule", 0, "mode", ModeTmpBuf, 32, EZPLIB_USE_CLI);

    printf("fw_func_1_7_5_to_1_7_6\n");

    /* If empty, nvram boot will create it. */ 
    /* Check the existence of the value. */
    if (value && *value) {
        /* 
         * upnp_rule is changed in the format. 
         *
         * Old: name^enable[0|1]^pool^zone
         * New: name^enable[0|1]^type[pool|ipaddr]^pool^ipaddr^zone
         */

        i = j = 0;
        ezplib_get_attr_val(rule_set, i, "name", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s^", tmp);
        j += len;
        str += len;

        ezplib_get_attr_val(rule_set, i, "enable", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s^", tmp);
        j += len;
        str += len;

        /* type */
        len = snprintf(str, 1024 - j, "pool^", tmp);
        j += len;
        str += len;
        /* pool: use new "type" to get old "pool". */
        ezplib_get_attr_val(rule_set, i, "type", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s^", tmp);
        j += len;
        str += len;

        /* ipaddr */
        len = snprintf(str, 1024 - j, "^", tmp);
        j += len;
        str += len;

        /* No ending "^". */ 
        /* zone: use new "pool" to get old "zone". */
        ezplib_get_attr_val(rule_set, i, "pool", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s", tmp);
        j += len;
        str += len;

        ezplib_replace_rule(rule_set, i, new);
    }

    rule_num = atoi(nvram_safe_get("ipsec_rule_num"));
    rule_set = "ipsec_rule";
    
    for (i = j = 0; i < rule_num; i++) {
        str = &new[0];
        ezplib_get_rule(rule_set, i, old, 1024);
        /* If empty, nvram boot will create it. */ 
        /* Check the existence of the value. */
        if (old[0]) {
            /* 
             * Old: name^enable^adv^mode^l2tp^local_extif^local_inipaddr^local_netmask^remote_gateway^remote_inipaddr^remote_netmask^conn_init^phase1_mode^phase1_auth_md5^phase1_auth_sha1^phase1_encrypt_des^phase1_encrypt_3des^phase1_encrypt_aes^phase1_id^phase2_auth_md5^phase2_auth_sha1^phase2_encrypt_des^phase2_encrypt_3des^phase2_encrypt_aes^phase1_lifetime^phase2_lifetime^pfs^ike_keymode^psk^rsa^ca
             * New: name^enable^mode^l2tp^local_extif^local_inipaddr^local_netmask^remote_gateway^remote_inipaddr^remote_netmask^conn_init^ike_keymode^psk^rsa^ca^adv^phase1_mode^phase1_id^phase1_lifetime^phase2_lifetime^phase1_auth^phase1_encrypt^phase1_group^phase2_auth^phase2_encrypt^phase2_group
             */

            /* name^enable */
            ezplib_get_subrule(rule_set, i, 0, 1, tmp, sizeof(tmp));
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            /* mode^l2tp^local_extif^local_inipaddr^local_netmask^remote_gateway^remote_inipaddr^remote_netmask^conn_init */
            ezplib_get_subrule(rule_set, i, 3, 11, tmp, sizeof(tmp));
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            /* ike_keymode^psk^rsa^ca */
            ezplib_get_subrule(rule_set, i, 27, 30, tmp, sizeof(tmp));
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            /* adv */
            ezplib_get_subrule(rule_set, i, 2, 2, tmp, sizeof(tmp));
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            /* phase1_mode */
            ezplib_get_subrule(rule_set, i, 12, 12, tmp, sizeof(tmp));
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            /* phase1_id */
            ezplib_get_subrule(rule_set, i, 18, 18, tmp, sizeof(tmp));
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            /* phase1_lifetime^phase2_lifetime */
            ezplib_get_subrule(rule_set, i, 24, 25, tmp, sizeof(tmp));
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            /* phase1_auth */
            ezplib_get_subrule(rule_set, i, 13, 13, tmp, sizeof(tmp));
            ezplib_get_subrule(rule_set, i, 14, 14, tmp2, sizeof(tmp));
            if (tmp[0] == '1') {
                len = snprintf(str, 1024 - j, "%s^", "md5");
            } else if (tmp2[0] == '1') {
                len = snprintf(str, 1024 - j, "%s^", "sha1");
            } else {
                /* default MD5 */
                len = snprintf(str, 1024 - j, "%s^", "md5");
            }
            j += len;
            str += len;

            /* phase1_encrypt */
            ezplib_get_subrule(rule_set, i, 15, 15, tmp, sizeof(tmp));
            ezplib_get_subrule(rule_set, i, 16, 16, tmp2, sizeof(tmp));
            ezplib_get_subrule(rule_set, i, 17, 17, tmp3, sizeof(tmp));
            if (tmp2[0] == '1') {
                len = snprintf(str, 1024 - j, "%s^", "3des");
            } else if (tmp3[0] == '1') {
                len = snprintf(str, 1024 - j, "%s^", "aes");
            } else if (tmp[0] == '1') {
                len = snprintf(str, 1024 - j, "%s^", "des");
            } else {
                /* default 3DES */
                len = snprintf(str, 1024 - j, "%s^", "3des");
            }
            j += len;
            str += len;

            /* phase1_group */
            len = snprintf(str, 1024 - j, "dh2^");
            j += len;
            str += len;

            /* phase2_auth */
            ezplib_get_subrule(rule_set, i, 19, 19, tmp, sizeof(tmp));
            ezplib_get_subrule(rule_set, i, 20, 20, tmp2, sizeof(tmp));
            if (tmp[0] == '1') {
                len = snprintf(str, 1024 - j, "%s^", "md5");
            } else if (tmp2[0] == '1') {
                len = snprintf(str, 1024 - j, "%s^", "sha1");
            } else {
                /* default MD5 */
                len = snprintf(str, 1024 - j, "%s^", "md5");
            }
            j += len;
            str += len;

            /* phase2_encrypt */
            ezplib_get_subrule(rule_set, i, 21, 21, tmp, sizeof(tmp));
            ezplib_get_subrule(rule_set, i, 22, 22, tmp2, sizeof(tmp));
            ezplib_get_subrule(rule_set, i, 23, 23, tmp3, sizeof(tmp));
            if (tmp2[0] == '1') {
                len = snprintf(str, 1024 - j, "%s^", "3des");
            } else if (tmp3[0] == '1') {
                len = snprintf(str, 1024 - j, "%s^", "aes");
            } else if (tmp[0] == '1') {
                len = snprintf(str, 1024 - j, "%s^", "des");
            } else {
                /* default 3DES */
                len = snprintf(str, 1024 - j, "%s^", "3des");
            }
            j += len;
            str += len;

            /* phase2_group */
            len = snprintf(str, 1024 - j, "dh2");
            j += len;
            str += len;

            ezplib_replace_rule(rule_set, i, new);
        }
    }


    return 0;
}
int fw_func_1_7_6_to_1_7_5(void)
{
    int i, j, len, rule_num;
    char new[1024], old[1024];
    char tmp[64];
    char *str = &new[0];
    char *rule_set = "ntp_rule";
    char *value = nvram_safe_get(rule_set), *value2;

	char ModeTmpBuf[32];
	ezplib_get_attr_val("wl_mode_rule", 0, "mode", ModeTmpBuf, 32, EZPLIB_USE_CLI);

    printf("fw_func_1_7_6_to_1_7_5\n");
    /* Check the existence of the value. */
    if (value && *value) {
        i = j = 0;
        ezplib_get_attr_val(rule_set, i, "name", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s^", tmp);
        j += len;
        str += len;

        ezplib_get_attr_val(rule_set, i, "enable", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s^", tmp);
        j += len;
        str += len;

        ezplib_get_attr_val(rule_set, i, "pool", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s^", tmp);
        j += len;
        str += len;

        ezplib_get_attr_val(rule_set, i, "zone", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        len = snprintf(str, 1024 - j, "%s", tmp);
        j += len;
        str += len;

        ezplib_replace_rule(rule_set, i, new);
    }

    rule_num = atoi(nvram_safe_get("ipsec_rule_num"));
    rule_set = "ipsec_rule";
    
    for (i = j = 0; i < rule_num; i++) {
        str = &new[0];
        ezplib_get_rule(rule_set, i, old, 1024);
        /* If empty, nvram boot will create it. */ 
        /* Check the existence of the value. */
        if (old[0]) {
            /* name^enable */
            ezplib_get_subrule(rule_set, i, 0, 1, tmp, sizeof(tmp));
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            /* adv */
            ezplib_get_attr_val(rule_set, i, "adv", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            /* mode^l2tp^local_extif^local_inipaddr^local_netmask^remote_gateway^remote_inipaddr^remote_netmask^conn_init */
            ezplib_get_subrule(rule_set, i, 2, 10, tmp, sizeof(tmp));
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            /* phase1_mode */
            ezplib_get_attr_val(rule_set, i, "phase1_mode", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            /* phase1_auth */
            ezplib_get_attr_val(rule_set, i, "phase1_auth", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            if (!strcmp(tmp, "md5")) {
                len = snprintf(str, 1024 - j, "1^0^");
            } else if (!strcmp(tmp, "sha1")) {
                len = snprintf(str, 1024 - j, "0^1^");
            } else {
                len = snprintf(str, 1024 - j, "1^1^");
            }
            j += len;
            str += len;

            /* phase1_encrypt */
            ezplib_get_attr_val(rule_set, i, "phase1_encrypt", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            if (!strcmp(tmp, "des")) {
                len = snprintf(str, 1024 - j, "1^0^0^");
            } else if (!strcmp(tmp, "3des")) {
                len = snprintf(str, 1024 - j, "0^1^0^");
            } else if (!strcmp(tmp, "aes")) {
                len = snprintf(str, 1024 - j, "0^0^1^");
            } else {
                len = snprintf(str, 1024 - j, "1^1^1^");
            }
            j += len;
            str += len;

            ezplib_get_attr_val(rule_set, i, "phase1_id", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            /* phase2_auth */
            ezplib_get_attr_val(rule_set, i, "phase2_auth", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            if (!strcmp(tmp, "md5")) {
                len = snprintf(str, 1024 - j, "1^0^");
            } else if (!strcmp(tmp, "sha1")) {
                len = snprintf(str, 1024 - j, "0^1^");
            } else {
                len = snprintf(str, 1024 - j, "1^1^");
            }
            j += len;
            str += len;

            /* phase2_encrypt */
            ezplib_get_attr_val(rule_set, i, "phase2_encrypt", tmp, sizeof(tmp),
                    EZPLIB_USE_CLI);
            if (!strcmp(tmp, "des")) {
                len = snprintf(str, 1024 - j, "1^0^0^");
            } else if (!strcmp(tmp, "3des")) {
                len = snprintf(str, 1024 - j, "0^1^0^");
            } else if (!strcmp(tmp, "aes")) {
                len = snprintf(str, 1024 - j, "0^0^1^");
            } else {
                len = snprintf(str, 1024 - j, "0^0^0^");
            }
            j += len;
            str += len;

            /* phase1_lifetime^phase2_lifetime */
            ezplib_get_subrule(rule_set, i, 18, 19, tmp, sizeof(tmp));
            len = snprintf(str, 1024 - j, "%s^", tmp);
            j += len;
            str += len;

            /* pfs */
            len = snprintf(str, 1024 - j, "0^", tmp);
            j += len;
            str += len;

            /* ike_keymode^psk^rsa^ca */
            ezplib_get_subrule(rule_set, i, 11, 14, tmp, sizeof(tmp));
            len = snprintf(str, 1024 - j, "%s", tmp);
            j += len;
            str += len;

            ezplib_replace_rule(rule_set, i, new);
        }
    }

    value = nvram_safe_get("wl0_radio");
	if (!strcmp(ModeTmpBuf, "ap")){
    	value2 = nvram_safe_get("wl_ap_basic_rule");
	} else {
		value2 = nvram_safe_get("wl_basic_rule");
	}

    if (!*value && *value2) {
        /* wl0_radio */
		if (!strcmp(ModeTmpBuf, "ap")){
        	rule_set = "wl_ap_basic_rule";
		}else {
			rule_set = "wl_basic_rule";
		}
        ezplib_get_attr_val(rule_set, 0, "enable", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        nvram_fset("wl0_radio", tmp);

        /* wl_ssid */
        rule_set = "wl_ssid_rule";
        ezplib_get_attr_val(rule_set, 0, "ssid", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        nvram_fset("wl_ssid", tmp);

        /* Unset the unused nvram variables. */
		if (!strcmp(ModeTmpBuf, "ap")){
			nvram_unset("wl_ap_basic_rule");
		}else {
			nvram_unset("wl_basic_rule");
		}
    }
    return 0;
}
int fw_func_1_7_6_to_1_7_7(void)
{
    char *rule_set;
    char new[1024], old[1024];
    char tmp[64];
    char *str = &new[0];
    int i, j, num, vnum;

	char ModeTmpBuf[32];
	ezplib_get_attr_val("wl_mode_rule", 0, "mode", ModeTmpBuf, 32, EZPLIB_USE_CLI);

    printf("fw_func_1_7_6_to_1_7_7\n");

    rule_set = "wl_wds_rule";
    ezplib_get_attr_val(rule_set, 0, "mode", tmp, sizeof(tmp), EZPLIB_USE_CLI);
    if (tmp[0] == '0') {
        /* This is due to correct the error in fw_func_1_7_5_to_1_7_6(). */
        ezplib_get_attr_val(rule_set, 0, "timeout", tmp, sizeof(tmp),
                            EZPLIB_USE_CLI);
        snprintf(new, sizeof(new), "disabled^%s", tmp);
        ezplib_replace_rule(rule_set, 0, new);
    }

    rule_set = "wl_wds_basic_rule";
    str = nvram_safe_get(rule_set);
    if (!strcmp(str, "0^1")) {
        /* This is due to correct the error in the original 
         * fw_func_1_7_5_to_1_7_6(). 
         */
        nvram_set(rule_set, WL0_WDS_BASIC_RULE);
    }

    /* wl_basic_rule:
     * From old format: enable^net_mode^txpower^channel
     * To new format: enable^net_mode^txpower^channel^bisolation
     */
    if (!strcmp(ModeTmpBuf, "ap")){
		rule_set = "wl_ap_basic_rule";
	}else {
		rule_set = "wl_basic_rule";
	}
    num = atoi(nvram_safe_get("wl_num"));
    for (i = 0; i < num; i++) {
        ezplib_get_rule(rule_set, i, old, sizeof(old));
        snprintf(new, sizeof(new), "%s^0", old);
        ezplib_replace_rule(rule_set, i, new);
    }

    /* wlX_basic_rule:
     * From old format: enable^hidden^rate^wme
     * To new format: enable^hidden^rate^wme^isolation
     */
    num = atoi(nvram_safe_get("wl_num"));
    /* From 1.7.5 to 1.7.7 directly, wlv_rule_set hasn't been set. */
    vnum = atoi(xstr(WL_RULE_NUM));
    for (i = 0; i < num; i++) {
        snprintf(tmp, sizeof(tmp), "wl%d_basic_rule", i);

        for (j = 0; j < vnum; j++) {
            ezplib_get_rule(tmp, j, old, sizeof(old));
            snprintf(new, sizeof(new), "%s^0", old);
            ezplib_replace_rule(tmp, j, new);
        }
    }

    /* Unset upnp_rule. So it falls back to default. */
    nvram_unset("upnp_rule");
}
int fw_func_1_7_7_to_1_7_6(void)
{
    char *rule_set;
    char new[1024], old[1024];
    char tmp[64];
    char *str = &new[0];
    int i, j, num, vnum;

	char ModeTmpBuf[32];
	ezplib_get_attr_val("wl_mode_rule", 0, "mode", ModeTmpBuf, 32, EZPLIB_USE_CLI);

    printf("fw_func_1_7_7_to_1_7_6\n");

    /* wl_basic_rule:
     * From new format: enable^net_mode^txpower^channel^bisolation
     * To old format: enable^net_mode^txpower^channel
     */
    if (!strcmp(ModeTmpBuf, "ap")){
		rule_set = "wl_ap_basic_rule";
	}else {
		rule_set = "wl_basic_rule";
	}
    num = atoi(nvram_safe_get("wl_num"));
    for (i = 0; i < num; i++) {
        ezplib_get_subrule(rule_set, i, 0, 3, new, sizeof(new));
        ezplib_replace_rule(rule_set, i, new);
    }

    /* wlX_basic_rule:
     * From new format: enable^hidden^rate^wme^isolation
     * To old format: enable^hidden^rate^wme
     */
    num = atoi(nvram_safe_get("wl_num"));
    vnum = atoi(nvram_safe_get("wlv_rule_num"));
    for (i = 0; i < num; i++) {
        snprintf(tmp, sizeof(tmp), "wl%d_basic_rule", i);

        for (j = 0; j < vnum; j++) {
            ezplib_get_subrule(tmp, i, 0, 3, new, sizeof(new));
            ezplib_replace_rule(tmp, j, new);
        }
    }

    /* Unset upnp_rule. So it falls back to default. */
    nvram_unset("upnp_rule");

    /* Rewind the MAC address. */
    system("echo 1 > /proc/ezp_rewind");
}
int fw_func_1_7_7_to_1_7_8(void)
{
    printf("fw_func_1_7_7_to_1_7_8\n");
    return 0;
}
int fw_func_1_7_8_to_1_7_7(void)
{
    printf("fw_func_1_7_8_to_1_7_7\n");
    return 0;
}
int fw_func_1_7_8_to_1_7_9(void)
{
    char tmp[1024], name[1024], ipaddr_start[64], ipaddr_end[64];
    char new[1024], old[1024];
    char *str;
    int nrule, i, j, dbm_nrule, len, start, end, num;

    printf("fw_func_1_7_8_to_1_7_9\n");

    nvram_fset("bw_dbm_rule", "");
    nvram_fset("bw_dbm_rule_num", "0");
    nvram_fset("bw_sbm_rule", "");
    nvram_fset("bw_sbm_rule_num", "0");

    nrule = atoi(nvram_safe_get("fl_rule_num"));
    for (i = 0; i < nrule; i++) {
        /* dbm_enable */
        ezplib_get_subrule("fl_rule", i, 14, 14, tmp, sizeof(tmp));
        if (tmp[0] == '1') {
            /* The rule with DBM enabled. */
            ezplib_get_attr_val("fl_rule", i, "name", name, sizeof(name),
                    EZPLIB_USE_CLI);
            ezplib_get_attr_val("fl_rule", i, "srcipaddr_start", ipaddr_start,
                    sizeof(ipaddr_start), EZPLIB_USE_CLI);
            ezplib_get_attr_val("fl_rule", i, "srcipaddr_end", ipaddr_end,
                    sizeof(ipaddr_end), EZPLIB_USE_CLI);
            /* 1.7.8 had a bug in L4 that the dbm_num is mistaken as 32.
             * Therefore, we cannot directly use this attribute in fl_rule.
             */
            start = ntohl((int)inet_addr(ipaddr_start));
            end = ntohl((int)inet_addr(ipaddr_end));
            num = end - start + 1;
            snprintf(tmp, sizeof(tmp), "%s^1^%s^%s^%d^", name, ipaddr_start, ipaddr_end, num);
            ezplib_append_rule("bw_dbm_rule", tmp);

            /* Increase the number of DBM rules. */
            dbm_nrule = atoi(nvram_safe_get("bw_dbm_rule_num"));
            snprintf(tmp, sizeof(tmp), "%d", dbm_nrule+1);
            nvram_set("bw_dbm_rule_num", tmp);
        }

        /* Remove the attribute 'dbm_enable' and 'dbm_num' from fl_rule */
        j = 0;
        str = &new[0];

        ezplib_get_subrule("fl_rule", i, 0, 13, tmp, sizeof(tmp));
        len = snprintf(str, sizeof(new) - j, "%s^", tmp);
        j += len;
        str += len;

        ezplib_get_subrule("fl_rule", i, 16, 18, tmp, sizeof(tmp));
        len = snprintf(str, sizeof(new) - j, "%s^", tmp);
        j += len;
        str += len;

        ezplib_replace_rule("fl_rule", i, new);
    }

    return 0;
}
int fw_func_1_7_9_to_1_7_8(void)
{
    char tmp[1024], name[1024], start[64], end[64], num[64], mark[64];
    char new[1024];
    int nrule, i, j, fl_nrule, len;
    char *str;

    printf("fw_func_1_7_9_to_1_7_8\n");

    /* Add back the attribute 'dbm_enable' and 'dbm_num' into fl_rule. */
    nrule = atoi(nvram_safe_get("fl_rule_num"));
    for (i = 0; i < nrule; i++, j += atoi(num)) {
        char ipaddr_start[64], ipaddr_end[64];

        j = 0;
        str = &new[0];

        ezplib_get_subrule("fl_rule", i, 0, 13, tmp, sizeof(tmp));
        len = snprintf(str, sizeof(new) - j, "%s^", tmp);
        j += len;
        str += len;

        /* 'dbm_enable' and 'dbm_num' */
        /* dbm_num would be updated as a correct value when executing
         * save_fl().
         */
        len = snprintf(str, sizeof(new) - j, "0^0^", tmp);
        j += len;
        str += len;

        ezplib_get_subrule("fl_rule", i, 14, 16, tmp, sizeof(tmp));
        len = snprintf(str, sizeof(new) - j, "%s^", tmp);
        j += len;
        str += len;

        ezplib_replace_rule("fl_rule", i, new);
    }

    /* Move the rules in bw_dbm_rule to fl_rule. */
    nrule = atoi(nvram_safe_get("bw_dbm_rule_num"));
    for (i = 0, j = 0; i < nrule; i++, j += atoi(num)) {
        ezplib_get_attr_val("bw_dbm_rule", i, "enable", tmp, sizeof(tmp),
                EZPLIB_USE_CLI);
        if (tmp[0] == '0') {
            continue;
        }

        ezplib_get_attr_val("bw_dbm_rule", i, "name", name, sizeof(name),
                EZPLIB_USE_CLI);
        ezplib_get_attr_val("bw_dbm_rule", i, "inipaddr_start", start,
                sizeof(start), EZPLIB_USE_CLI);
        ezplib_get_attr_val("bw_dbm_rule", i, "inipaddr_end", end,
                sizeof(end), EZPLIB_USE_CLI);
        ezplib_get_attr_val("bw_dbm_rule", i, "inipaddr_num", num,
                sizeof(num), EZPLIB_USE_CLI);

        snprintf(mark, sizeof(mark), "%04X", j+1);
        snprintf(tmp, sizeof(tmp), "%s^1^^^%s^%s^^^^^^^^1^1^%s^%s^0^0", name, start,
                end, num, mark);

        ezplib_append_rule("fl_rule", tmp);

        fl_nrule = atoi(nvram_safe_get("fl_rule_num"));
        snprintf(tmp, sizeof(tmp), "%d", fl_nrule+1);

        nvram_set("fl_rule_num", tmp);
    }
        
    /* Roll back the wan_bw_rule. */
    nrule = atoi(nvram_safe_get("wan_num"));
    for (i = 0; i < nrule; i++) {
        char dl[64], ul[64], dl_max[64], dl_min[64], ul_max[64], ul_min[64];
        char *p_link_percent = "75";
        int link_percent = 75;
        int global_percent = 90;
        int user_percent = 90;
        int bw_num, used_ul, used_dl, ulmax, ulmin, dlmax, dlmin;

        bw_num = atoi(nvram_safe_get("dbm_max"));

        ezplib_get_attr_val("wan_bw_rule", i, "dl", dl, sizeof(dl),
                EZPLIB_USE_CLI);
        ezplib_get_attr_val("wan_bw_rule", i, "ul", ul, sizeof(ul),
                EZPLIB_USE_CLI);

        used_ul = atoi(ul);
        used_dl = atoi(dl);

        ulmax = used_ul * link_percent / 100;
        ulmax = ulmax * global_percent / 100;
        ulmax = ulmax * user_percent / 100;
        ulmin = ulmax / bw_num;

        dlmax = used_dl * link_percent / 100;
        dlmax = dlmax * global_percent / 100;
        dlmax = dlmax * user_percent / 100;
        dlmin = dlmax / bw_num;

        snprintf(ul_max, sizeof(ul_max), "%d", ulmax);
        snprintf(ul_min, sizeof(ul_min), "%d", ulmin);
        snprintf(dl_max, sizeof(dl_max), "%d", dlmax);
        snprintf(dl_min, sizeof(dl_min), "%d", dlmin);

        ezplib_replace_attr("wan_bw_rule", i, "link_percent", p_link_percent);
        ezplib_replace_attr("wan_bw_rule", i, "dbm_upmax", ul_max);
        ezplib_replace_attr("wan_bw_rule", i, "dbm_upmin", ul_min);
        ezplib_replace_attr("wan_bw_rule", i, "dbm_downmax", dl_max);
        ezplib_replace_attr("wan_bw_rule", i, "dbm_downmin", dl_min);
    }

    nvram_unset("bw_dbm_rule");
    nvram_unset("bw_dbm_rule_num");
    nvram_unset("bw_sbm_rule");
    nvram_unset("bw_sbm_rule_num");

    return 0;
}
int fw_func_1_7_9_to_1_7_10(void)
{
    printf("fw_func_1_7_9_to_1_7_10\n");
    return 0;
}
int fw_func_1_7_10_to_1_7_9(void)
{
    printf("fw_func_1_7_10_to_1_7_9\n");
    return 0;
}
int fw_func_1_7_10_to_1_7_11(void)
{
    printf("fw_func_1_7_10_to_1_7_11\n");
    nvram_set("bw_mask", "0xFFFF0000");
    return 0;
}
int fw_func_1_7_11_to_1_7_10(void)
{
    printf("fw_func_1_7_11_to_1_7_10\n");
    nvram_set("bw_mask", "0x00FFFFFF");
    return 0;
}
int fw_func_1_7_11_to_2_0_0(void)
{
    printf("fw_func_1_7_11_to_2_0_0\n");
    return 0;
}
int fw_func_2_0_0_to_1_7_11(void)
{
    printf("fw_func_2_0_0_to_1_7_11\n");
    return 0;
}
int fw_func_2_0_1_to_2_0_0(void)
{
    char codepage[64], swap_size[64];
    char tmp[1024];
    printf("fw_func_2_0_1_to_2_0_0\n");
    ezplib_get_attr_val("storage_rule", 0, "codepage",
            codepage, sizeof(codepage), EZPLIB_USE_CLI);
    ezplib_get_attr_val("storage_rule", 0, "swap_size",
            swap_size, sizeof(swap_size), EZPLIB_USE_CLI);
    snprintf(tmp, sizeof(tmp), "^%s^%s^",
            codepage, swap_size);
    ezplib_replace_rule("storage_rule", 0, tmp);
    nvram_unset("storage_state_rule");
    return 0;
}
int fw_func_2_0_0_to_2_0_1(void)
{
    char buf[64];
    char tmp[1024];
    printf("fw_func_2_0_0_to_2_0_1\n");
    nvram_fset("storage_state_rule", "^");
    ezplib_get_subrule("storage_rule", 0, 1, 2, buf, sizeof(buf));
    snprintf(tmp, sizeof(tmp), "%s^4096", buf);
    ezplib_replace_rule("storage_rule", 0, tmp);
    return 0;
}
int fw_func_2_0_2_to_2_0_1(void)
{
    char buf[64];
    char tmp[1024];
    printf("fw_func_2_0_2_to_2_0_1\n");
    ezplib_get_subrule("lan_dhcps_rule", 0, 0, 6, buf, sizeof(buf));
    snprintf(tmp, sizeof(tmp), "%s", buf);
    ezplib_replace_rule("lan_dhcps_rule", 0, tmp);

    /* stats_rule */
#ifdef CONFIG_EZP_ARCH_RT305X
#if ( WAN_NUM == 1 ) && ( LAN_NUM == 1 )
    ezplib_replace_rule("stats_rule", 0,
            "^1^60^vlan1 vlan2 ppp0 br0 ra0");
#elif ( WAN_NUM == 2 ) && ( LAN_NUM == 1 ) 
    ezplib_replace_rule("stats_rule", 0,
        "^1^60^vlan3 vlan1 vlan2 ppp0 ppp1 br0 ra0");
#endif
#endif
    return 0;
}
int fw_func_2_0_1_to_2_0_2(void)
{
    char buf[64];
    char tmp[1024];
    printf("fw_func_2_0_1_to_2_0_2\n");
    ezplib_get_subrule("lan_dhcps_rule", 0, 0, 6, buf, sizeof(buf));
    snprintf(tmp, sizeof(tmp), "%s^dnsrelay^^^^", buf);
    ezplib_replace_rule("lan_dhcps_rule", 0, tmp);

    /* stats_rule */
#ifdef CONFIG_EZP_ARCH_RT305X
#if ( WAN_NUM == 1 ) && ( LAN_NUM == 1 )
    ezplib_replace_rule("stats_rule", 0,
            "^1^60^vlan1 vlan2 ppp0 br0 ra0 usb0 eth0");
#elif ( WAN_NUM == 2 ) && ( LAN_NUM == 1 ) 
    ezplib_replace_rule("stats_rule", 0,
        "^1^60^vlan3 vlan1 vlan2 ppp0 ppp1 br0 ra0 usb0 eth0 eth1");
#endif
#endif
    return 0;
}
int fw_func_2_0_3_to_2_0_2(void)
{
    char buf[256], buf2[256];
    char tmp[1024];
    int i, nrule;

	char ModeTmpBuf[32];
	ezplib_get_attr_val("wl_mode_rule", 0, "mode", ModeTmpBuf, 32, EZPLIB_USE_CLI);

    printf("fw_func_2_0_3_to_2_0_2\n");

    nrule = atoi(nvram_safe_get("wan_num"));
    /* wan_pppoe_rule */
    for (i = 0; i < nrule; i++) {
        ezplib_get_subrule("wan_pppoe_rule", i, 1, 8, buf, sizeof(buf));
        ezplib_replace_rule("wan_pppoe_rule", i, buf);
    }

    /* wan_wwan_rule */
    for (i = 0; i < nrule; i++) {
        ezplib_get_subrule("wan_wwan_rule", i, 0, 5, buf, sizeof(buf));
        ezplib_get_subrule("wan_wwan_rule", i, 7, 21, buf2, sizeof(buf2));
        snprintf(tmp, sizeof(tmp), "%s^%s", buf, buf2);
        ezplib_replace_rule("wan_wwan_rule", i, tmp);
    }

    ezplib_get_subrule("ftpd_rule", 0, 0, 9, buf, sizeof(buf));
    snprintf(tmp, sizeof(tmp), "%s", buf);
    ezplib_replace_rule("ftpd_rule", 0, tmp);
    
    /* Wireless advanced rule downgrade */
    nrule = atoi(nvram_safe_get("wl_rule_num"));
    for (i = 0; i < nrule; i++) {
		if (!strcmp(ModeTmpBuf, "ap")){
			ezplib_get_subrule("wl_ap_advanced_rule", i, 0, 13, buf, sizeof(buf));
			ezplib_replace_rule("wl_ap_advanced_rule", i, buf);
		} else {
			ezplib_get_subrule("wl_advanced_rule", i, 0, 13, buf, sizeof(buf));
			ezplib_replace_rule("wl_advanced_rule", i, buf);
		}
    }

    /* Routing rule */
    nrule = atoi(nvram_safe_get("rt_rule_num"));
    for (i = 0; i < nrule; i++) {
        ezplib_get_subrule("rt_rule", i, 0, 11, buf, sizeof(buf));
        ezplib_replace_rule("rt_rule", i, buf);
    }
    
    /* OpenDNS rule */
    nrule = atoi(nvram_safe_get("wan_num"));
    for (i = 0; i < nrule; i++) {
        ezplib_get_subrule("wan_opendns_rule", i, 0, 4, buf, sizeof(buf));
        ezplib_replace_rule("wan_opendns_rule", i, buf);
    }

    return 0;
}
int fw_func_2_0_2_to_2_0_3(void)
{
    char buf[256], buf2[256];
    char tmp[1024];
    int i, nrule;
	char ModeTmpBuf[32];
	ezplib_get_attr_val("wl_mode_rule", 0, "mode", ModeTmpBuf, 32, EZPLIB_USE_CLI);

    printf("fw_func_2_0_2_to_2_0_3\n");

    nrule = atoi(nvram_safe_get("wan_num"));
    /* wan_pppoe_rule */
    for (i = 0; i < nrule; i++) {
        ezplib_get_subrule("wan_pppoe_rule", i, 0, 7, buf, sizeof(buf));
        snprintf(tmp, sizeof(tmp), "chap^%s", buf);
        ezplib_replace_rule("wan_pppoe_rule", i, tmp);
    }
    /* wan_wwan_rule */
    for (i = 0; i < nrule; i++) {
        ezplib_get_subrule("wan_wwan_rule", i, 0, 5, buf, sizeof(buf));
        ezplib_get_subrule("wan_wwan_rule", i, 6, 20, buf2, sizeof(buf2));
        snprintf(tmp, sizeof(tmp), "%s^chap^%s", buf, buf2);
        ezplib_replace_rule("wan_wwan_rule", i, tmp);
    }
    
    ezplib_get_subrule("ftpd_rule", 0, 0, 9, buf, sizeof(buf));
    snprintf(tmp, sizeof(tmp), "%s^0", buf);
    ezplib_replace_rule("ftpd_rule", 0, tmp);
    nvram_unset("samba_rule");

    /* Wireless advanced rule */
    nrule = atoi(nvram_safe_get("wl_rule_num"));
    for (i = 0; i < nrule; i++) {
		if (!strcmp(ModeTmpBuf, "ap")){
			ezplib_get_rule("wl_ap_advanced_rule", i, buf, sizeof(buf));
			snprintf(tmp, sizeof(tmp), "%s^1^0^0^0^0^0^1^1^0^33^5^0^1^8^0", buf);
			ezplib_replace_rule("wl_ap_advanced_rule", i, tmp);
		} else {
			ezplib_get_rule("wl_advanced_rule", i, buf, sizeof(buf));
			snprintf(tmp, sizeof(tmp), "%s^1^0^0^0^0^0^1^1^0^33^5^0^1^8^0", buf);
			ezplib_replace_rule("wl_advanced_rule", i, tmp);
		} 
    }

    /* Routing rule */
    nrule = atoi(nvram_safe_get("rt_rule_num"));
    for (i = 0; i < nrule; i++) {
        ezplib_get_rule("rt_rule", i, buf, sizeof(buf));
        snprintf(tmp, sizeof(tmp), "%s^default^", buf);
        ezplib_replace_rule("rt_rule", i, tmp);
    }

    /* Dhcp rule */
    /* Replace DNS setting to default opendns */
    ezplib_replace_attr("lan_dhcps_rule", 0, "dnstype", "opendns");
    ezplib_replace_attr("lan_dhcps_rule", 0, "dnsaddr1", "208.67.220.220");
    ezplib_replace_attr("lan_dhcps_rule", 0, "dnsaddr2", "208.67.222.222");

    /* OpenDNS rule */
    nrule = atoi(nvram_safe_get("wan_num"));
    for (i = 0; i < nrule; i++) {
        ezplib_get_rule("wan_opendns_rule", i, buf, sizeof(buf));
        snprintf(tmp, sizeof(tmp), "%s^", buf);
        ezplib_replace_rule("wan_opendns_rule", i, tmp);
    }

    return 0;
}
int fw_func_2_0_4_to_2_0_3(void)
{
    char buf[256], buf2[256];
    char tmp[1024];
    int i, nrule;
    printf("fw_func_2_0_4_to_2_0_3\n");
    /* get bt rule attr */
    /* bt rule
     * enable[0|1]^peer^max_peer^max_task^download_rate^upload_rate^directory^session^load_start^port_range_start^port_range_end^dht_port^check_hash */
    ezplib_get_subrule("bt_rule", 0, 1, 1, buf, sizeof(buf));
    ezplib_get_subrule("bt_rule", 0, 3, 12, buf2, sizeof(buf2));

    /* set rtorrent_rule */
    /*rtorrent rule
     * enable[0|1]^min_peer^max_peer^max_upload^download_rate^upload_rate^directory^session^load_start^port_range_start^port_range_end^dht_port^check_hash
     */
    snprintf(tmp, sizeof(tmp), "1^10^%s^2^%s" , buf, buf2);
    nvram_set("rtorrent_rule", tmp);
    ezplib_get_subrule("bt_torrent_max", 0, 0, 0, buf, sizeof(buf));
    nvram_set("rtorrent_torrent_max", buf);
    
    /* unset bt rule */
    nvram_unset("bt_rule");
    nvram_unset("bt_torrent_max");

    /* wan_wwan_rule */
    nrule = atoi(nvram_safe_get("wan_num"));
    for (i = 0; i < nrule; i++) {
    /* apn_type^location^isp^apn^pin^mode^auth^username^passwd^demand^
     * idletime^redialperiod^mru^mtu^dialstr^brand^model^numtty^datatty^
     * ctrltty^devname^flag^turbolink
     */
        ezplib_get_subrule("wan_wwan_rule", i, 1, 22, buf, sizeof(buf));

    /* apn_type^location^isp^apn^pin^mode^auth^username^passwd^demand^
     * idletime^redialperiod^mru^mtu^dialstr^brand^model^numtty^datatty^
     * ctrltty^devname^flag
     */
        ezplib_replace_rule("wan_wwan_rule", i, buf);
    }

   /* wan_htc_rule */
    nrule = atoi(nvram_safe_get("wan_num"));
    for (i = 0; i < nrule; i++) {
   /* hostname^domain^lease^mtu^turbolink */
        ezplib_get_subrule("wan_htc_rule", i, 1, 4, buf, sizeof(buf));

   /* hostname^domain^lease^mtu */
        ezplib_replace_rule("wan_htc_rule", i, buf);
    }
    return 0;
}
int fw_func_2_0_3_to_2_0_4(void)
{
    char buf[256], buf2[256];
    char tmp[1024];
    int i, nrule;
    printf("fw_func_2_0_3_to_2_0_4\n");
    /* get rtorrent rule attr */
    /*rtorrent rule
     * enable[0|1]^min_peer^max_peer^max_upload^download_rate^upload_rate^directory^session^load_start^port_range_start^port_range_end^dht_port^check_hash
     */
    ezplib_get_subrule("rtorrent_rule", 0, 2, 2, buf, sizeof(buf));
    ezplib_get_subrule("rtorrent_rule", 0, 4, 12, buf2, sizeof(buf2));
    /* set bt_rule */
    /* bt rule
     * enable[0|1]^peer^max_peer^max_task^download_rate^upload_rate^directory^session^load_start^port_range_start^port_range_end^dht_port^check_hash
     */
    snprintf(tmp, sizeof(tmp), "1^%s^250^10^%s" , buf, buf2);
    nvram_set("bt_rule", tmp);
    /* unset rtorrent rule */
    nvram_unset("rtorrent_rule");
    nvram_unset("rtorrent_torrent_max");

    /* wan_wwan_rule */
    nrule = atoi(nvram_safe_get("wan_num"));
    for (i = 0; i < nrule; i++) {
    /* apn_type^location^isp^apn^pin^mode^auth^username^passwd^demand^
     * idletime^redialperiod^mru^mtu^dialstr^brand^model^numtty^datatty^
     * ctrltty^devname^flag
     */
        ezplib_get_subrule("wan_wwan_rule", i, 1, 22, buf, sizeof(buf));

    /* apn_type^location^isp^apn^pin^mode^auth^username^passwd^demand^
     * idletime^redialperiod^mru^mtu^dialstr^brand^model^numtty^datatty^
     * ctrltty^devname^flag^turbolink
     */
        snprintf(tmp, sizeof(tmp), "%s^0", buf);
        ezplib_replace_rule("wan_wwan_rule", i, tmp);
    }

    /* wan_htc_rule */
    nrule = atoi(nvram_safe_get("wan_num"));
    for (i = 0; i < nrule; i++) {
    /* hostname^domain^lease^mtu */
        ezplib_get_subrule("wan_htc_rule", i, 1, 4, buf, sizeof(buf));

    /* hostname^domain^lease^mtu^turbolink */
        snprintf(tmp, sizeof(tmp), "%s^0", buf);
        ezplib_replace_rule("wan_htc_rule", i, tmp);
    }
    return 0;
}
int fw_func_2_0_5_to_2_0_4(void)
{
    char tmp[256];
    printf("fw_func_2_0_5_to_2_0_4\n");
    /* Modify storage_rule form 
     * codepage^swap_size^block_size^ext_enable^target_ip^target_dir^user^pw^gp
     * to
     * codepage^swap_size^block_size
     */
    
    return 0;
}
int fw_func_2_0_4_to_2_0_5(void)
{
    char tmp[256], buf[256];
    int rule_num;
    printf("fw_func_2_0_4_to_2_0_5\n");
    /* Modify storage_rule form 
     * codepage^swap_size^block_size
     * to
     * codepage^swap_size^block_size^ext_enable^target_ip^target_dir^user^pw^gp
     */
    rule_num = atoi(nvram_safe_get("bw_sbm_rule_num"));
    if (rule_num == 8) {
        ezplib_append_rule("bw_sbm_rule",GuestLANtoLAN);
        ezplib_append_rule("bw_sbm_rule",GuestLANtoWAN);
        rule_num += 2;
        sprintf(tmp, "%d", rule_num);
        nvram_set("bw_sbm_rule_num", tmp);
    }
    rule_num = atoi(nvram_safe_get("br_rule_num"));
    if (rule_num == 2) {
        ezplib_append_rule("br_rule", BR_RULE_GUESTLAN);
        rule_num += 1;
        sprintf(tmp, "%d", rule_num);
        nvram_set("br_rule_num", tmp);
    }
    rule_num = atoi(nvram_safe_get("lan_num"));
    if (rule_num == 1) {
        ezplib_append_rule("lan_main_rule", LAN_MAIN_GUESTLAN);
        ezplib_append_rule("lan_static_rule", LAN_STATIC_GUESTLAN);
        ezplib_append_rule("lan_static_rule_default", LAN_STATIC_GUESTLAN);
        ezplib_append_rule("lan_dhcps_rule", LAN_DHCPS_GUESTLAN);
        rule_num += 1;
        sprintf(tmp, "%d", rule_num);
        nvram_set("lan_num", tmp);
    }
    ezplib_append_rule("",GuestLANtoLAN);
    return 0;
}

/* Added by Liteon, 2013-7-9 */

/* Begin: from v0.1.6 to v0.1.7 */
/* 
 * 1) Firmware version of Liteon from v0.1.6 to v0.1.7
 * 2) fw_func_2_0_6_to_2_0_5: The version of NVRAM upgrade from 2.0.6 to 2.0.5 
 * 3) fw_func_2_0_5_to_2_0_6: The version of NVRAM downgrade from 2.0.6 to 2.0.5
 */
int fw_func_2_0_6_to_2_0_5(void)
{
    char tmp[256];
    printf("fw_func_2_0_6_to_2_0_5\n");
    
    return 0;
}

int fw_func_2_0_5_to_2_0_6(void)
{
    char tmp[256], buf[256];
    int rule_num;
    printf("fw_func_2_0_5_to_2_0_6\n");

    return 0;
}
/* End: from v0.1.6 to v0.1.7 */

/* Begin: from v0.1.7 to v0.1.8 */
int fw_func_2_0_7_to_2_0_6(void)
{
    char tmp[256];
    printf("fw_func_2_0_7_to_2_0_6\n");
    
    return 0;
}
int fw_func_2_0_6_to_2_0_7(void)
{
    char tmp[256], buf[256];
    int rule_num;
    printf("fw_func_2_0_6_to_2_0_7\n");

    return 0;
}
/* End: from v0.1.7 to v0.1.8*/

/* Ended Liteon */
