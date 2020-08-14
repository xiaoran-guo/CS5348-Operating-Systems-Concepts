/*
We have constructed the filesystem structure and necessary logical operations that consititute the required functionality. There are some issues which we could not figure out where and what they are.
 
To run it, load it into the server, type in " cc fsaccess.c " command and execute, followed by
" ./a.out* PATHNAME BLOCKNUMBER INODENUMBER " and execute, and then type in various command and let the program operate accordingly, eg " initfs PATHNAME BLOCKNUMBER INODENUMBER " for initializing the filesystem or " q " for quiting.
 
We have tried our best effort for this part. Regarding the remaining issues, we will try our best to tackle them and submit the final version with part 2 results. 
 
 
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <zconf.h>
#include <string.h>
#include <fcntl.h>

struct super_block {
    unsigned int isize;
    unsigned int fsize;
    unsigned int nfree;
    unsigned int free[100];
    unsigned int ninode;
    unsigned int inode[100];
    char flock;
    char ilock;
    char fmod;
    unsigned int time_t;
    char unused[201];
};

struct i_node {
    unsigned short flags;
    char nlinks;
    char uid;
    char gid;
    unsigned int size;
    unsigned int addr[8];
    unsigned int actime[2];
    unsigned int modtime[2];
    char unused[15];

};

struct directory_entry_struct{
    unsigned short id_inode;
    char file_name[14];
};

#define sizeofblk 1024
#define sizeofinode 64

int fd = -1;
struct super_block super_block = {0, 0, 0, {0}, 0, {0}, 0, 0, 0, 0, {0}};


int savesuper_block() {
    lseek(fd, sizeofblk, SEEK_SET);
    write(fd, &super_block, sizeof(struct super_block));
    return 0;
}

int writeblk(unsigned int blkid) {
    if(blkid >= super_block.fsize) {
        return -1;
    }

    if(super_block.nfree == 100) {
        int offset = blkid * sizeofblk;

        lseek(fd, offset, SEEK_SET);
        write(fd, &super_block.nfree, 4);
        offset += 4;

        lseek(fd, offset, SEEK_SET);
        write(fd, super_block.free, 4 * super_block.nfree);

        printf("Block#%d: nfree=%u, free[1]=%d,free[99]=%d\n", blkid,super_block.nfree,super_block.free[1],super_block.free[99]);

        super_block.free[0] = blkid;
        super_block.nfree = 1;
    }
    else {
        super_block.free[super_block.nfree] = blkid;
        if (blkid == super_block.fsize - 1) {
            printf("Block#%d: nfree=%u, free[1]=%d,free[%d]=%d\n", blkid,super_block.nfree,super_block.free[1],super_block.nfree,super_block.free[super_block.nfree]);
        }
        super_block.nfree++;
    }



    savesuper_block();

    return 0;
}

int setfreeblocks(unsigned int startpos, unsigned int endpos) {
    super_block.free[0] = 0;
    super_block.nfree = 1;

    savesuper_block();

    unsigned int pointer;

    for(pointer = startpos; pointer <= endpos; pointer++) {
        writeblk(pointer);
    }

    return 0;
}


unsigned int readblk() {
    super_block.nfree--;

    unsigned int blkid;

    if(super_block.nfree == 0) {
        blkid = super_block.free[0];

        int offset = blkid * sizeofblk;

        lseek(fd, offset, SEEK_SET);
        read(fd, &super_block.nfree, 4);
        offset += 4;

        lseek(fd, offset, SEEK_SET);
        read(fd, &super_block.free, 4 * super_block.nfree);
    }
    else {
        blkid = super_block.free[super_block.nfree];
    }

    savesuper_block();

    char empty[sizeofblk] = {0};

    lseek(fd, blkid * sizeofblk, SEEK_SET);
    write(fd, empty, sizeofblk);
    return blkid;
}

int setrootdir() {
    int offset = 2 * sizeofblk;
    struct directory_entry_struct directory_entry;
    struct i_node inode1 = {0,0,0,0,0,{0},(unsigned int)time(NULL),(unsigned int)time(NULL),{0}};

    inode1.flags |=1 <<15;
    inode1.flags |=1 <<14;
    inode1.flags |=0 <<13;



    directory_entry.id_inode = 1;
    strcpy(directory_entry.file_name, ".");
    unsigned int dir_file_block = readblk();
    lseek(fd,dir_file_block*sizeofblk,SEEK_SET);
    write(fd,&directory_entry,16);
    strcpy(directory_entry.file_name, "..");
    lseek(fd,dir_file_block*sizeofblk + 16 ,SEEK_SET);
    write(fd,&directory_entry,16);
    inode1.addr[0]=dir_file_block;


    lseek(fd, offset, SEEK_SET);
    write(fd, &inode1, sizeofinode);
    return 0;
}

int fillinodes( unsigned int total_inodes) {

    unsigned int iNum;
    int maxNum = total_inodes;

    for (iNum = 2; iNum <= maxNum; iNum++) 
    {
        
        super_block.inode[super_block.ninode] = iNum;  // write
        super_block.ninode++;

        if (super_block.ninode == 100) 
        {
            break;
        }
    }
}

int initfs(char* path, unsigned int total_blocks,unsigned int total_inodes) {
    
    printf("initializing filesystem...\n");
    
    printf("\n");
    
    printf("datablock size = %d bytes\n", sizeofblk);
    
    printf("\n");
    
    printf("inode size = %d bytes\n", sizeofinode);
    
    printf("\n");
    
    fd = open(path, O_RDWR | O_TRUNC | O_CREAT);
    if(fd == -1) {
        printf("no loaded file\n");
        return -1;
    }

    unsigned int numOfBlock = total_blocks;
    unsigned int numOfinode = total_inodes;

    if(numOfBlock == 0 || numOfinode == 0 || numOfinode > numOfBlock * 16) {
        printf("incorrect parameters\n");
        return -1;
    }


    char emptyBlock[sizeofblk] = {0};
    int i;
    for (i = 0; i < numOfBlock; i++) {
        lseek(fd, i * sizeofblk, SEEK_SET);
        write(fd, emptyBlock, sizeofblk);
    }

    super_block.isize = numOfinode / 16;
    super_block.isize += (super_block.isize * 16) >= numOfinode ? 0 : 1;
    super_block.fsize = numOfBlock;
    super_block.nfree = 0;
    super_block.ninode = 0;
    super_block.flock = 0;
    super_block.ilock = 0;
    super_block.fmod = 0;
    super_block.time_t = (unsigned int)time(NULL);

    savesuper_block();

    setfreeblocks(super_block.isize + 2, super_block.fsize - 1);

    setrootdir();

    fillinodes(total_inodes);

    return 1;
}


int welcome(){

    printf("\n");
    printf("\n");
    printf("\n");
    printf("                                                                  :#@@@@#\n");
    printf("            --                             ~==:                  :#@@@@@@@=\n");
    printf("           #@@@!                          !@@@@!               ~#@@@@@@@@@@~\n");
    printf("          ~@@@@@*                         #@@@@#              !@@@@@@@@@@@@~\n");
    printf("          ;@@@@@@~                       ;@@@@@#             #@@@@@@@@@@@@!\n");
    printf("          ,@@@@@@@,                      $@@@@@=           :#@@@@@@!-,~@#-\n");
    printf("           ~@@@@@@$                     .@@@@@@,          !@@@@@@@~\n");
    printf("            ,#@@@@@*                    $@@@@@!          -@@@@@@=\n");
    printf("              @@@@@@;                  ,@@@@@=          ;@@@@@@*\n");
    printf("              :@@@@@@                  :@@@@@.         ,@@@@@@=\n");
    printf("               *@@@@@:                ,@@@@@$          *@@@@@~\n");
    printf("               .@@@@@@                *@@@@@,         .@@@@@;\n");
    printf("                -@@@@@;              .@@@@@:          =@@@@:\n");
    printf("                 :@@@@@-             #@@@@$          :@@@@@.           .#@@@@@@@:\n");
    printf("                  #@@@@=           .#@@@@$.          #@@@@-         ~$@@@@@@@@@@@$~\n");
    printf("                  ~@@@@@,          *@@@@#.          :@@@@~       ,*@@@@@@@@@@@@@@@@~\n");
    printf("                   !@@@@$         :@@@@#.           @@@@@.     -#@@@@@@@@@@@##@@@@@@,\n");
    printf("                   .@@@@@.       .@@@@@;           -@@@@*    ;@@@@@@@@@#-      =@@@@:\n");
    printf("                    -@@@@*       $@@@@$            :@@@@!  .$@@@@@@@@*-         @@@@#\n");
    printf("                     #@@@@,     ;@@@@#.            $@@@@. ~#@@@@@@=:.          ;@@@@=\n");
    printf("                     :@@@@:    ;@@@@@-            :@@@@$ ;@@@@@@$;            :@@@@@\n");
    printf("                      @@@@#   -@@@@@;             :@@@@:-@@@@@@:            .@@@@@@;\n");
    printf("                      #@@@@- .#@@@@*              ,@@@@ $@@@@@-            ;@@@@@@!\n");
    printf("                      :@@@@=:#@@@@*               -@@@@;@@@@$,           :#@@@@@@$,\n");
    printf("                       @@@@@@@@@@$.               .@@@@@@@@@.        .~!#@@@@@@=~\n");
    printf("                       ;@@@@@@@@@.                 #@@@@@@@$      .:=@@@@@@@@#;\n");
    printf("                       .@@@@@@@@~                  ;@@@@@@@-  .:$@@@@@@@@@@@;\n");
    printf("                        ,@@@@@@:                   ,@@@@@@@@@#@@@@@@@@@@@#~\n");
    printf("                         $@@@@@                     ,@@@@@@@@@@@@@@@@@#*~\n");
    printf("                         ,@@@@@                      -=@@@@@@@@@@@@$*;\n");
    printf("                          !@@@@                        ,@@@@@@@$~\n");
    printf("                           @@@-                          ,!* ”)\n");

    printf("\n");
    printf("\n");
    printf("\n");
    printf("\n");
    int i = 0;
    for (i = 0; i < 3; i++) {
        sleep(1);
        printf("\r%s", "                                      ■■");
        fflush(stdout);
        sleep(1);
        printf("%s", "■■");
        fflush(stdout);
        sleep(1);
        printf("%s", "■■");
        fflush(stdout);
        sleep(1);
        printf("%s", "■■");
        fflush(stdout);
        sleep(1);
        printf("%s", "■■");
        fflush(stdout);
        sleep(1);
        printf("%s", "■■");
        fflush(stdout);
        sleep(1);
        printf("%c[2K", 27);
    }
    return 0;
}

int main(int args, char* argv[]) {
    char *parser, cmd[256];
    char *file_path;
    char *num1, *num2;
    unsigned int bno =0, inode_no=0;
    unsigned short num_bytes;
    welcome();
    printf("\n\nInitialize file system by using initfs <<name of your v6filesystem>> << total blocks>> << total inodes>>\n");
    while(1)
    {
        printf("\nEnter command\n");
        scanf(" %[^\n]s", cmd);
        parser = strtok(cmd," ");
        if(strcmp(parser, "initfs")==0)
        {
            file_path = strtok(NULL, " ");
            num1 = strtok(NULL, " ");
            num2 = strtok(NULL, " ");
            if(access(file_path,X_OK) != -1)
            {
                if( fd = open(file_path, O_RDWR) == -1)
                {
                    printf("File system exists, open failed");
                    return 1;
                }
                access(file_path, R_OK |W_OK | X_OK);
                printf("Filesystem already exists.\n");
                printf("Same file system to be used\n");
            }
            else
            {
                if (!num1 || !num2)
                    printf("Enter all arguments in form initfs v6filesystem 5000 400.\n");
                else
                {
                    bno = atoi(num1);
                    inode_no = atoi(num2);
                    if(initfs(file_path,bno, inode_no))
                    {
                        printf("File System has been Initialized \n");
                    }
                    else
                    {
                        printf("Error: File system initialization error.\n");
                    }
                }
            }
            parser = NULL;
            printf("\n");
        }
        else if(strcmp(parser, "q")==0)
        {
            printf("\nEXITING FILE SYSTEM NOW....\n");
            lseek(fd,sizeofblk,0);
            if((num_bytes =write(fd,&super_block,sizeofblk)) < sizeofblk)
            {
                printf("\nerror in writing the super block\n");
            }
            return 0;
        }
        else
        {
            printf("\nInvalid command\n ");
            printf("\n");
        }
    }
}
