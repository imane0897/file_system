#include "rm.h"
#include <stdbool.h>

void remove_file(sysStatus *pstatus, char *cmdstr) {
    char filename[1025];
    int i, last;

    if ('\0' == cmdstr[0] || '\n' == cmdstr[0]) {
        printf(">>> delete|rm [filename]\n"
               ">>> Please enter the name of the exist file.\n");
        return;
    } else {
        sscanf(cmdstr, "%s", filename);
    }
    for (i = pstatus->fcbs[pstatus->pwd].subFCB, last = -1; i != -1;
         i = pstatus->fcbs[i].nextFCB) {
        if (!strcmp(filename, pstatus->fcbs[i].filename)) {
            if (pstatus->fcbs[i].flagFoder) {
                printf("\033[31m>>> Not a file!\033[0m\n");
                return;
            }
            if (-1 == last) {
                pstatus->fcbs[pstatus->pwd].subFCB = pstatus->fcbs[i].nextFCB;
                writeFCB(pstatus, pstatus->pwd);
            } else {
                pstatus->fcbs[last].nextFCB = pstatus->fcbs[i].nextFCB;
                writeFCB(pstatus, last);
            }
            pstatus->fcbs[i].dadFCB = pstatus->fcbs[i].subFCB = -1;
            pstatus->fcbs[i].strlen = pstatus->fcbs[i].size = 0;
            pstatus->fcbs[i].nextFCB = pstatus->free_fcb;
            free(pstatus->fcbs[i].filename);
            pstatus->fcbs[i].filename = NULL;
            pstatus->fcbs[i].create = 0;
            pstatus->fcbs[i].lastChange = 0;
            pstatus->fcbs[i].flagFoder = false;
            freeFCB(pstatus, i);
            clearIB(pstatus, pstatus->fcbs[i].nextIB);
            pstatus->fcbs[i].nextIB = -1;
            writeFCB(pstatus, i);
            return;
        }
        last = i;
    }
    printf("\033[31m>>> File does not exist!\033[0m\n");
    return;
}
