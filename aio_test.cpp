//
// Created by lhj on 17-12-18.
//
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <aio.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
double get_time(void) {
    struct timeval mytime;
    gettimeofday(&mytime,NULL);
    return (mytime.tv_sec*1.0+mytime.tv_usec/1000000.0);
}

int main(int argc, char **argv){
    struct aiocb64 read_aio;
    struct aiocb64 write_aio;
    char buf_write[4097];
    char buf_read[4097 * 5];
    char dev_name[30];
    double stat_time = 0.0;
    double end_time = 0.0;
    strcpy(dev_name, "/dev/pblk_0_1");
    cout<<"1"<<endl;
    //dev_name
    int fd = open(dev_name, O_RDWR|O_LARGEFILE);
    cout<<"fd:"<<fd<<endl;
    if(fd == -1){
        printf("Open device error!\n");
        return -1;
    }
    bzero((char *)&read_aio, sizeof(struct aiocb64));
    bzero((char *)&write_aio, sizeof(struct aiocb64));
    cout<<"2"<<endl;
    for(int i=0; i < 4096; i++){
        buf_write[i] = ('a'+ (i % 26));
    }
    cout<<"3"<<endl;
    write_aio.aio_buf = malloc(4097);
    read_aio.aio_buf = malloc(4097);
    if(!write_aio.aio_buf)
        perror("malloc\n");
    write_aio.aio_fildes = fd;
    write_aio.aio_nbytes = 4096;
    write_aio.aio_offset = 2618368;
    memcpy((void *)write_aio.aio_buf, (void *)buf_write, 4096);
    cout<<"write_aio: "<<write_aio.aio_buf<<endl;
    cout<<"4"<<endl;
    aio_write64(&write_aio);
    while(EINPROGRESS == aio_error64(&write_aio));
    read_aio.aio_fildes = fd;
    read_aio.aio_nbytes = 4096;
    read_aio.aio_offset = 2618368;
    stat_time= get_time();
    aio_read64(&read_aio);
    cout<<"5"<<endl;


    while(EINPROGRESS == aio_error64(&read_aio));
    char *read_tmp = (char *)malloc(4096);
    char *write_tmp = (char *)malloc(4096);
    memcpy(read_tmp, (void *)read_aio.aio_buf, 4096);
    memcpy(write_tmp,(void *)write_aio.aio_buf,4096);
    cout<<"read:"<<read_tmp<<endl;
    if(memcmp((void *)read_aio.aio_buf,(void *) write_aio.aio_buf, 4096))
       cout << "abcd" << endl;
    else cout << "smae" << endl;
    //cout<<"read_aio: "<<(string)read_aio.aio_buf<<endl;
    cout<<"6"<<endl;
    end_time = get_time();
    cout<<"end_time: "<<end_time<<endl;
   // printf("5 chunk:%.6lf\navertime:%.6lf", (end_time - stat_time) * 1000, (end_time - stat_time) * 1000.0 / 5);
    sleep(1);
    close(fd);

    fd=open("dev_name",O_RDWR|O_LARGEFILE);     //以只读的方式打开文件 test.txt
    char buf[4096];
    int n;
    int count=1024;
    read(fd,buf,count)  ;
    cout<<"buf:"<<buf<<endl;
    close(fd);

    /*
    read_aio.aio_fildes = fd;
    read_aio.aio_nbytes = 4096;
    read_aio.aio_offset = 0;
    stat_time= get_time();
    aio_read64(&read_aio);
    cout<<"7"<<endl;
    while(EINPROGRESS == aio_error64(&read_aio));
    end_time = get_time();
    printf("5 chunk:%.6lf\navertime:%.6lf", (end_time - stat_time) * 1000 * 5, (end_time - stat_time) * 1000.0);

//    printf("%s\n", buf_write);
//    printf("%s\n", buf_read);
     */
    return 1;
}
