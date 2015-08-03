#include <queue>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <cfloat>

#include "SD_Swag.h"
#include "xbt/dict.h"
#include "xbt/dynar.h"
#include "surf/surf.h"
#include "../simdag/private.h"
#include "xbt/sysdep.h"
#include "surf/surf_resource.h"
#include "instr/instr_interface.h"
#include "simgrid/sg_config.h"
#include "xbt/ex.h"
#include "xbt/log.h"
#include "xbt/str.h"
#include "xbt/config.h"
#include "surf/surfxml_parse.h"
#ifdef HAVE_LUA
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
#endif

#ifdef HAVE_JEDULE
	#include "instr/jedule/jedule_sd_binding.h"
#endif
