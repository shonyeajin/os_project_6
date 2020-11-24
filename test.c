#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

int main(){
		char str[]="aaaaabbbbbccccc";
		//문자열 길이 알 수 있음
		//printf("%ld",strlen(str));
		char seg[6];

		for(int i=0;i<5;i++){
				seg[i]=str[i];
		}
		seg[5]='\0';
		for(int i=0;i<5;i++){
				seg[i]=str[5+i];
		}

		printf("%s\n",seg);
}
int func(){
		//몇개의 블럭을 사용해야하는지 계산
		int iter=nbytes/64;
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
		free(inode);
		return 0;

}
