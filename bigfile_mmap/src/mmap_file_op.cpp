#include "common.h"
#include "mmap_file_op.h"

namespace ww
{
    namespace file
    {

        int mmap_file_operation::pread_file(char *buf, const int32_t nbyte, const int64_t offset)
        {
            if (is_mapped_)
            {
                if (nbyte + offset > mmap_file_->get_size())
                {
                    printf("超出文件范围,正在尝试扩容!\n");
                    if (!mmap_file_->remmap_file())
                    {
                        fprintf(stderr, "pread_file error\n");
                    }
                }
                else
                {
                    memcpy(buf, (char *)mmap_file_->get_data() + offset, nbyte);
                    return 0;
                }
                return -1;
            }
            return file_operation::pread_file(buf, nbyte, offset);
        }

        int mmap_file_operation::pwrite_file(const char *buf, const int32_t nbyte, const int64_t offset)
        {
            if (is_mapped_)
            {
                if (nbyte + offset > mmap_file_->get_size())
                {
                    printf("超出文件范围,正在尝试扩容!\n");
                    if (!mmap_file_->remmap_file())
                    {
                        fprintf(stderr, "pwrite_file error\n");
                    }
                }
                else
                {
                    memcpy(mmap_file_->get_data()+offset,buf,nbyte);
                    return 0;
                }
                return -1;
            }
            return file_operation::pwrite_file(buf,nbyte,offset);
        }
            void *mmap_file_operation::get_data() const
            {
                if (is_mapped_)
                {
                    return mmap_file_->get_data();
                }
                return nullptr;
            }
            int mmap_file_operation::flush_file()
            {
                if (is_mapped_)
                {
                    if (mmap_file_->sync_file())
                    {
                        return 0;
                    }
                    else
                    {
                        fprintf(stderr, "flush_file error\n");
                        return -1;
                    }
                }
                return file_operation::flush_file();

            } // 更新文件
            int mmap_file_operation::mmap_file(const MMapOption &mmap_option)
            {
                if (fd_ < 0)
                {
                    fprintf(stderr, "文件打开error\n");
                    return -1;
                }
                if (!is_mapped_)
                {
                    if (mmap_file_)
                        delete mmap_file_;
                    mmap_file_ = new MMapfile(mmap_option, fd_);
                    is_mapped_ = mmap_file_->mmap_file(true);
                }
                if (is_mapped_)
                    return 0;
                return -1;

            } // 映射
            int mmap_file_operation::munmap_file()
            {
                if (is_mapped_ && mmap_file_)
                {
                    delete (mmap_file_);
                    is_mapped_ = false;
                }
                return 0;
            } // 解除映射
        }
    }