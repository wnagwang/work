
#include <sstream>
#include "../include/index_handle.h"
#include <iostream>

using namespace std;

static const int debug = 1;

//内存映射参数
const static ww::file::MMapOption mmap_option = {1024000,4096,4096};
//主块文件大小
const static uint32_t main_blocksize = 1024*1024*64;
//哈希桶的大小
const static uint32_t bucket_size = 1000;

static int32_t block_id = 1;

int main(void)
{
    string mainblock_path = "";
    string index_path = "";
    int32_t ret = FUN_SUCCEED;

    cout<<"please input your block id:"<<endl;
    cin>>block_id;
    if(block_id < 1)
    {
        cerr<<"invalid blockid!!"<<endl;
        return -1;
    }
    
    

    //1.加载索引文件
    ww::file::IndexHandle* index_handle = new ww::file::IndexHandle(".",block_id);
    if(debug)
    {
        fprintf(stdout,"load index file...\n");
    }
    ret = index_handle->load(block_id,bucket_size,mmap_option);
    if(ret != FUN_SUCCEED)
    {
       fprintf(stderr,"load index %d failed.\n",block_id); 
       delete index_handle;
       return -2;
    }

    //2.将文件写入到主块文件
    std::stringstream tmp_streams;
    tmp_streams << "."<<ww::file::MAINBLOCK<<block_id;
    tmp_streams >> mainblock_path;
    //创建主块操作对象
    ww::file::file_operation* mainblock = new ww::file::file_operation(mainblock_path,O_RDWR|O_CREAT|O_LARGEFILE);
    
    char buffer[4096];
    memset(buffer,'6',sizeof(buffer));
    //拿到主块偏移
    int32_t data_offset = index_handle->get_offset();
    //拿到可分配的文件编号
    uint32_t file_no = index_handle->block_info()->seq_no_;

    ret = mainblock->pwrite_file(buffer,sizeof(buffer),data_offset);
    if(ret!=FUN_SUCCEED)
    {
        fprintf(stderr,"write to main block failed.ret:%d,reason:%s\n",ret,strerror(errno));
        mainblock->close_file();
        delete mainblock;
        delete index_handle;
        return -3;
    }
    //文件已经写入成功主块
    //3.索引文件中写入metaInfo
    ww::file::MetaInfo meta;
    meta.set_id(file_no);
    meta.set_offset(data_offset);
    meta.set_size(sizeof(buffer));
    printf("meta_key:%u\n",meta.get_id());
    ret = index_handle->write_segment_meta(meta.get_key(),meta);    
    
    if(ret==FUN_SUCCEED)
    {
        //1.更新索引头部信息
        index_handle->commit_block_data_offset(sizeof(buffer));
        //2.更新块信息
        index_handle->updata_block_info(ww::file::C_OPER_INSERT,sizeof(buffer));

        ret = index_handle->flush();
        if(ret!=FUN_SUCCEED)
        {
            fprintf(stderr,"flush mainblock %d failed %d failed.file_no:%u\n",block_id,file_no);
        }
    }
    else
    {
        fprintf(stderr,"write_segment_meta failed mainblock %d failed %d failed.file_no:%u\n",block_id,file_no);

    }

    if(ret==FUN_SUCCEED)
    {
        if(debug)
            fprintf(stdout,"write succesfully.file_no:%u,block_id:%d\n",file_no,block_id);
    }

    //4.其他操作
    mainblock->close_file();
    index_handle->flush();

    delete mainblock;
    delete index_handle;
    return 0;
}