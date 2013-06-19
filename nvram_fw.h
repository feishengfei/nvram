#ifndef _EZPLIB_FW_H_
#define _EZPLIB_FW_H_

typedef enum
{
    EZPLIB_FW_0_0_0 = 0,
    EZPLIB_FW_1_6_5,
} nvram_fw_version;

struct nvram_fw_tuple {
    char *fw_str;
    nvram_fw_version fw_version;
    int (*fw_upgrade_func)(void); 
    int (*fw_downgrade_func)(void); 
};


#endif /* _EZPLIB_FW_H_ */

