#include <stdio.h>
#include "flash.h"

extern void flash_read(long offset, unsigned char *buff,int n) {
	FILE *fp ;
	int i ;
	if ( ( fp = fopen("flash.hex","rb") )==NULL ) {
		printf( "can't open file for read!\n");
		return ;
	}
	fseek(fp, offset,SEEK_SET);
	for ( i=0;i<n;i++){
		if ( (*(buff+i)=getc(fp))==EOF) break ;
	}
	fclose(fp);
	return ;
}

extern void flash_write(long offset, unsigned char *buff,int n) {
	FILE *fp ;
	int i ;
	if ( ( fp = fopen("flash.hex","rb+") )==NULL ) {
		printf( "can't open file for write!\n");
		return ;
	}
    printf ("Writing offset:0x%x\n", offset);
	fseek(fp, offset,SEEK_SET);
	fwrite(buff,n,1,fp);
	fclose(fp);
}
