#ifndef __NVRAM_FACTORY_H_
#define __NVRAM_FACTORY_H_
#include "nvram.h"

nvram_tuple_t nvram_factory_default[] = {                        
	{ "abc_rule", "a^0x61|b^0x62|c^0x63",                                   
		NVRAM_NONE, 0                                                                     
	}, 
	{ "abc", "a^0x61|b^0x62|c^0x63",                                   
		NVRAM_NONE, 0                                                                     
	}, 

	//goahead
	{ "lan_static_rule", "192.168.1.20^255.255.255.0^192.168.1.1^^^^^^^^^^",
		NVRAM_NONE, 0
	},
	{ "http_rule", "^1^0^admin^1234^guest^1234^admin^80^^5",
		NVRAM_NONE, 0
	},

	{0,0,0,0}
};
#endif
