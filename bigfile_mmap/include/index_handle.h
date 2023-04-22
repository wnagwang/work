#ifndef INDEX_HANDLE_H
#define INDEX_HANDLE_H

#include "common.h"
#include "mmap_file_op.h"

namespace ww
{
    namespace file
    {
        struct IndexHeader
        {
            IndexHeader()
            {
                memset(this, 0, sizeof(IndexHeader));
            }
            BlockInfo block_info_;     // 块信息
            int32_t bucket_size_;      // 哈希桶大小
            int32_t data_file_offset_; // 数据存储在主块位置
            int32_t index_file_size_;  // 文件最后位置
            int32_t free_head_offset_; // 可重用节点
        };

        class IndexHandle
        {
        public:
            IndexHandle(const std::string &base_path, const uint32_t block_id);
            ~IndexHandle();
            //创建索引文件
            int create(const uint32_t logic_block_id, const int32_t bucket_size, const MMapOption &mmapOption);//创建文件表
            IndexHeader *index_hander()
            {
                return reinterpret_cast<IndexHeader *>(file_op_->get_data());
            }
            //加载(检查)索引文件
            int load(const uint32_t logic_block_id, const int32_t bucket_size, const MMapOption map_option);//检查文件是否成功
            BlockInfo *block_info()
            {
                return reinterpret_cast<BlockInfo *>(file_op_->get_data());
            }
            //解除映射
            int remove(const uint32_t logic_block_id);
            int flush();
            int32_t *bucket_slot()
            {
                return reinterpret_cast<int32_t *>(reinterpret_cast<char *>(file_op_->get_data()) + sizeof(IndexHeader));
            }
            int32_t get_offset();
            int32_t get_free_offset()
            {
                return index_hander()->free_head_offset_;
            }
            //更新数据
            int updata_block_info(const OperType opertype, const uint32_t modify_size);
            void commit_block_data_offset(const int file_size)
            {
                reinterpret_cast<IndexHeader *>(file_op_->get_data())->data_file_offset_ += file_size;
            }
            //删除meteinfo
            int32_t delete_segment_meta(const uint64_t key);
            int hash_find(const uint64_t key, int32_t &current_offset, int32_t &previous_offset);
            int hash_insert(const uint64_t key, int32_t &previous_offset, MetaInfo &meta);
            //写入meteinfo
            int write_segment_meta(const uint64_t key, MetaInfo &meta);
            //读出meteinfo
            int read_segment_meta(const uint64_t key, MetaInfo &meta);

        private:
            mmap_file_operation *file_op_; //文件操作
            bool is_load_; // 判断索引文件是否加载
        };

    }
}

#endif