#ifndef FILE_OP_H
#define FILE_OP_H

#include "common.h"

namespace ww{

    namespace file{
        class file_operation{
            public:
            file_operation(const std::string& file_name,const int open_flags = O_RDWR|O_LARGEFILE);
            ~file_operation();

            int open_file(); //打开文件
            void close_file(); //关闭文件

            int flush_file();//写入文件
            
            int unlink_file();//删除文件

            int pread_file(char* buf,const int32_t nbyte,const int64_t offset); //读文件

            int pwrite_file(const char* buf, const int32_t nbyte,const int64_t offset); //写入文件

            int write_file(const char *buf ,const int32_t nbyte);
            
            int64_t get_size(); //获取文件大小

            int ftruncate_file(const int64_t length);//修改文件大小

            int seek_file(const int64_t offset);//移动文件读写指针

            int get_fd()const;

            protected:
            static const mode_t OPEN_MOOD = 0644; //默认权限
            static const int MAX_DISK_TIMES = 5;  //最大读取磁盘次数

            
            protected:
            int fd_;
            int open_flag_;
            char* file_name_;
        };
    }
}



#endif