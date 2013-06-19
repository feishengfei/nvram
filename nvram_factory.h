#ifndef __EZPLIB_FACTORY_H_
#define __EZPLIB_FACTORY_H_
#include "nvram.h"

nvram_tuple_t nvram_factory_default[] = {                        
	{ "abc_rule", "a^0x61|b^0x62|c^0x63",                                   
		EZPLIB_NONE, 0                                                                     
	}, 
	{ "abc", "a^0x61|b^0x62|c^0x63",                                   
		EZPLIB_NONE, 0                                                                     
	}, 
	{0,0,0,0}
};
#endif
