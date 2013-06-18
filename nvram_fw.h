#ifndef _NVRAM_FW_H_
#define _NVRAM_FW_H_

typedef enum
{
    NVRAM_FW_0_0_0 = 0,
    NVRAM_FW_1_6_5,
} nvram_fw_version;

struct nvram_fw_tuple {
    char *fw_str;
    nvram_fw_version fw_version;
    int (*fw_upgrade_func)(void); 
    int (*fw_downgrade_func)(void); 
};


#endif /* _NVRAM_FW_H_ */

