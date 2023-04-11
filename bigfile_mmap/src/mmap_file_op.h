#ifndef MMAP_FILE_OP_H
#define MMAP_FILE_OP_H

#include "common.h"
#include "mmap_file.h"
#include "file_op.h"

namespace ww
{

    namespace file
    {

        class mmap_file_operation : public file_operation
        {

        public:
            mmap_file_operation(const std::string &file_name, const int open_flags = O_CREAT | O_RDWR | O_LARGEFILE)
                : file_operation(file_name, open_flags), mmap_file_(nullptr), is_mapped_(false)
            {
            }
            ~mmap_file_operation()
            {
                if (mmap_file_)
                {
                    delete (mmap_file_);
                    mmap_file_ = nullptr;
                }
            }

            int pread_file(char *buf, const int32_t nbyte, const int64_t offset);

            int pwrite_file(const char *buf, const int32_t nbyte, const int64_t offset);
            void *get_data() const;
            int flush_file(); //更新文件
            int mmap_file(const MMapOption &mmap_option); //映射
            int munmap_file();  //解除映射

        private:
            MMapfile *mmap_file_; // 文件映射操作
            bool is_mapped_;      // 判断是否映射
        };
    }

}

#endif