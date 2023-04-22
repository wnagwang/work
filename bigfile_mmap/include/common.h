#ifndef COMMON_H
#define COMMON_H
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sstream>
#include <iostream>

#define FUN_SUCCEED 0
#define FUN_EXIT -1

namespace ww
{
    namespace file
    {

        static const std::string MAINBLOCK="/mainblock/";
        static const std::string INDEX="/index/";
        static const mode_t DIR_MODE=0755;

        struct BlockInfo
        {
            uint32_t block_id_;      // 块编号
            int32_t version_;        // 块版本号
            int32_t file_count_;     // 已经保存文件总数
            int32_t size_;           // 已保存数据大小
            int32_t del_file_count_; // 删除文件数量
            int32_t del_size_;       // 删除文件大小
            uint32_t seq_no_;        // 下一个可分配文件编号

            BlockInfo()
            {
                memset(this, 0, sizeof(BlockInfo));
            }
            inline bool operator==(const BlockInfo &rhs) const
            {
                return block_id_ == rhs.block_id_ && version_ == rhs.version_ &&
                       file_count_ == rhs.file_count_ && size_ == rhs.size_ &&
                        del_file_count_ == rhs.del_file_count_ && del_size_ == rhs.del_size_ 
                        && seq_no_ == rhs.seq_no_;
            }
        };

        enum OperType //更新操作
        {
            C_OPER_INSERT = 1, // 插入
            C_OPER_DELETE      // 删除   
        };

        struct MetaInfo
        {
            
            struct 
            {
                int32_t inner_offset_;//文件在内部的偏移量
                int32_t size_; // 文件大小
            }location_;
            int32_t next_meta_offset_;//下一个节点在索引文件的偏移量
            uint64_t fileid_;   //文件id
            MetaInfo()
            {
                memset(this,0,sizeof(MetaInfo));
            }
            MetaInfo(const uint64_t file_id,const int32_t in_offset,const int32_t file_size,const int32_t next_meta_offset)
            {
                fileid_=file_id;
                location_.inner_offset_=in_offset;
                location_.size_=file_size;
                next_meta_offset_=next_meta_offset;
            }

            MetaInfo(const MetaInfo& meta)
            {
                memcpy(this,&meta,sizeof(MetaInfo));
            }

            MetaInfo& operator=(const MetaInfo&meta)
            {
                if(this==&meta) return *this;
                fileid_=meta.fileid_;
                next_meta_offset_=meta.next_meta_offset_;
                location_.inner_offset_=meta.location_.inner_offset_;
                location_.size_=meta.location_.size_;
                return *this;
            }
            MetaInfo &clone(const MetaInfo&meta)
            {
                assert(this!=&meta);
                fileid_=meta.fileid_;
                next_meta_offset_=meta.next_meta_offset_;
                location_.inner_offset_=meta.location_.inner_offset_;
                location_.size_=meta.location_.size_;
                return *this;
            }
            
            bool operator ==(const MetaInfo&meta) const
            {
                return fileid_==meta.fileid_&&
                next_meta_offset_==meta.next_meta_offset_&&
                location_.inner_offset_==meta.location_.inner_offset_&&
                location_.size_==meta.location_.size_;
            }
             bool operator !=(const MetaInfo&meta) const
            {
                return fileid_!=meta.fileid_&&
                next_meta_offset_!=meta.next_meta_offset_&&
                location_.inner_offset_!=meta.location_.inner_offset_&&
                location_.size_!=meta.location_.size_;
            }
            int32_t get_size()const 
            {
                return location_.size_;
            }
            void set_size(int32_t size)
            {
                location_.size_=size;
            }
            int32_t get_offset()
            {
                return location_.inner_offset_;
            }

            void set_offset(int32_t offset)
            {
                location_.inner_offset_=offset;
            }
            uint64_t get_key()const
            {
                return fileid_;
            }
            void set_key(uint64_t key)
            {
                fileid_=key;
            }
            uint64_t get_id()const
            {
                return fileid_;
            }

            void set_id(uint64_t id)
            {
                fileid_=id;
            }

            int32_t get_next_meta_offset()const 
            {
                return next_meta_offset_;
            }
            void set_next_meta_offset(int32_t next_meta_offset)
            {
                next_meta_offset_=next_meta_offset;
            }
            
            
        };
    }
}

#endif