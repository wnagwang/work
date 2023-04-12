#include "file_op.h"

int main(){
    const char*filename = "file_test.txt";
    ww::file::file_operation *file_op = new ww::file::file_operation(filename,O_CREAT|O_RDWR|O_LARGEFILE);
    int fd = file_op->open_file();
    if(fd<0)
    {
        fprintf(stderr,"%s",strerror(-fd));
        exit(-1);
    }

    char buf[64];
    memset(buf,'6',sizeof(buf));
    int ret = file_op->pwrite_file(buf,64,1024);
    
    memset(buf,0,sizeof(buf));
    ret=file_op->pread_file(buf,63,1024);
   
    buf[63]='\0';
    printf("buf :%s\n",buf);
    file_op->write_file(buf,64);
    file_op->close_file();
    //file_op->unlink_file();
    return 0;
}