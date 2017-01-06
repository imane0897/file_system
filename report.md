# 文件系统

## 概要设计

1. 创建、打开、退出文件系统
2. 创建和删除子目录
3. 显示当前目录的内容
4. 更改当前目录
5. 创建、打开、关闭、删除文件
6. 读写文件

## 主要数据结构

### 索引节点

```c
typedef struct IB {
    int nextIB; 		// 下一个索引节点的指针
    int contentBlockID; // 指向该节点的内容块的指针
} IB;
```



### 文件控制块

```c
typedef struct FCB {
    int dadFCB;			// 父节点，指向上级目录
    int nextFCB; 		// 下一节点，指向同级目录或文件
    int subFCB;			//	子节点，指向子目录
    char *filename;		// 文件名
    int strlen;			// 文件名长度
    boolean flagFolder;	// 标记是目录还是文件
    time_t create;		// 创建时间
    time_t lastChange;	// 最后一次修改时间
    int size;			// 文件长度
    int nextIB;			// 下一索引节点的指针
} FCB;
```



### 文件系统

```c
typedef struct sysStatus {
    char *disk;			// 文件系统名
    FCB fcbs[fcbNum];	// FCB数组
    IB ibs[ibNum];		// IB数组
    int free_fcb;		// 指向第一个空闲FCB
    int free_ib;		// 指向第一个空闲IB
    int full_fcb;		// 指向被使用的最末FCB
    int pwd;			// 当前目录
    boolean opened;		// 标记是否打开
} sysStatus;
```



## 详细设计

|  序号  |   命令    |    说明     |
| :--: | :-----: | :-------: |
|  0   | exitfs  |  退出文件系统   |
|  1   |   new   | 新建一个文件系统  |
|  2   |   sfs   | 打开一个文件系统  |
|  3   |  mkdir  |   创建子目录   |
|  4   |  rmdir  |   删除子目录   |
|  5   |   ls    |   显示目录    |
|  6   |   cd    |   更改目录    |
|  7   | create  |   创建文件    |
|  8   |  open   |   打开文件    |
|  9   |  close  |   关闭文件    |
|  10  |  read   |    读文件    |
|  11  |  write  |    写文件    |
|  12  | delete  |   删除文件    |
|  17  | status  |  显示当前状态   |
|  18  |  sync   |   写入磁盘    |
|  19  | closefs | 关闭打开的文件系统 |



1. 用索引节点对磁盘块进行管理。

2. 创建新的文件系统时需要先关闭当前的文件系统。

3. 对目录和文件的操作需要在打开一个文件系统的条件下进行。

4. status 命令显示 fcb 的状态。

5. tree 命令显示目录的树形结构。

6. 读文件时，可以指定从第几个字符开始读，如果不指定则默认从第一个字符开始读。

   ```c
   char readChar(sysStatus *pstatus, int i) {
       int block, moveby, blockID;
       block = i / contentSize;
       moveby = i - block * contentSize;
       for (blockID = pstatus->fcbs[pstatus->pwd].nextIB; block--;
            blockID = pstatus->ibs[blockID].nextIB);
       return pstatus->disk[512 * (64 + blockID) + moveby + sizeof(int)];
   }

   void read_file(sysStatus *pstatus, char *cmdstr) {
       int i;
       int begin = 0, end = pstatus->fcbs[pstatus->pwd].size;
       sscanf(cmdstr, "%d%d", &begin, &end);
       if (end > pstatus->fcbs[pstatus->pwd].size) {
           end = pstatus->fcbs[pstatus->pwd].size;
           printf("\033[33m>>> Request out of range!\033[0m\n");
       }
       for (i = begin; i < end; i++) {
           putchar(readChar(pstatus, i));
       }
       return;
   }
   ```

   ​

7. 写文件时默认从第一个字符开始写，如果指定了起始位置则从起始位置开始覆盖写，写文件的终止条件是在新一行遇到 .[Enter]。

   写入的策略是先将需要写的字符一次性读入buffer中，再计算每一个字符串需要被写入到磁盘块的位置，将该字符填入相应的位置。

   ```c
   void writeChar(sysStatus *pstatus, int i, char ch) {
       int block, moveby, blockID;
       block = i / contentSize;
       moveby = i - block * contentSize;
       if (-1 == pstatus->fcbs[pstatus->pwd].nextIB) {
           blockID = pstatus->fcbs[pstatus->pwd].nextIB = newIB(pstatus);
           writeFCB(pstatus, pstatus->pwd);
       } else {
           for (blockID = pstatus->fcbs[pstatus->pwd].nextIB;
                block-- && pstatus->ibs[blockID].nextIB != -1;
                blockID = pstatus->ibs[blockID].nextIB)
               ;
           if (!block) {
               writeIBp(pstatus, blockID, newIB(pstatus));
               blockID = pstatus->ibs[blockID].nextIB;
           }
       }
       pstatus->disk[512 * (64 + blockID) + moveby + sizeof(int)] = ch;
       return;
   }

   void write_file(sysStatus *pstatus, char *cmdstr) {
       int begin = 0, p, i, length;
       char buf[1024];
       sscanf(cmdstr, "%d", &begin);
       if (begin > pstatus->fcbs[pstatus->pwd].size) {
           begin = pstatus->fcbs[pstatus->pwd].size;
           printf("\033[33m>>> Request out of range!\033[0m\n");
       }
       printf(">>> Writing starts from %d\n", begin);
       p = begin;
       printf("\033[32m>>> end with .<Enter> in one line\033[0m\n");
       while (True) {
           printf("<<< ");
           memset(buf, 0, sizeof(buf));
           fgets(buf, 1024, stdin);
           if(strcmp(buf, ".\n") == 0)
               break;
           length = strlen(buf);
           for (i = 0; i < length; p++, i++) {
               writeChar(pstatus, p, buf[i]);
           }
           
       }
       if (p > pstatus->fcbs[pstatus->pwd].size) {
           pstatus->fcbs[pstatus->pwd].size = p;
       }
       writeFCB(pstatus, pstatus->pwd);
       return;
   }

   ```

   ​


## 测试结果

![屏幕快照 2017-01-05 下午8.41.22](/Users/AnYameng/Desktop/屏幕快照 2017-01-05 下午8.41.22.png)

![屏幕快照 2017-01-05 下午8.41.40](/Users/AnYameng/Desktop/屏幕快照 2017-01-05 下午8.41.40.png)

![屏幕快照 2017-01-05 下午8.52.31](/Users/AnYameng/Desktop/屏幕快照 2017-01-05 下午8.52.31.png)![屏幕快照 2017-01-05 下午8.52.40](/Users/AnYameng/Desktop/屏幕快照 2017-01-05 下午8.52.40.png)

（ls 表的 file name，白色的是文件，蓝色的是目录）