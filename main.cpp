#include <iostream>
#include <aio.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

using namespace std;

#ifdef DEBUG
#define debug2(v1,v2) {cout<<v1<<"    "<<v2<<endl;}
#define debugr3(v1,v2,v3) {cout<<v1<<"    "<<v2<<"    "<<v3<<endl;}
#define debug4(v1,v2,v3,v4) {cout<<v1<<"    "<<v2<<"    "<<v3<<"    "<<v4<<endl;}
#else
#define debug2(v1,v2) ;
#define debug3(v1,v2,v3) ;
#define debug4(v1,v2,v3,v4) ;
#endif

const	int	    debug		=	1;
const	long	BLOCK_SIZE	=	512;
const	long 	MAX_BLOCK	=	256;
const	long	MAX_TRACE_COUNT	= 	500000;
const	long	WRITE		=	0;
const	long	READ		=	1;

//trace struct
typedef	struct	_io_trace
{
    double			time;
    unsigned long   blkno;   //blk的序号
    int				blkcount;   //长度占几个blk
    unsigned int	flag;
}io_trace;

typedef	struct	_io_time
{
    double	start_time;
    double	end_time;
    double	elpsd_time;
    unsigned int	flag;     //读操作为0，写操作为1
}io_time;

//signal
typedef struct _sig_data
{
    int 		number;
    struct	aiocb64	*aio_req;
}sig_data;

//functions
double get_time(void);
void do_io();
//void callback(sigval_t sigval);
void aio_complete_note( int signo, siginfo_t *info, void *context );
int trace_reader(char *file_name, io_trace *trace, unsigned long max_trace_num, float timescale, float rangescale);
void usage(void);
void deal_by_num();
void deal_by_time();
unsigned long trace_stat(char *file_name, unsigned long  *max_dev_addr);

//define
struct aiocb64 my_aiocb[MAX_TRACE_COUNT];
sig_data	my_data[MAX_TRACE_COUNT];
io_trace	trace[MAX_TRACE_COUNT];
io_time		my_time[MAX_TRACE_COUNT];
static	int 		total=0;
static	double	start=0;
char 	*exit_code;

static	unsigned long	long	dev_size=0;
//defines
char 	trace_file_name[255];
char 	result_file_name[255];
char 	myname[255];
char	dev_name[255];
int 	deal_time,deal_num;
int		trace_num;
float		timescale;
float		rangescale;
//unsigned long rangescale;
double trace_end_time =0 ;

unsigned long   max_trace_addr;


int main(int argc,char **argv) {
    std::cout << "Hello, World!" << std::endl;
   // char s[255];
  //  cin>>s;
   // strcpy(dev_name,s);
   // string s="/dev/pblk_0_3";
    strcpy(dev_name,"/dev/pblk_0_3");
    //cout<<dev_name<<endl;
    if(strlen(dev_name)==0)
    {
        printf("error: no dev!\n");
       // usage();
        exit(0);
    }
    do_io();
    return 0;
}
void aio_complete_note( int signo, siginfo_t *info, void *context )
{
    sig_data *req;
    int ret;

    /* Ensure it's our signal */
    if (info->si_signo == SIGIO) {
        req = (sig_data *)info->si_value.sival_ptr;
        /* Did the request complete? */
        if (aio_error64(req->aio_req) == 0) {
            /* Request completed successfully, get the return status */
            ret = aio_return64(req->aio_req);
        //    my_time[req->number].end_time=get_time()-start;
      //      my_time[req->number].elpsd_time=my_time[req->number].end_time-my_time[req->number].start_time;
//		printf("--we get here--used time =%lf---\n",my_time[req->number].elpsd_time);
        }
    }
    return;
}
void do_io()
{
    int i=0;
    struct sigaction sig_act; //add by maobo

    int fd=open(dev_name,O_RDWR|O_LARGEFILE);   //读写模式打开，
    if(fd==-1)
    {
        cout<<"open "<<dev_name<<" error!"<<endl;
        exit(0);
    }
    cout<<"1"<<endl;
    struct stat st;
    fstat(fd,&st);  //得到文件打开状态，赋值给st

    struct  aiocb64 myaio;
    unsigned long max=1;
    //for(i=0;i<total;i++)
     //   if(trace[i].blkcount>max)
         //   max=trace[i].blkcount;
    myaio.aio_buf=malloc(max*BLOCK_SIZE+5);   //aio_buffer 起始位置
    cout<<"2"<<endl;
    cout<<"myaio.aio_buf:"<<myaio.aio_buf<<endl;
 //   memset(myaio.aio_buf, 'a',sizeof(max*BLOCK_SIZE+1));
    total=10;
    for(i=0;i<total;i++)
    {
        bzero((char *)&my_aiocb[i],sizeof(struct aiocb));

        my_aiocb[i].aio_fildes = fd;
        my_aiocb[i].aio_buf = myaio.aio_buf;
        my_aiocb[i].aio_nbytes = 1*BLOCK_SIZE;   //转换长度
        my_aiocb[i].aio_offset = (i+1)*BLOCK_SIZE;

        sigemptyset(&sig_act.sa_mask);      //add by maobo
        sig_act.sa_flags = SA_SIGINFO;
        sig_act.sa_sigaction = aio_complete_note;
        my_aiocb[i].aio_sigevent.sigev_notify = SIGEV_SIGNAL;
        my_aiocb[i].aio_sigevent.sigev_signo = SIGIO;


        //link callback
        if(i<1)
        {cout<<"L:"<<my_aiocb[i].aio_nbytes<<",";
            cout<<"blkno:"<<trace[i].blkno<<",";
            cout<<"offset:"<<my_aiocb[i].aio_offset<<endl;
        }

       my_data[i].number=i;
        my_data[i].aio_req=&my_aiocb[i];
        my_aiocb[i].aio_sigevent.sigev_value.sival_ptr = &my_data[i];
//
        sigaction( SIGIO, &sig_act, NULL );//add by maobo

    }
    cout<<"3"<<endl;
    double temp_time;
    i=0;
  //  start=get_time();
    struct  aiocb64 lhjaio;
    while(i < total)
    {

            cout<<"写入"<<endl;
                aio_write64(&my_aiocb[i]);  //异步写入，如何实现？
              //  my_time[i].flag=1;

                cout<<"读出"<<endl;
        lhjaio.aio_fildes=my_aiocb[i].aio_fildes ;
       // lhjaio.aio_buf=my_aiocb[i].aio_buf ;
        lhjaio.aio_nbytes=my_aiocb[i].aio_nbytes ;   //转换长度
        lhjaio.aio_offset=my_aiocb[i].aio_offset+2618368 ;
               aio_read64(&lhjaio);   //异步读
                cout<<"lhjaio.aio_buf: "<<lhjaio.aio_buf<<endl;
                cout<<"lhjaio.aio_nbytes: "<<lhjaio.aio_nbytes<<endl;
                cout<<"lhjaio.aio_offset: "<<lhjaio.aio_offset<<endl;
              //  my_time[i].flag=0;

            i++;
            debug4("time:",temp_time,"send req:",i-1);

    }
    total=i-1;
   // trace_end_time= get_time();
    sleep(1);
    close(fd);
}
