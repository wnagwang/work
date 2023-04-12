#ifndef MMAP_FILE_H_
#define MMAP_FILE_H_

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "common.h"

namespace ww{
    
    namespace file{
        struct MMapOption
        {
            int32_t max_mmap_size; //文件最大大小
            int32_t per_mmap_size; //一次追加大小
            int32_t fir_mmap_size; //第一次分配大小
        };
        
        class MMapfile{
        public:
            MMapfile();
            explicit MMapfile(const int fd);
            ~MMapfile();
            MMapfile(const MMapOption& mmap_option,const int fd);
            
            bool sync_file();//同步文件
            bool munmap_file();//解除映射
            bool remmap_file();//重新映射
            void* get_data() const; //得到首地址
            int32_t get_size() const;//获取文件大小
            bool mmap_file(const bool write); //映射文件
        private:
            bool ensure_file_size(const int32_t size); //扩容
            int32_t mmap_size_; 
            void* data_;
            int fd_;
            struct MMapOption mmap_option_;
        };
    }
}


#endif