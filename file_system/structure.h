#ifndef _STRUCTURE_H
#define _STRUCTURE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern const char True;
extern const char False;
extern const int diskSize;
extern const int contentSize;

typedef char boolean;

typedef struct IB {
    int nextIB;
    int contentBlockID;
} IB;

typedef struct FCB {
    int dadFCB;
    int nextFCB;
    int subFCB;
    char *filename;
    int strlen;
    boolean flagFolder;
    time_t create;
    time_t lastChange;
    int size;
    int nextIB;
} FCB;

#define fcbNum 448
#define ibNum 4032

typedef struct sysStatus {
    char *disk;
    FCB fcbs[fcbNum];
    IB ibs[ibNum];
    int free_fcb;
    int free_ib;
    int full_fcb;
    int pwd;
    boolean opened;
} sysStatus;

void freeFCB(sysStatus *pstatus, int x);

void initFCB(sysStatus *pstatus, int x, boolean flagFolder);

int checkExist(sysStatus *pstatus, char *name);

void writeFCB(sysStatus *pstatus, int fcbid);

void readFCB(sysStatus *pstatus, int fcbid, FCB *dest);

void printFCB(FCB buf);

void writeIBp(sysStatus *pstatus, int id, int next);

void freeIBp(sysStatus *pstatus, int id);

void clearIB(sysStatus *pstatus, int id);

int newIB(sysStatus *pstatus);

void readIB(sysStatus *pstatus, int id);

void readStatus(sysStatus *pstatus);

#endif
