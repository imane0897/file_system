#ifndef _MKDIR_H
#define _MKDIR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structure.h"

void make_dir(
	char * pwd,
	char * cmdstr,
	sysStatus * pstatus
);

#endif
