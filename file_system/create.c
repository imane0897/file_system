#include "create.h"
#include <stdbool.h>

void create_file(
	char * pwd,
	char * cmdstr,
	sysStatus * pstatus
)
{
	char filename[1025];
	int newfcbid;

	if ( '\0' == cmdstr[0] || '\n' == cmdstr[0] )
	{
		printf(">>> create [filename]\n"
			">>> Please enter the name of the new file.\n");
		return ;
	}
	else
	{
		sscanf(cmdstr, "%s", filename);
		if ( ~ checkExist(pstatus, filename) )
		{
			printf("\033[31m>>> Name exists!\033[0m\n");
			return ;
		}
		if ( strlen(filename) > 22 )
		{
			printf("\033[31m>> file name is too long.\033[0m\n");
			return ;
		}
		if ( pstatus->pwd )
		{
			printf(">>> new [%s/%s]\n", pwd, filename);
		}
		else
		{
			printf(">>> new [%s%s]\n", pwd, filename);
		}
	}
	// new a directory in FCB
	newfcbid = pstatus->free_fcb;
	initFCB(pstatus, newfcbid, false);
	pstatus->fcbs[newfcbid].nextFCB = pstatus->fcbs[pstatus->pwd].subFCB;
	pstatus->fcbs[newfcbid].strlen = strlen(filename);
	pstatus->fcbs[newfcbid].filename = (char *) malloc(pstatus->fcbs[newfcbid].strlen+2);
	strcpy(pstatus->fcbs[newfcbid].filename, filename);
	pstatus->fcbs[pstatus->pwd].subFCB = newfcbid;
	writeFCB(pstatus, newfcbid);
	writeFCB(pstatus, pstatus->pwd);
	return ;
}

