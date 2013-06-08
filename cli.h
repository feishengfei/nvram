#ifndef __CLI_H_
#define __CLI_H_
#include "nvram.h"

void puts_trim_cr(char *str);

/*inner function*/
int _do_show(nvram_handle_t *nvram);
int _do_get(nvram_handle_t *nvram, const char *var);
int _do_set(nvram_handle_t *nvram, const char *pair);
int _do_unset(nvram_handle_t *nvram, const char *var);
int _do_info(nvram_handle_t *nvram);

/*public function*/
int do_show();
int do_info();
int do_get(const char *var);
int do_set(const char *pair);
int do_fset(const char *pair);
int do_unset(const char *var);

#endif
