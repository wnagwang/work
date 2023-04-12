#include "mmap_file.h"


const static ww::file::MMapOption mmap_option = {1024000,1024,1024};
int main(){
    int fd = open("test.txt",O_RDWR|O_CREAT|O_LARGEFILE,0644);
    if(fd < 0){
        fprintf(stderr,"文件打开失败!\n");
        return -1;
    }

    ww::file::MMapfile *mmap_file = new ww::file::MMapfile(mmap_option,fd);

    if(!mmap_file -> mmap_file(true)){
        printf("失败!\n");
    }else{
        printf("成功!\n");
        memset(mmap_file->get_data(),'8',mmap_file->get_size());
        getchar();
        mmap_file->sync_file();
        getchar();
        mmap_file->munmap_file();
    }
    close(fd);
    return 0;
}