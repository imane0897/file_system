#include "structure.h"

const int diskSize = 2 * 1024 * 1024;
const int contentSize = 512 - sizeof(int);

void freeFCB(sysStatus *pstatus, int x) {
    char *dp = pstatus->disk, *mp;
    pstatus->free_fcb = x;
    mp = (char *)&(pstatus->free_fcb);
    memcpy(&dp, &mp, sizeof(int));
}

void initFCB(sysStatus *pstatus, int x, boolean flagFoder) {
    time_t temptime;
    freeFCB(pstatus, pstatus->fcbs[x].nextFCB);
    pstatus->fcbs[x].dadFCB = pstatus->pwd;
    pstatus->fcbs[x].nextFCB = -1;
    pstatus->fcbs[x].subFCB = -1;
    pstatus->fcbs[x].flagFoder = flagFoder;
    time(&temptime);
    pstatus->fcbs[x].create = temptime;
    pstatus->fcbs[x].lastChange = temptime;
    pstatus->fcbs[x].size = 0;
    pstatus->fcbs[x].nextIB = -1;
}

int checkExist(sysStatus *pstatus, char *name) {
    int i;
    for (i = pstatus->fcbs[pstatus->pwd].subFCB; i != -1;
         i = pstatus->fcbs[i].nextFCB) {
        if (!strcmp(name, pstatus->fcbs[i].filename)) {
            return i;
        }
    }
    return -1;
}

void writeFCB(sysStatus *pstatus, int fcbid) {
    char *dp = pstatus->disk + 4096 + fcbid * 64;
    char *mp;
    mp = (char *)&(pstatus->fcbs[fcbid].dadFCB);
    memcpy(&dp, &mp, sizeof(int));
    mp = (char *)&(pstatus->fcbs[fcbid].nextFCB);
    memcpy(&dp, &mp, sizeof(int));
    mp = (char *)&(pstatus->fcbs[fcbid].subFCB);
    memcpy(&dp, &mp, sizeof(int));
    mp = (char *)&(pstatus->fcbs[fcbid].strlen);
    memcpy(&dp, &mp, sizeof(int));
    *dp = pstatus->fcbs[fcbid].flagFoder;
    dp++;
    mp = (char *)&(pstatus->fcbs[fcbid].create);
    memcpy(&dp, &mp, sizeof(long));
    mp = (char *)&(pstatus->fcbs[fcbid].lastChange);
    memcpy(&dp, &mp, sizeof(long));
    mp = (char *)&(pstatus->fcbs[fcbid].size);
    memcpy(&dp, &mp, sizeof(int));
    mp = (char *)&(pstatus->fcbs[fcbid].nextIB);
    memcpy(&dp, &mp, sizeof(int));
    mp = pstatus->fcbs[fcbid].filename;
    memcpy(&dp, &mp, pstatus->fcbs[fcbid].strlen);
    *dp = 0;
    return;
}

void readFCB(sysStatus *pstatus, int fcbid, FCB *dest) {
    char *dp = pstatus->disk + 4096 + fcbid * 64;
    char *mp;
    mp = (char *)&(dest->dadFCB);
    memcpy(&mp, &dp, sizeof(int));
    mp = (char *)&(dest->nextFCB);
    memcpy(&mp, &dp, sizeof(int));
    mp = (char *)&(dest->subFCB);
    memcpy(&mp, &dp, sizeof(int));
    mp = (char *)&(dest->strlen);
    memcpy(&mp, &dp, sizeof(int));
    dest->flagFoder = *dp;
    dp++;
    mp = (char *)&(dest->create);
    memcpy(&mp, &dp, sizeof(long));
    mp = (char *)&(dest->lastChange);
    memcpy(&mp, &dp, sizeof(long));
    mp = (char *)&(dest->size);
    memcpy(&mp, &dp, sizeof(int));
    mp = (char *)&(dest->nextIB);
    memcpy(&mp, &dp, sizeof(int));
    if (0 != dest->strlen) {
        dest->filename = (char *)malloc(dest->strlen + 2);
        mp = dest->filename;
        memcpy(&mp, &dp, dest->strlen);
    }
    return;
}

void printFCB(FCB buf) {
    char *c, *l;
    c = ctime(&(buf.create));
    c[strlen(c) - 1] = 0;
    l = ctime(&(buf.lastChange));
    l[strlen(l) - 1] = 0;

    printf("dad: %d\nnext: %d\nsub: %d\nname: %s\nstrlen: %d\n"
           "flag: %s\ncreate: %s\nlast: %s\nsize: %d\nnextIB: %d\n",
           buf.dadFCB, buf.nextFCB, buf.subFCB, buf.filename, buf.strlen,
           buf.flagFoder ? "True" : "False", c, l, buf.size, buf.nextIB);
}

void writeIBp(sysStatus *pstatus, int id, int next) {
    int *dp = (int *)(pstatus->disk + 512 * (64 + id));
    pstatus->ibs[id].nextIB = next;
    // memcpy(&dp, &next, sizeof(int));
    *dp = next;
}

void freeIBp(sysStatus *pstatus, int id) {
    char *dp = pstatus->disk + sizeof(int);
    char *mp = (char *)&id;
    pstatus->free_ib = id;
    memcpy(&dp, &mp, sizeof(int));
}

void clearIB(sysStatus *pstatus, int id) {
    int i;
    if (-1 == id) {
        return;
    }
    for (i = id; ~pstatus->ibs[i].nextIB; i = pstatus->ibs[i].nextIB)
        ;
    writeIBp(pstatus, i, pstatus->free_ib);
    freeIBp(pstatus, id);
}

int newIB(sysStatus *pstatus) {
    int ret = pstatus->free_ib;
    freeIBp(pstatus, pstatus->ibs[ret].nextIB);
    writeIBp(pstatus, ret, -1);
    return ret;
}

void readIB(sysStatus *pstatus, int id) {
    char *dp = pstatus->disk + 512 * (64 + id),
         *mp = (char *)&pstatus->ibs[id].nextIB;
    memcpy(&mp, &dp, sizeof(int));
}

void readStatus(sysStatus *pstatus) {
    char *dp, *mp;
    dp = pstatus->disk;
    mp = (char *)&pstatus->free_fcb;
    memcpy(&mp, &dp, sizeof(int));
    mp = (char *)&pstatus->free_ib;
    memcpy(&mp, &dp, sizeof(int));
    pstatus->full_fcb = 0;
}
