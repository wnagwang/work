#include <curl/curl.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>


struct filedata
{
    char* fileptr;
    int offfset;
    int end;
};



                //url回来的数据        类型      个数          用户参数
size_t writefunc(void* ptr,size_t size,size_t memb, void* userdata){
    //printf("writefunc\n");
    
    struct filedata *data = (struct filedata*) userdata;

    memcpy(data->fileptr+data->offfset,ptr,size*memb);

    data->offfset+= (size*memb);

    printf("write%d\n",size*memb);

    return size*memb;
}

double getdownloadfilelength(const char* url){
    double downloadfilelength = 0;
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl,CURLOPT_URL,url);
    curl_easy_setopt(curl,CURLOPT_HEADER,1);
    curl_easy_setopt(curl,CURLOPT_NOBODY,1);

    CURLcode res = curl_easy_perform(curl);
    
    if(res == CURLE_OK){
        printf("getlength success\n");
        curl_easy_getinfo(curl,CURLINFO_CONTENT_LENGTH_DOWNLOAD,&downloadfilelength);
        printf("length1 %lf\n",downloadfilelength);//
    }else{
        perror("getlength error\n");
        downloadfilelength = -1;
    }
 
    curl_easy_cleanup(curl);
    printf("length1 %lf\n",downloadfilelength);//
    return downloadfilelength;
}

int download(const char* url,const char* filename){
    int fd = open(filename,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR);

    if(fd == -1) return -1;


    long downloadfilelength = getdownloadfilelength(url);
    printf("length2 %d\n",downloadfilelength);//

    if(-1 == lseek(fd,downloadfilelength - 1,SEEK_SET)){
        printf("lseek\n");
        close(fd);
        return -1;
    }

    if(1 != write(fd,"",1)){
        printf("write\n");
        close(fd);
        return -1;
    }

    char* ptr = (char*) mmap(NULL,downloadfilelength,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    if(ptr == MAP_FAILED){
        printf("mmap");
        close(fd);
        return -1;
    }

    struct filedata * data = (struct filedata * )malloc(sizeof (struct filedata));

    if(data == NULL){
        printf("malloc");
        close(fd);
        return -1;
    }
    data->fileptr = ptr;
    data->offfset = 0;
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl,CURLOPT_URL,url); 
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,writefunc);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,data);
    CURLcode res = curl_easy_perform(curl);
    
    if(res != CURLE_OK){
        printf("res %d\n",res);
    }

    curl_easy_cleanup(curl);
    printf("write success\n");
    munmap(ptr,downloadfilelength);
    free(data);
    close(fd);
    return 0;
}

int main(int argc,char* argv[]){
    download("https://releases.ubuntu.com/22.04/ubuntu-22.04.3-live-server-amd64.iso.zsync","pip");
    return 0;
}