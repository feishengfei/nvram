#ifndef __CLI_H_
#define __CLI_H_
#include "nvram.h"

int do_show(nvram_handle_t *nvram);
int do_get(nvram_handle_t *nvram, const char *var);
int do_unset(nvram_handle_t *nvram, const char *var);
int do_set(nvram_handle_t *nvram, const char *pair);
int do_info(nvram_handle_t *nvram);

#endif
