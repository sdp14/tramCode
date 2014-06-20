#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <stdlib.h>


int bandTLimit = 150;
int stopLimit = 150;
/* signal process */
void timeout_info(int signo)
{
   if(bandTLimit == 0)
   {
       //printf("restarting battery");
       //bandTLimit = 1000;
       exit(0);  
   }
   if(stopLimit == 0)
   {
       //printf("restarting stop base");
       //stopLimit = 50;
       exit(0);
   }
     bandTLimit--;
     stopLimit--;
}

/* init sigaction */
void init_sigaction(void)
{
    struct sigaction act;

    act.sa_handler = timeout_info;
    act.sa_flags   = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGPROF, &act, NULL);
} 

/* init */
void init_time(void)
{
    struct itimerval val;

    val.it_value.tv_sec = 0;
    val.it_value.tv_usec = 1;
    val.it_interval = val.it_value;
    setitimer(ITIMER_PROF, &val, NULL);
}


int main(void)
{
    init_sigaction();
    init_time();

    while(1);
}
