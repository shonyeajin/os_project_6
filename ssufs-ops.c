#include "ssufs-ops.h"

extern struct filehandle_t file_handle_array[MAX_OPEN_FILES];

int ssufs_allocFileHandle() {
	for(int i = 0; i < MAX_OPEN_FILES; i++) {
		if (file_handle_array[i].inode_number == -1) {
			return i;
		}
	}
	return -1;
}

int ssufs_create(char *filename){
	/* 1 */
		//free한 inode 있는지 확인
		int inoNum=ssufs_allocInode();
		int fd;
		struct inode_t *inode;

		if(inoNum==-1)
				return -1;
		if((fd=open(filename, O_CREAT|O_EXCL,0777))<0){
				printf("file create failed\n");
				return -1;
		}
		close(fd);
		ssufs_readInode(inoNum,inode);
		memcpy(inode->name,filename,MAX_NAME_STRLEN);
		inode->status=INODE_IN_USE;
		//inode->file_size 어떻게 조정해야할지 아직 모르겠음
		ssufs_writeInode(inoNum,inode);

		return inoNum;


}
void ssufs_delete(char *filename){
	/* 2 */
		//지울때 해당하는 핸들러 정보 먼저 지움
		//해당 이름의 file inode 찾음
		int inum;
		int handleIdx;
		if((inum=open_namei(filename))<0){
				printf("open_namei failed\n");
		}
		ssufs_freeInode(inum);
		/*
		//해당 inode 번호에 해당하는 handler array에서 index찾음
		//핸들러 정보 지워주는거 구현하려면 하기 근데 일단 보류
		for(int i<0;i<MAX_OPEN_FILES;i++){
				if(file_handle_array[i].inode_number==inum){
						handleIdx=i;
				}
		}
		ssufs_close();
		*/

		if(remove(filename)<0){
				printf("remove failed\n");
		}
		
}

int ssufs_open(char *filename){
	/* 3 */
		int fd;
		int inum;
		int handleIdx;
		if((fd=open(filename,O_RDWR))<0){
				printf("file open failed2\n");
				return -1;
		}
		if((inum=open_namei(filename))<0){
				printf("open_namei failed\n");
				return -1;
		}
		if((handleIdx=ssufs_allocFileHandle())==-1){
				printf("file handle alloc failed\n");
				return -1;
		}
		file_handle_array[handleIdx].inode_number=inum;
		return handleIdx;
}

void ssufs_close(int file_handle){
	file_handle_array[file_handle].inode_number = -1;
	file_handle_array[file_handle].offset = 0;
}

int ssufs_read(int file_handle, char *buf, int nbytes){
	/* 4 */
		//인자로 받은 file_handle(=fd)에서 해당 byte만큼을 읽어야함
		//우선
}

int ssufs_write(int file_handle, char *buf, int nbytes){
	/* 5 */
		//해당 handle array에서 inode_number알아냄
		//alloc data block 해서 할당받음
		//블록번호(0~30)에 buf내용 씀,BLOCk_SIZE만큼


		if(nbytes>256){
				printf("file size too mush big\n");
				return -1;
		}
		int blockNum;

		if((blockNum=ssufs_allocDataBlock())<0){
				printf("alloc Datablock failed\n");
				ssufs_freeDataBlock(blockNum);
				return -1;
		}
		ssufs_writeDataBlock(blockNum,buf);
/*
		//해당 fd의 inode 불러옴..->direct_block값 변경하기 위해
		int inoNum=file_handle_array[file_handle].inode_number;
		struct inode_t *inode;
		ssufs_readInode(inoNum,inode);
		//inode->direct_blocks;

*/







}

int ssufs_lseek(int file_handle, int nseek){
	int offset = file_handle_array[file_handle].offset;

	struct inode_t *tmp = (struct inode_t *) malloc(sizeof(struct inode_t));
	ssufs_readInode(file_handle_array[file_handle].inode_number, tmp);
	
	int fsize = tmp->file_size;
	
	offset += nseek;

	if ((fsize == -1) || (offset < 0) || (offset > fsize)) {
		free(tmp);
		return -1;
	}

	file_handle_array[file_handle].offset = offset;
	free(tmp);

	return 0;
}
