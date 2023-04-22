
#include <sstream>
#include "../include/index_handle.h"
#include <iostream>

using namespace std;

// 内存映射参数
const static ww::file::MMapOption mmap_option = {1024000, 4096, 4096};
// 主块文件大小
const static uint32_t main_blocksize = 1024 * 1024 * 64;
// 哈希桶的大小
const static uint32_t bucket_size = 1000;

static int32_t block_id = 1;

int main(void)
{
    string mainblock_path = "";
    string index_path = "";
    int32_t ret = FUN_SUCCEED;

    cout << "please input your block id:" << endl;
    cin >> block_id;
    if (block_id < 1)
    {
        cerr << "invalid blockid!!" << endl;
        return -1;
    }

    // 1.加载索引文件
    ww::file::IndexHandle *index_handle = new ww::file::IndexHandle(".", block_id);

    fprintf(stdout, "load index file...\n");

    ret = index_handle->load(block_id, bucket_size, mmap_option);
    if (ret != FUN_SUCCEED)
    {
        fprintf(stderr, "load index %d failed.\n", block_id);
        delete index_handle;
        return -2;
    }

    delete index_handle;
    return 0;
}