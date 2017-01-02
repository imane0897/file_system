#ifndef _SYNC_LOAD_H
#define _SYNC_LOAD_H

#include <stdio.h>
#include <string.h>

#include "structure.h"

void sync_file(sysStatus * pstatus, char * filename);

void load_file(sysStatus * pstatus, char * cmdstr, char * filename, char * pwd);

#endif
