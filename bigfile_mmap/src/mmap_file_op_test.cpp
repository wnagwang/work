#include "mmap_file_op.h"

const static ww::file::MMapOption mmapoption = {102400,1024,1024};
int main(){
    const char*file_name = "mmap_file_op.txt";
    ww::file::mmap_file_operation *mmap_file_op = new ww::file::mmap_file_operation(file_name);
    int fd = mmap_file_op->open_file();
    if(fd < 0){
        fprintf(stderr,"file open error %s\n",strerror(-fd));
        return -1;
    }
    char buf[128];
    mmap_file_op ->mmap_file(mmapoption);
    
    memset(buf,'1',sizeof buf);
    int ret = mmap_file_op -> pwrite_file(buf,127,20);

    if(ret == FUN_EXIT){
        fprintf(stderr,"pwrite error\n");
        exit(-1);
    }
    mmap_file_op->flush_file(); 
    memset(buf,0,sizeof buf);
    ret = mmap_file_op->pread_file(buf,126,20);
    buf[127]='\0';
    printf("buf : %s\n",buf);
    mmap_file_op->munmap_file();
    mmap_file_op->close_file();
}