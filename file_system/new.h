#ifndef _NEW_H
#define _NEW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structure.h"

void new_fs(
	char * filename, 
	char * cmdstr, 
	char * pwd, 
	sysStatus * pstatus
);

#endif
