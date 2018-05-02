#include <stdio.h>
#include "serial.h"
#include <time.h>

int main()
{
    while(1)
    {
        sem_wait(&semS2);
	    clock_gettime(CLOCK_REALTIME,&start_s2); 
        S2Cnt++;
        
        if(sent>53)  sent=49;
         

        write(port,&sent,1);
        
        read(port,&received,1); 
        cout<<"Sent : "<<sent<<" Motor Ack : "<<received<<endl;
        sent++;

        clock_gettime(CLOCK_REALTIME,&end_s2); 
        
	    printf("Motor thread execution time: sec : %d  nsec: %d \n"
            (end_s2.tv_sec-start_s2.tv_sec),(end_s2.tv_nsec-start_s2.tv_nsec));
    }
}