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
		struct inode_t *inode = (struct inode_t *) malloc(sizeof(struct inode_t));
		ssufs_readInode(inum, inode);
		//printf("inum:%d\n",inum);

		for(int i=0;i<MAX_FILE_SIZE;i++){
				if(inode->direct_blocks[i]==DATA_BLOCK_USED){
						printf("here\n");
						ssufs_freeDataBlock(inode->direct_blocks[i]);
				}
		}
		free(inode);
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
		int iter=nbytes/64+1;
		if(nbytes%64==0)
				iter-=1;
		char total[257];
		for(int i=0;i<257;i++){
				total[i]='\0';
		}
		//해당 inode불러옴
		struct inode_t *inode=(struct inode_t *) malloc(sizeof(struct inode_t));
		ssufs_readInode(file_handle_array[file_handle].inode_number,inode);

		//inode에 저장되어 있는 direct_blocks에 저장되어 있는 값을 통해 읽어들임
		for(int i=0;i<iter;i++){
				if(inode->direct_blocks[i]==-1)
						break;
				char seg[65];
				for(int j=0;j<65;j++){
						seg[j]='\0';
				}
				ssufs_readDataBlock(inode->direct_blocks[i],seg);
				for(int j=0;j<64;j++){
						if(seg[j]=='\0')
								break;
						total[64*i+j]=seg[j];
				}


		}
		memcpy(buf,total,nbytes);
		file_handle_array[file_handle].offset+=nbytes;
		free(inode);
		printf("buf 내용:%s\n",buf);
		return 0;

}
int ssufs_write(int file_handle, char *buf, int nbytes){
		//몇개의 블럭을 사용해야하는지 계산
		int iter=nbytes/64+1;
		if(nbytes%64==0)
				iter-=1;
		char total[257];
		for(int i=0;i<257;i++){
				total[i]='\0';
		}
		for(int i=0;i<strlen(buf);i++){
				total[i]=buf[i];
		}
		//해당 inode불러옴
		struct inode_t *inode=(struct inode_t *) malloc(sizeof(struct inode_t));
		ssufs_readInode(file_handle_array[file_handle].inode_number,inode);

		//한블록보다 크면 토큰 나눔
		for(int i=0;i<iter;i++){
				int blockNum;

				//사용가능한 블록있는지 확인
				if((blockNum=ssufs_allocDataBlock())<0){
						printf("alloc DataBlock failed\n");
						ssufs_freeDataBlock(blockNum);
						return -1;
				}

				//문자열을 segment로 자름
				char seg[65];
				for(int j=0;j<65;j++){
						seg[j]='\0';
				}
				for(int j=0;j<64;j++){
						if(total[64*i+j]=='\0')
								break;
						seg[j]=total[64*i+j];
				}

				ssufs_writeDataBlock(blockNum,seg);
				inode->direct_blocks[i]=blockNum;

		}

		//성공하면 한번만
		inode->file_size+=nbytes;
		ssufs_writeInode(file_handle_array[file_handle].inode_number,inode);
		//handle의 offset 값 갱신
		file_handle_array[file_handle].offset+=nbytes;
		free(inode);
		return 0;



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
