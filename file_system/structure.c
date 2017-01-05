#include "structure.h"
const char True = 1;
const char False = 0;
const int diskSize = 2 * 1024 * 1024;
const int contentSize = 512 - sizeof(int);

void copyContent(char **dp, char **mp, int size) {
    for (int i = 0; i < size; i++) {
        *(*dp) = *(*mp);
        (*dp)++;
        (*mp)++;
    }
    return;
}

void freeFCB(sysStatus *pstatus, int x) {
    char *dp = pstatus->disk;
    char *mp;
    pstatus->free_fcb = x;
    mp = (char *)&(pstatus->free_fcb);
    copyContent(&dp, &mp, sizeof(int));
}

void initFCB(sysStatus *pstatus, int x, boolean flagFolder) {
    time_t temptime;
    freeFCB(pstatus, pstatus->fcbs[x].nextFCB);

    pstatus->fcbs[x].dadFCB = pstatus->pwd;
    pstatus->fcbs[x].nextFCB = -1;
    pstatus->fcbs[x].subFCB = -1;
    pstatus->fcbs[x].flagFolder = flagFolder;
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
    copyContent(&dp, &mp, sizeof(int));
    mp = (char *)&(pstatus->fcbs[fcbid].nextFCB);
    copyContent(&dp, &mp, sizeof(int));
    mp = (char *)&(pstatus->fcbs[fcbid].subFCB);
    copyContent(&dp, &mp, sizeof(int));
    mp = (char *)&(pstatus->fcbs[fcbid].strlen);
    copyContent(&dp, &mp, sizeof(int));

    *dp = pstatus->fcbs[fcbid].flagFolder;
    dp++;

    mp = (char *)&(pstatus->fcbs[fcbid].create);
    copyContent(&dp, &mp, sizeof(long));
    mp = (char *)&(pstatus->fcbs[fcbid].lastChange);
    copyContent(&dp, &mp, sizeof(long));
    mp = (char *)&(pstatus->fcbs[fcbid].size);
    copyContent(&dp, &mp, sizeof(int));
    mp = (char *)&(pstatus->fcbs[fcbid].nextIB);
    copyContent(&dp, &mp, sizeof(int));
    mp = pstatus->fcbs[fcbid].filename;

    copyContent(&dp, &mp, pstatus->fcbs[fcbid].strlen);
    *dp = 0;
    return;
}

void readFCB(sysStatus *pstatus, int fcbid, FCB *dest) {
    char *dp = pstatus->disk + 4096 + fcbid * 64;
    char *mp;
    mp = (char *)&(dest->dadFCB);
    copyContent(&mp, &dp, sizeof(int));
    mp = (char *)&(dest->nextFCB);
    copyContent(&mp, &dp, sizeof(int));
    mp = (char *)&(dest->subFCB);
    copyContent(&mp, &dp, sizeof(int));
    mp = (char *)&(dest->strlen);
    copyContent(&mp, &dp, sizeof(int));
    dest->flagFolder = *dp;
    dp++;
    mp = (char *)&(dest->create);
    copyContent(&mp, &dp, sizeof(long));
    mp = (char *)&(dest->lastChange);
    copyContent(&mp, &dp, sizeof(long));
    mp = (char *)&(dest->size);
    copyContent(&mp, &dp, sizeof(int));
    mp = (char *)&(dest->nextIB);
    copyContent(&mp, &dp, sizeof(int));
    if (0 != dest->strlen) {
        dest->filename = (char *)malloc(dest->strlen + 2);
        mp = dest->filename;
        copyContent(&mp, &dp, dest->strlen);
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
           buf.flagFolder ? "True" : "False", c, l, buf.size, buf.nextIB);
}

void writeIBp(sysStatus *pstatus, int id, int next) {
    char *dp = pstatus->disk + 512 * (64 + id), *mp = (char *)&next;
    pstatus->ibs[id].nextIB = next;
    copyContent(&dp, &mp, sizeof(int));
}

void freeIBp(sysStatus *pstatus, int id) {
    char *dp = pstatus->disk + sizeof(int), *mp = (char *)&id;
    pstatus->free_ib = id;
    copyContent(&dp, &mp, sizeof(int));
}

void clearIB(sysStatus *pstatus, int id) {
    int i;
    if (id == -1) {
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
    copyContent(&mp, &dp, sizeof(int));
}

void readStatus(sysStatus *pstatus) {
    char *dp, *mp;
    dp = pstatus->disk;
    mp = (char *)&pstatus->free_fcb;
    copyContent(&mp, &dp, sizeof(int));
    mp = (char *)&pstatus->free_ib;
    copyContent(&mp, &dp, sizeof(int));
    pstatus->full_fcb = 0;
}
