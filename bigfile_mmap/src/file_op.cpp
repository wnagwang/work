#include "file_op.h"

namespace ww
{
    namespace file
    {
        file_operation::file_operation(const std::string &file_name, const int open_flags) : fd_(-1), open_flag_(open_flags)
        {
            file_name_ = strdup(file_name.c_str());
        }

        file_operation::~file_operation()
        {
            if (fd_ > 0)
            {
                ::close(fd_);
                fd_ = -1;
            }
            if (nullptr != file_name_)
            {
                ::free(file_name_);
                file_name_ = nullptr;
            }
        }

        int file_operation::open_file()
        {
            fd_ = open(file_name_, open_flag_, OPEN_MOOD);
            if (fd_ < 0)
                return -errno;
            return fd_;
        } // 打开文件

        void file_operation::close_file()
        {
            if (fd_ < 0)
                return;
            ::close(fd_);
            fd_ = -1;
        } // 关闭文件

        int file_operation::flush_file()
        {
            if (open_flag_ & O_SYNC)
                return 0;

            if (fd_ < 0)
            {
                open_file();
            }

            int st = -1;

            if ((st = fsync(fd_)) == 0)
            {
                printf("同步文件中\n");
            }
            else
            {
                printf("fsync error\n");
            }
            return st;
        } // 写入文件

        int file_operation::unlink_file()
        {
            close_file();
            int st = -1;
            if ((st = unlink(file_name_)) == 0)
            {
                printf("删除成功\n");
            }
            else
            {
                printf("unlink error\n");
            }
            return st;
        } // 删除文件

        int file_operation::pread_file(char *buf, const int32_t nbyte, const int64_t offset)
        {
            if (fd_ < 0)
            {
                open_file();
            }

            int32_t now = nbyte; // 剩余长度
            char *p = buf;
            int64_t start_offset = offset;
            int32_t read_length = 0;
            int i = 0; // 读取次数
            while (now > 0)
            {
                i++;
                if (i >= MAX_DISK_TIMES)
                {
                    break;
                }
                read_length = pread64(fd_, p, now, start_offset);
                if (read_length < 0)
                {
                    read_length = errno;
                    if (read_length == EINTR || read_length == EAGAIN)
                    {
                        continue;
                    }
                    else if (EBADF == read_length)
                    {
                        fd_ = -1;
                        fprintf(stderr,"pread64 error%s\n",strerror(read_length));
                        return -read_length;
                    }
                    else
                    {
                        return -read_length;
                    }
                }
                else if (read_length == 0)
                {
                    break;
                }
                now -= read_length;
                start_offset += read_length;
                p += read_length;
            }
            if (now != 0)
            {
                fprintf(stderr, "读取长度太大,超过读取磁盘次数\n");
                return -1;
            }
            return 0;
        } // 读文件

        int file_operation::pwrite_file(const char *buf, const int32_t nbyte, const int64_t offset)
        {
            if (fd_ < 0)
            {
                open_file();
            }

            int32_t now = nbyte; // 剩余长度
            const char *p = buf;
            int64_t start_offset = offset;
            int32_t write_length = 0;
            int i = 0; // 读取次数
            while (now > 0)
            {
                i++;
                if (i >= MAX_DISK_TIMES)
                {
                    break;
                }
                write_length = pwrite64(fd_, p, now, start_offset);
                if (write_length < 0)
                {
                    write_length = errno;
                    if (write_length == EINTR || write_length == EAGAIN)
                    {
                        continue;
                    }
                    else if (EBADF == write_length)
                    {
                        fd_ = -1;
                        fprintf(stderr,"pwrite64 error%s\n",strerror(write_length));
                        return -write_length;
                    }
                    else
                    {
                        return -write_length;
                    }
                }
                else if (write_length == 0)
                {
                    break;
                }
                now -= write_length;
                start_offset += write_length;
                p += write_length;
            }
            if (now != 0)
            {
                fprintf(stderr, "写入长度太大,超过读取磁盘次数\n");
                return -1;
            }
            return 0;
        } // 写入文件

        int file_operation::write_file(const char *buf, const int32_t nbyte)
        {
            if (fd_ < 0)
            {
                open_file();
            }

            int32_t now = nbyte; // 剩余长度
            const char *p = buf;
            int32_t write_length = 0;
            int i = 0; // 读取次数
            while (now > 0)
            {
                i++;
                if (i >= MAX_DISK_TIMES)
                {
                    break;
                }
                write_length = write(fd_, p, now);
                if (write_length < 0)
                {
                    write_length = errno;
                    if (write_length == EINTR || write_length == EAGAIN)
                    {
                        continue;
                    }
                    else if (EBADF == write_length)
                    {
                        fd_ = -1;
                        fprintf(stderr,"write error%s\n",strerror(write_length));
                        return -write_length;
                    }
                    else
                    {
                        return -write_length;
                    }
                }
                else if (write_length == 0)
                {
                    break;
                }
                now -= write_length;
                p += write_length;
            }
            if (now != 0)
            {
                fprintf(stderr, "写入长度太大,超过读取磁盘次数\n");
                return -1;
            }
            return 0;
        }
        int64_t file_operation::get_size()
        {
            if (fd_ < 0)
            {
                open_file();
            }
            struct stat buf;
            if (fstat(fd_, &buf) != 0)
            {
                return -1;
            }
            return buf.st_size;
        } // 获取文件大小

        int file_operation::ftruncate_file(const int64_t length)
        {
            if (fd_ < 0)
            {
                open_file();
            }
            int size = get_size();
            int st = -1;
            if ((st = ftruncate(fd_, length)) == 0)
                printf("old size %d , now size %d\n", size, (int)get_size());
            else
                printf("ftruncate error\n");
            return st;

        } // 修改文件大小

        int file_operation::seek_file(const int64_t offset)
        {

            if (fd_ < 0)
            {
                open_file();
            }
            int st = -1;

            if ((st = lseek(fd_, offset, SEEK_SET)) == 0)
            {
                printf("读写指针移动成功\n");
            }
            else
            {
                printf("lseek error\n");
            }
            return st;
        } // 移动文件读写指针

        int file_operation::get_fd() const
        {
            return fd_;
        }
    }
}