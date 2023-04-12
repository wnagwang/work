#include "mmap_file.h"
#include "common.h"

const static int debug = 1;

namespace ww
{
    namespace file
    {
        MMapfile::MMapfile() : mmap_size_(0), data_(NULL), fd_(-1)
        {
        }

        MMapfile::MMapfile(const int fd) : mmap_size_(0), data_(NULL), fd_(fd)
        {
        }

        MMapfile::MMapfile(const MMapOption& mmap_option,const int fd) : mmap_size_(0), data_(NULL), fd_(fd)
        {
            mmap_option_.max_mmap_size = mmap_option.max_mmap_size;
            mmap_option_.per_mmap_size = mmap_option.per_mmap_size;
            mmap_option_.fir_mmap_size = mmap_option.fir_mmap_size;
        }
        MMapfile::~MMapfile()
        {
            if (data_)
            {
                msync(data_, mmap_size_, MS_SYNC);
                munmap(data_, mmap_size_);

                mmap_size_ = 0;
                data_ = NULL;
                fd_ = -1;
                mmap_option_.max_mmap_size = 0;
                mmap_option_.per_mmap_size = 0;
                mmap_option_.fir_mmap_size = 0;
            }
        }
        bool MMapfile::sync_file()
        {
            if (data_ && mmap_size_ > 0)
            {
                printf("数据同步中\n");
                if(msync(data_, mmap_size_, MS_ASYNC)!= 0)
                {    
                    fprintf(stderr,"MMapfile--sync_file error\n");
                    return false;
                }
                return true;
            }
            printf("没有映射or映射大小<=0\n");
            return false;
        }

        bool MMapfile::munmap_file()
        {
            if (munmap(data_, mmap_size_) == 0)
                return true;
            else
                fprintf(stderr,"MMapfile -- munmap_file error\n");
            return false;
        }

        bool MMapfile::remmap_file()
        {
            if (fd_ < 0 || data_ == NULL)
                return false;
            if (mmap_size_ >= mmap_option_.max_mmap_size)
            {
                fprintf(stderr, "超出最大size %d\n",mmap_option_.max_mmap_size);
                return false;
            }

            int32_t new_size = mmap_size_ + mmap_option_.per_mmap_size;
            if (new_size >= mmap_option_.max_mmap_size)
                new_size = mmap_option_.max_mmap_size;

            if (!ensure_file_size(new_size))
                return false;

            void* new_map_data = mremap(data_, mmap_size_, new_size, MREMAP_MAYMOVE);
            if (MAP_FAILED == new_map_data)
            {
                fprintf(stderr, "error:%s\n", strerror(errno));

            }

            mmap_size_ = new_size;
            data_ = new_map_data;
            return true;
            
        }

        int32_t MMapfile::get_size() const
        {
            return mmap_size_;
        }

        void* MMapfile::get_data()const
        {
            return data_;
        }
        bool MMapfile::mmap_file(const bool write)
        {
            int flags = PROT_READ;
            if (write)
            {
                flags |= PROT_WRITE;
            }

            if (fd_ < 0)
            {
                return false;
            }

            if (0 == mmap_option_.max_mmap_size)
            {
                printf("允许最大值为%d\n",mmap_option_.max_mmap_size);
                return false;
            }

            if (mmap_size_ < mmap_option_.max_mmap_size)
            {
                mmap_size_= mmap_option_.fir_mmap_size;
            }
            else
            {
                mmap_size_ = mmap_option_.max_mmap_size;
            }
            if (!ensure_file_size(mmap_size_))
                return false;

            data_ = mmap(0, mmap_size_, flags, MAP_SHARED, fd_, 0);

            if(MAP_FAILED == data_)
            {
                fprintf(stderr, "mmap error:%s\n", strerror(errno));
                fd_ = -1;
                mmap_size_ = 0;
                data_ = NULL;
                return false;
            }
            return true;
        }
        bool MMapfile::ensure_file_size(const int32_t size)
        {
            struct stat s;

            if (fstat(fd_, &s) < 0)
            {
                fprintf(stderr, "fstat error:%s\n", strerror(errno));

                return false;
            }

            if (s.st_size < size)
            {
                if (ftruncate(fd_, size) < 0)
                {
                    fprintf(stderr, " ftruncate error:%s\n", strerror(errno));
                    return false;
                }
            }

            return true;
        }
    }
}