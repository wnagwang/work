#include <curl/curl.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <thread>

#define THREAD_NUM 10

struct filedata
{
    const char *url;
    char *fileptr;
    int offfset;
    int end;
    int downloadlen;  
    pthread_t thread_id;
};

struct filedata** info;
long downloadfilelength;
// url回来的数据        类型      个数          用户参数
size_t writefunc(void *ptr, size_t size, size_t memb, void *userdata)
{
    // printf("writefunc\n");

    struct filedata *data = (struct filedata *)userdata;

    memcpy(data->fileptr + data->offfset, ptr, size * memb);

    data->offfset += (size * memb);

    //printf("write%d\n", size * memb);

    return size * memb;
}

int progressfunc(void* userdata,double download,double nowdownload,double upload,double nowupload){
    int percent = 0;
    static int print = 1;
    struct filedata* data = (struct filedata*) userdata;
    data->downloadlen = nowdownload;
    if(download > 0){ 
        int i = 0;
        double alldownload = 0;
        for(i = 0 ; i <= THREAD_NUM; i++){
            alldownload += info[i]->downloadlen;
        }
        percent = (int)(alldownload/downloadfilelength*100);
    }  
    if(percent == print){
        printf("down--------------%d%%\n",percent);
        print += 1;
    }
    return 0;
}
void* work(void* arg){
    struct filedata *data = (struct filedata*)arg;
    const char *url = data->url;
    
    char range[64]={0};
    snprintf(range,64,"%d-%d",data->offfset,data->end);
    
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl,CURLOPT_URL,url);
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,writefunc);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,data);
    curl_easy_setopt(curl,CURLOPT_NOPROGRESS,0L);
    curl_easy_setopt(curl,CURLOPT_PROGRESSFUNCTION,progressfunc);
    curl_easy_setopt(curl,CURLOPT_PROGRESSDATA,data);
    curl_easy_setopt(curl,CURLOPT_RANGE,range);
    printf("thradid:%ld -----down%d to %d\n",data->thread_id,data->offfset,data->end);
    CURLcode res = curl_easy_perform(curl);

    if(res != CURLE_OK){
        printf("res %d\n",res);
    }

    curl_easy_cleanup(curl);
}


double getdownloadfilelength(const char *url)
{
    double downloadfilelength = 0;
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK)
    {
        printf("getlength success\n");
        curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadfilelength);
        printf("length1 %lf\n", downloadfilelength); //
    }
    else
    {
        perror("getlength error\n");
        downloadfilelength = -1;
    }

    curl_easy_cleanup(curl);
    printf("length1 %lf\n", downloadfilelength); //
    return downloadfilelength;
}

int download(const char *url, const char *filename)
{
    int fd = open(filename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if (fd == -1)
        return -1;

    downloadfilelength = getdownloadfilelength(url);
    printf("length2 %d\n", downloadfilelength); //

    if (-1 == lseek(fd, downloadfilelength - 1, SEEK_SET))
    {
        printf("lseek\n");
        close(fd);
        return -1;
    }

    if (-1 == write(fd, "", 1))
    {
        printf("write\n");
        close(fd);
        return -1;
    }

    char *ptr = (char *)mmap(NULL, downloadfilelength, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        printf("mmap");
        close(fd);
        return -1;
    }

    //pthread_t td[THREAD_NUM+1] ={0};
    int downsize = downloadfilelength / THREAD_NUM;

    struct filedata *data[THREAD_NUM+1] = {0};
    
    for (int i = 0; i <= THREAD_NUM; i++)
    {
        data[i] = (struct filedata *)malloc(sizeof(struct filedata));

        if (data[i] == NULL)
        {
            printf("malloc");
            close(fd);
            return -1;
        }
        data[i]->url = url;
        data[i]->fileptr = ptr;
        data[i]->offfset = i*downsize;
        
        if(i<THREAD_NUM){
            data[i]->end = (i+1)*downsize-1;
        }else{
            data[i]->end = downloadfilelength - 1;
        }
        //data[i]->thread_id = td[i];
    }
    info = data;
    for (int i = 0; i <= THREAD_NUM; i++){
        //printf("create\n");
        pthread_create(&(data[i]->thread_id),NULL,work,data[i]);
    }
    for (int i = 0; i <= THREAD_NUM; i++){
        pthread_join(data[i]->thread_id,NULL);
    }
    for (int i = 0; i <= THREAD_NUM; i++){
        free(data[i]);
    }
    // CURL* curl = curl_easy_init();
    // curl_easy_setopt(curl,CURLOPT_URL,url);
    // curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,writefunc);
    // curl_easy_setopt(curl,CURLOPT_WRITEDATA,data);
    // CURLcode res = curl_easy_perform(curl);

    // if(res != CURLE_OK){
    //     printf("res %d\n",res);
    // }

    // curl_easy_cleanup(curl);
    // printf("write success\n");
    munmap(ptr, downloadfilelength);
    //free(data);
    close(fd);
    return 0;
}

int main(int argc, char *argv[])
{

    download("https://releases.ubuntu.com/22.04/ubuntu-22.04.3-live-server-amd64.iso.zsync", "pip");
    return 0;
}