#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>
#include <native/event.h>
//#include <rtdk.h>
#include <native/queue.h>

#define TASK1_PRIO 51
#define TASK2_PRIO 50

#define TASK_MODE   0
#define TASK_STKSZ  0
#define SAMPLING_TIME 60
#define QUEUE_SIZE 255

#define TASK1_PERIOD 1000000       // 1ms
#define TASK2_PERIOD 1000000       // 1ms
#define BUFSIZE (SAMPLING_TIME*1000)
//#define T2
//#define SEM
//#define EVF
//#define MSGQ

RT_TASK sem_task1;
RT_TASK sem_task2;
RT_EVENT event;
RT_EVENT event2;
RT_SEM semapore;
RT_QUEUE queue;

int quitFlag=0, i=0;
int jitter_buf[BUFSIZE+100] = {'\0'};
int computing_buf[BUFSIZE+100] = {'\0'};
int period_buf[BUFSIZE+100] = {'\0'};
int jitter_buf2[BUFSIZE+100] = {'\0'};
int computing_buf2[BUFSIZE+100] = {'\0'};
int period_buf2[BUFSIZE+100] = {'\0'};
void signal_handler(int sig){
    quitFlag = 1;
}

void SemTask1(void *arg){
    RTIME prev_c, now_c;
    RTIME prev_j, now_j;
    int i=0;
    unsigned long mask=0,mask2=0;
    char buf;
    int computing, jitter, period;

    prev_c = rt_timer_read();
    prev_j = rt_timer_read();

    while(!quitFlag){
        now_j = rt_timer_read();
        //rt_task_wait_period(NULL);
        rt_event_wait(&event, 0x00000001, &mask, EV_ALL, TM_INFINITE);
        rt_event_clear(&event, 0xffffffff, &mask);
        prev_c = rt_timer_read();

#ifdef SEM
    	rt_sem_p(&semapore,TM_INFINITE);
#endif
#ifdef MSGQ
        rt_queue_read(&queue, &buf, sizeof(buf),TM_INFINITE);
#endif
#ifdef EVF
        rt_event_wait(&event2, 0x00000002, &mask2, EV_ALL, TM_INFINITE);
        rt_event_clear(&event2, 0xffffffff, &mask2);
#endif
        if(period >= TASK1_PERIOD){
            jitter_buf2[i] = period - TASK1_PERIOD;
        }
        else{
            jitter_buf2[i] = TASK1_PERIOD - period;
        }
        period_buf2[i] = period;
        //rt_printf("Task jitter1 : %d.%06d\n",jitter/1000000,jitter%1000000);
        now_c = rt_timer_read();
        computing_buf2[i] = (int)now_c - (int)prev_c;
        //rt_printf("Task computing_time1 : %d.%06d\n",computing_buf2[i]/1000000,computing_buf2[i]%1000000);
        period = (int)now_j - (int)prev_j;
        prev_j = now_j;
#ifdef SEM
        rt_sem_v(&semapore);
#endif
        i++;
        
    }
}


void SemTask2(void *arg){
    RTIME prev_c, now_c;
    RTIME prev_j, now_j;
    char buf;
    int period;
    prev_c = rt_timer_read();
    prev_j = rt_timer_read();

    while(!quitFlag){
        now_j = rt_timer_read();

        rt_task_wait_period(NULL); 
        prev_c = rt_timer_read();
        rt_event_signal(&event, 0x00000001);
#ifdef SEM
        rt_sem_p(&semapore,TM_INFINITE);     
#endif
#ifdef MSGQ
        rt_queue_write(&queue,&buf,sizeof(buf),Q_NORMAL);
#endif
#ifdef EVF
        rt_event_signal(&event2, 0x00000002);
#endif
        if(period >= TASK1_PERIOD){
            jitter_buf[i] = period - TASK1_PERIOD;
        }
        else{
            jitter_buf[i] = TASK1_PERIOD - period;
        }
        period_buf[i] = period;
        //rt_printf("Task jitter2 : %d.%06d\n",jitter_buf[i]/1000000,jitter_buf[i]%1000000);
        now_c = rt_timer_read();
        computing_buf[i] = (int)now_c - (int)prev_c;
        //rt_printf("Task computing_time2 : %d.%06d\n",computing_buf[i]/1000000,computing_buf[i]%1000000);
        period = (int)now_j - (int)prev_j;
        prev_j = now_j;
#ifdef SEM
        rt_sem_v(&semapore);
#endif
        i++;
    }
}


void task_init(){
    //rt_timer_set_mode(0);
    rt_print_auto_init(1);

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    mlockall(MCL_CURRENT|MCL_FUTURE);
    rt_event_create(&event, "Event", 0, EV_FIFO);

#ifdef SEM
    rt_sem_create(&semapore, "Semapore", 1, S_FIFO);
#endif
#ifdef MSGQ
    rt_queue_create(&queue,"queue",QUEUE_SIZE,10,Q_FIFO|Q_SHARED);
#endif
#ifdef EVF
    rt_event_create(&event2, "Event", 0, EV_FIFO);
#endif

    rt_task_create(&sem_task1, "semTask1", TASK_STKSZ, TASK1_PRIO, TASK_MODE);
    rt_task_set_periodic(&sem_task1, TM_NOW, rt_timer_ns2ticks(TASK1_PERIOD));
    rt_task_start(&sem_task1, &SemTask1, NULL);

    rt_task_create(&sem_task2, "semTask2", TASK_STKSZ, TASK2_PRIO, TASK_MODE);
    rt_task_set_periodic(&sem_task2, TM_NOW, rt_timer_ns2ticks(TASK2_PERIOD));
    rt_task_start(&sem_task2, &SemTask2, NULL);
    //rt_printf("task start\n");
}

void task_exit(){
    rt_task_delete(&sem_task1);
    rt_task_delete(&sem_task2);
    rt_event_delete(&event);

#ifdef SEM
    rt_sem_delete(&semapore);
#endif
#ifdef MSGQ
    rt_queue_delete(&queue);
#endif
#ifdef EVF
    rt_event_delete(&event2);
#endif
}

/*****************************************************************************/
int main(int argc, char * argv[])
{
    FILE *fp;
    int ii;
#ifdef SEM
    fp = fopen("sem.txt","w");
#endif
#ifdef MSGQ
    fp = fopen("msgq.txt","w");
#endif
#ifdef EVF
    fp = fopen("evf.txt","w");
#endif
#ifdef T2
    fp = fopen("task2.txt","w");
#endif

    task_init();
    while(!quitFlag){
        if(i == BUFSIZE-1){
            quitFlag = 1;
        }
        usleep(1);
    }
    for(ii=0;ii<BUFSIZE;ii++){
        fprintf(fp, "%d.%06d %d.%06d %d.%06d           %d.%06d %d.%06d %d.%06d\n",
                period_buf[ii]/1000000, period_buf[ii]%1000000 ,jitter_buf[ii]/1000000, jitter_buf[ii]%1000000, computing_buf[ii]/1000000, computing_buf[ii]%1000000,
                period_buf2[ii]/1000000, period_buf2[ii]%1000000 ,jitter_buf2[ii]/1000000, jitter_buf2[ii]%1000000, computing_buf2[ii]/1000000, computing_buf2[ii]%1000000);
        if(!computing_buf[ii+1]){
            break;
        }
    }
    task_exit();
    return 0;
}

