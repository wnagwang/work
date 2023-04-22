#include "../include/index_handle.h"

namespace ww
{
    namespace file
    {
        IndexHandle::IndexHandle(const std::string &base_path, const uint32_t block_id)
        {
            std::stringstream s;
            s << base_path << INDEX << block_id;
            std::string path;
            s >> path;

            file_op_ = new mmap_file_operation(path, O_CREAT | O_RDWR | O_LARGEFILE);
            is_load_ = false;
        }

        IndexHandle::~IndexHandle()
        {
            if (file_op_)
            {
                delete file_op_;
                file_op_ = NULL;
            }
        }

        int IndexHandle::create(const uint32_t logic_block_id, const int32_t bucket_size, const MMapOption &mmapOption)
        {
            int res;
            if (is_load_)
                return FUN_SUCCEED;
            int64_t file_size = file_op_->get_size();

            if (file_size < 0)
            {
                fprintf(stderr, "file is error\n");
                return FUN_EXIT;
            }
            else if (file_size == 0)
            {
                IndexHeader header;
                header.block_info_.block_id_ = logic_block_id;
                header.block_info_.seq_no_ = 1;
                header.bucket_size_ = bucket_size;
                header.index_file_size_ = sizeof(IndexHeader) + (sizeof(int32_t) * bucket_size);
                char *data = new char[header.index_file_size_];
                memcpy(data, &header, sizeof(IndexHeader));
                memset(data + sizeof(IndexHeader), 0, header.index_file_size_ - sizeof(IndexHeader));
                res = file_op_->pwrite_file(data, header.index_file_size_, 0);
                delete[] data;
                data = nullptr;
                if (res != FUN_SUCCEED)
                {
                    fprintf(stderr, "create---pwrite_file error\n");
                    return FUN_EXIT;
                }

                res = file_op_->flush_file();
                if (res != FUN_SUCCEED)
                {
                    fprintf(stderr, "create---flush_file error\n");
                    return FUN_EXIT;
                }
                res = file_op_->mmap_file(mmapOption);
                if (res != FUN_SUCCEED)
                {
                    fprintf(stderr, "create---mmap_file error\n");
                    return FUN_EXIT;
                }
                is_load_ = true;
                printf("create success!\n");
                printf("init block_id : %d seq_no_ : %d bucket_size : %d file_size_ : %d\n",
                       block_info()->block_id_, block_info()->seq_no_, bucket_size, file_op_->get_size());
                return FUN_SUCCEED;
            }
            else
            {
                fprintf(stderr, "文件已被创建\n");
                return FUN_EXIT;
            }
        }

        int IndexHandle::load(const uint32_t logic_block_id, const int32_t bucket_size, const MMapOption map_option)
        {
            int res;
            // if(is_load_) return FUN_EXIT;

            int64_t file_size = file_op_->get_size();
            if (file_size < 0)
            {
                fprintf(stderr, "file_size < 0 load is error\n");
                return FUN_EXIT;
            }
            else if (file_size == 0)
            {
                fprintf(stderr, "file_size = 0 load is error\n");
                return FUN_EXIT;
            }
            MMapOption now = map_option;
            if (file_size > now.fir_mmap_size && file_size <= now.max_mmap_size)
                now.fir_mmap_size = file_size;
            res = file_op_->mmap_file(now);
            if (res != FUN_SUCCEED)
            {

                fprintf(stderr, "load mmap_file is error\n");
                return FUN_EXIT;
            }
            if (0 == index_hander()->bucket_size_ || 0 == block_info()->block_id_)
            {
                printf("file error\n");
                return FUN_EXIT;
            }
            int32_t index_file_size = sizeof(IndexHeader) + index_hander()->bucket_size_ * sizeof(int32_t);
            if (file_size < index_file_size)
            {
                fprintf(stderr, "file size error\n");
                return FUN_EXIT;
            }

            if (logic_block_id != block_info()->block_id_)
            {
                fprintf(stderr, "logic_block_id:%d != block_id:%d\n", logic_block_id, block_info()->block_id_);
                return FUN_EXIT;
            }

            if (index_hander()->bucket_size_ != bucket_size)
            {
                printf("bucket_size is error file_bucket_size:%d  !=  bucket_size:%d\n", index_hander()->bucket_size_, bucket_size);
                return FUN_EXIT;
            }
            is_load_ = true;
            printf("load block info  blockid:%d version:%u file_count :%u  del_file_count:%u size :%u del_size:%u seq_no:%u \n", block_info()->block_id_, block_info()->version_, block_info()->file_count_, block_info()->del_file_count_, block_info()->size_, block_info()->del_size_, block_info()->seq_no_);
            return FUN_SUCCEED;
        }

        int IndexHandle::remove(const uint32_t logic_block_id)
        {
            if (is_load_)
            {
                if (logic_block_id != block_info()->block_id_)
                {
                    fprintf(stderr, "remove error logic_block_id :%d  block_id :%d\n", logic_block_id, block_info()->block_id_);
                    return FUN_EXIT;
                }
            }
            if (file_op_->munmap_file() != FUN_SUCCEED)
            {
                fprintf(stderr, "remove munmap_file error\n");
                return FUN_EXIT;
            }
            return file_op_->unlink_file();
        }
        int IndexHandle::flush()
        {
            if (file_op_->flush_file() != FUN_SUCCEED)
            {
                fprintf(stderr, "indexhandle flush error\n");
                return FUN_EXIT;
            }
            return FUN_SUCCEED;
        }

        int IndexHandle::write_segment_meta(const uint64_t key, MetaInfo &meta)
        {
            int32_t current_offset = 0;
            int32_t previous_offset = 0;
            // 1.key是否已经存在，并分别处理两种情况
            // 查找key是否存在hash_find(key,current_offset,previous_offset)

            int ret = hash_find(key, current_offset, previous_offset);
            // 说明找到了
            if (ret == FUN_SUCCEED)
            {
                return ret;
            }
            // 没找到就加入
            ret = hash_insert(key, previous_offset, meta);
            return ret;
        }

        int IndexHandle::hash_find(const uint64_t key, int32_t &current_offset, int32_t &previous_offset)
        {
            MetaInfo meta_info;
            current_offset = 0;
            previous_offset = 0;

            int32_t slot = static_cast<uint32_t>(key) % index_hander()->bucket_size_;
            int32_t pos = bucket_slot()[slot];
            for (; pos != 0;)
            {
                int ret = file_op_->pread_file(reinterpret_cast<char *>(&meta_info), sizeof(MetaInfo), pos);
                if (ret != FUN_SUCCEED)
                {
                    printf("hash pread error\n");
                    return ret;
                }
                if (meta_info.get_key() == key)
                {
                    current_offset = pos;
                    return FUN_SUCCEED;
                }
                previous_offset = pos;
                pos = meta_info.get_next_meta_offset();
            }

            return FUN_EXIT;
        }
        int IndexHandle::get_offset()
        {
            return index_hander()->data_file_offset_;
        }
        int IndexHandle::hash_insert(const uint64_t key, int32_t &previous_offset, MetaInfo &meta)
        {
            int32_t slot = static_cast<uint32_t>(key) % index_hander()->bucket_size_;
            MetaInfo tem;
            int32_t current_offset = 0;
            int res = FUN_SUCCEED;
            if (get_free_offset() != 0)
            {
                res = file_op_->pread_file(reinterpret_cast<char *>(&tem), sizeof(MetaInfo), get_free_offset());
                if (res != FUN_SUCCEED)
                {
                    fprintf(stderr, "hash_insert pread_file get_free_offset() error\n");
                    return res;
                }
                current_offset = get_free_offset();
                index_hander()->free_head_offset_ = tem.get_next_meta_offset();
                printf("reuse metainfo,current_offset:%d\n", current_offset);
            }
            else
            {
                current_offset = index_hander()->index_file_size_;
                index_hander()->index_file_size_ += sizeof(MetaInfo);
            }
            meta.set_next_meta_offset(0);

            res = file_op_->pwrite_file(reinterpret_cast<const char *>(&meta), sizeof(meta), current_offset);

            if (res != FUN_SUCCEED)
            {
                index_hander()->index_file_size_ -= sizeof(MetaInfo);
                fprintf(stderr, "hash_insert pwrite_file  current_offset error \n");
                return res;
            }

            if (0 != previous_offset)
            {
                res = file_op_->pread_file(reinterpret_cast<char *>(&tem), sizeof(MetaInfo), previous_offset);
                if (res != FUN_SUCCEED)
                {
                    index_hander()->index_file_size_ -= sizeof(MetaInfo);
                    fprintf(stderr, "hash_insert pread_file previous_offset error\n");
                    return res;
                }

                tem.set_next_meta_offset(current_offset);
                res = file_op_->pwrite_file(reinterpret_cast<const char *>(&tem), sizeof(MetaInfo), previous_offset);
                if (res != FUN_SUCCEED)
                {
                    index_hander()->index_file_size_ -= sizeof(MetaInfo);
                    fprintf(stderr, "hash_insert pwrite_file previous_offset error\n");
                    return res;
                }
            }
            else
            {
                bucket_slot()[slot] = current_offset;
            }
            return FUN_SUCCEED;
        }
        
        int IndexHandle::updata_block_info(const OperType opertype, const uint32_t modify_size)
        {
            if (block_info()->block_id_ == 0)
            {
                return FUN_EXIT;
            }
            if (opertype == C_OPER_INSERT)
            {
                ++block_info()->version_;
                ++block_info()->file_count_;
                ++block_info()->seq_no_;
                block_info()->size_ += modify_size;
            }
            else if (opertype == C_OPER_DELETE)
            {
                ++block_info()->version_;
                --block_info()->file_count_;
                block_info()->size_ -= modify_size;
                ++block_info()->del_file_count_;
                block_info()->del_size_ += modify_size;
            }
            printf("updata block info  blockid:%d version:%u file_count :%u  del_file_count:%u size :%u del_size:%u seq_no:%u data_offset:%u\n", block_info()->block_id_, block_info()->version_, block_info()->file_count_, block_info()->del_file_count_, block_info()->size_, block_info()->del_size_, block_info()->seq_no_, get_offset());
            return FUN_SUCCEED;
        }
        int32_t IndexHandle::delete_segment_meta(const uint64_t key)
        {
            // 初始化要删除节点的偏移值以及上一个节点的偏移值
            int32_t current_offset = 0;
            int32_t previous_offset = 0;
            // 找到要删除节点的当前offset以及上个节点的offset
            int32_t ret = hash_find(key, current_offset, previous_offset);

            if (ret != FUN_SUCCEED)
            {
                fprintf(stderr, "当前节点不存在\n");
                return ret;
            }
            MetaInfo meta;
            MetaInfo previous_meta;
            // 将要删除节点的各种信息进行读取到meta中
            ret = file_op_->pread_file(reinterpret_cast<char *>(&meta), sizeof(MetaInfo), current_offset);
            if (ret != FUN_SUCCEED)
            {
                fprintf(stderr,"delete_segment_meta pread_file current_offset errpr\n");
                return ret;
            }

            int32_t next_pos = meta.get_next_meta_offset();
            // 如果要删除的节点位于桶的“开头”
            if (previous_offset == 0)
            {
                int32_t slot = static_cast<uint32_t>(key) % index_hander()->bucket_size_;
                bucket_slot()[slot] = next_pos;
            }
            else
            {
                ret = file_op_->pread_file(reinterpret_cast<char *>(&previous_meta), sizeof(MetaInfo), previous_offset);
                if (ret != FUN_SUCCEED)
                {
                    fprintf(stderr, "delete_segment_meta pread_file previous_offset error");
                    return ret;
                }
                // 将previous_meta节点重新写回去
                previous_meta.set_next_meta_offset(next_pos);
                ret = file_op_->pwrite_file(reinterpret_cast<char *>(&previous_meta), sizeof(MetaInfo), previous_offset);
                if (ret != FUN_SUCCEED)
                {
                    fprintf(stderr, "delete_segment_meta pwrite_file previous_offset error");
                    return ret;
                }
            }
            // 可重用链表的插入，头插法
            meta.set_next_meta_offset(get_free_offset());
            ret = file_op_->pwrite_file(reinterpret_cast<char *>(&meta), sizeof(MetaInfo), current_offset);
            if (ret != FUN_SUCCEED)
            {
                fprintf(stderr, "delete_segment_meta pread_file current_offset error");
                return ret;
            }

            // 将索引的可重用节点的偏移进行更新
            index_hander()->free_head_offset_ = current_offset;
            fprintf(stdout, "delete_segment_meta delete metainfo,current_offset:%d\n", current_offset);

            // 更新块的信息
            updata_block_info(C_OPER_DELETE, meta.get_size());
            return FUN_SUCCEED;
        }

        int32_t IndexHandle::read_segment_meta(const uint64_t key,MetaInfo& meta)
        {
            int32_t current_offset = 0;
            int32_t previous_offset = 0;
            
            int32_t ret =hash_find(key,current_offset,previous_offset);
            //说明存在key
            if(ret == FUN_SUCCEED)
            {
                ret = file_op_->pread_file(reinterpret_cast<char*>(&meta),sizeof(MetaInfo),current_offset);
                
                if(ret == FUN_EXIT){
                    fprintf(stderr,"read_segment_meta pread_file error\n");
                    return ret;
                }
            }
            return ret;
        }
    }
}