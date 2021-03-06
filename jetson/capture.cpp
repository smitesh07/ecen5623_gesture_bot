/**
*@file 
*@brief hand gesture controlled robot
*@Author Smitesh Modak and Aakash Kumar
*@date 
*/

// Standard Libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

//Libraries for thread creation and profiling
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>

#include <time.h>
#include <syslog.h>
#include <sys/time.h>

#include <errno.h>

//Libraries for opencv
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//Personal Libraries
#include "serial.h"

//Macros
#define USEC_PER_MSEC (1000)
#define NANOSEC_PER_SEC (1000000000)
#define NUM_CPU_CORES (1)

//#define TRUE (1)
//#define FALSE (0)

#define NUM_THREADS (2+1)

//#define HRES 640
//#define VRES 480

using namespace cv;
using namespace std;

int abortTest=FALSE;
int abortS1=FALSE, abortS2=FALSE;

sem_t semS1, semS2;

struct timeval start_time_val;

//structure for threads
typedef struct
{
    int threadIdx;
    unsigned long long sequencePeriods;
} threadParams_t;

//prototypes of functions
void *Sequencer(void *threadp);

void *Service_1(void *threadp);
void *Service_2(void *threadp);

double getTimeMsec(void);
void print_scheduler(void);

// Transform display window
char timg_window_name[] = "Hand Gesture Detecion";

//Global variables
CvCapture* capture;
IplImage* src;
IplImage* gray;
char serial_sig;

/***************************************************************************************************************************/

//Main function
int main(void)
{	

	int dev = 0;    
    capture = (CvCapture *)cvCreateCameraCapture(dev);			//opens the camera
    
	if(!cvQueryFrame(capture))
	{
		cout<<"Video camera capture status: OK"<<endl;
	}
	else
	{
		cout<<"Video capture failed, please check the camera."<<endl;
	}
    
	struct timeval current_time_val;
    int i, rc, scope;
    
    
    //pthread attribute initialization
    cpu_set_t threadcpu;
    pthread_t threads[NUM_THREADS];
    threadParams_t threadParams[NUM_THREADS];
    pthread_attr_t rt_sched_attr[NUM_THREADS];
    int rt_max_prio, rt_min_prio;
    struct sched_param rt_param[NUM_THREADS];
    struct sched_param main_param;
    pthread_attr_t main_attr;
    pid_t mainpid;
    cpu_set_t allcpuset;

    printf("Starting Sequencer Demo\n");
    gettimeofday(&start_time_val, (struct timezone *)0);
    gettimeofday(&current_time_val, (struct timezone *)0);
    syslog(LOG_CRIT, "Sequencer @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);

   	//printf("System has %d processors configured and %d available.\n", get_nprocs_conf(), get_nprocs());

   	CPU_ZERO(&allcpuset);

   	//setting cpu affinity
   	for(i=0; i < NUM_CPU_CORES; i++)
       CPU_SET(i, &allcpuset);

   	printf("Using CPUS=%d from total available.\n", CPU_COUNT(&allcpuset));


    // initialize the sequencer semaphores
    //
    if (sem_init (&semS1, 0, 0)) { printf ("Failed to initialize S1 semaphore\n"); exit (-1); }
    if (sem_init (&semS2, 0, 0)) { printf ("Failed to initialize S2 semaphore\n"); exit (-1); }

    mainpid=getpid();

    //get maximum and minimum priorities for threads
	rt_max_prio = sched_get_priority_max(SCHED_FIFO);
    rt_min_prio = sched_get_priority_min(SCHED_FIFO);
	
	//giving main function the highest priority
    rc=sched_getparam(mainpid, &main_param);
    main_param.sched_priority=rt_max_prio;
    rc=sched_setscheduler(getpid(), SCHED_FIFO, &main_param);
    if(rc < 0) perror("main_param");
    print_scheduler();


    pthread_attr_getscope(&main_attr, &scope);

	//determine the scope of the system and print it
    if(scope == PTHREAD_SCOPE_SYSTEM)
      printf("PTHREAD SCOPE SYSTEM\n");
    else if (scope == PTHREAD_SCOPE_PROCESS)
      printf("PTHREAD SCOPE PROCESS\n");
    else
      printf("PTHREAD SCOPE UNKNOWN\n");

    printf("rt_max_prio=%d\n", rt_max_prio);
    printf("rt_min_prio=%d\n", rt_min_prio);
	
	//setting attributes for all the threads
    for(i=0; i < NUM_THREADS; i++)
    {

      CPU_ZERO(&threadcpu);
      CPU_SET(3, &threadcpu);

      rc=pthread_attr_init(&rt_sched_attr[i]);
      rc=pthread_attr_setinheritsched(&rt_sched_attr[i], PTHREAD_EXPLICIT_SCHED);
      rc=pthread_attr_setschedpolicy(&rt_sched_attr[i], SCHED_FIFO);
      //rc=pthread_attr_setaffinity_np(&rt_sched_attr[i], sizeof(cpu_set_t), &threadcpu);

      rt_param[i].sched_priority=rt_max_prio-i;
      pthread_attr_setschedparam(&rt_sched_attr[i], &rt_param[i]);

      threadParams[i].threadIdx=i;
    }

    printf("Service threads will run on %d CPU cores\n", CPU_COUNT(&threadcpu));

    // Create Service threads which will block awaiting release for:
    //

    // Servcie_1 (Hand Detection and Gesture Control) = RT_MAX-1	@ 30 Hz  (30fps)
    //
    rt_param[1].sched_priority=rt_max_prio-1;
    pthread_attr_setschedparam(&rt_sched_attr[1], &rt_param[1]);
    rc=pthread_create(&threads[1],               // pointer to thread descriptor
                      &rt_sched_attr[1],         // use specific attributes
                      //(void *)0,               // default attributes
                      Service_1,                 // thread function entry point
                      (void *)&(threadParams[1]) // parameters to pass in
                     );
    if(rc < 0)
        perror("pthread_create for service 1");
    else
        printf("pthread_create successful for service 1\n");


    // Service_2 (Motor Control) = RT_MAX-2	
    //
    rt_param[2].sched_priority=rt_max_prio-2;
    pthread_attr_setschedparam(&rt_sched_attr[2], &rt_param[2]);
    rc=pthread_create(&threads[2], &rt_sched_attr[2], Service_2, (void *)&(threadParams[2]));
    if(rc < 0)
        perror("pthread_create for service 2");
    else
        printf("pthread_create successful for service 2\n");

    
    //sem_post(&semS1);
    // Create Sequencer thread, which like a cyclic executive, is highest prio
    printf("Start sequencer\n");
    //threadParams[0].sequencePeriods=900;

    // Sequencer = RT_MAX	@ 30 Hz
    //
    rt_param[0].sched_priority=rt_max_prio;
    pthread_attr_setschedparam(&rt_sched_attr[0], &rt_param[0]);
    rc=pthread_create(&threads[0], &rt_sched_attr[0], Sequencer, (void *)&(threadParams[0]));
    if(rc < 0)
        perror("pthread_create for sequencer service 0");
    else
        printf("pthread_create successful for sequeencer service 0\n");

    
   for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);

   printf("\nTEST COMPLETE\n");
	return 0;
}


//sequencer thread 
void *Sequencer(void *threadp)
{
    struct timeval current_time_val;
    struct timespec delay_time = {0,33333333}; // delay for 33.33 msec, 30 Hz
    struct timespec remaining_time;
    double current_time;
    double residual;
    int rc, delay_cnt=0;
    unsigned long long seqCnt=0;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    gettimeofday(&current_time_val, (struct timezone *)0);
    syslog(LOG_CRIT, "Sequencer thread @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
    printf("Sequencer thread @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
	
    
   
	//loop to determine the total delay count and increment the counter
     do
    {
        delay_cnt=0; residual=0.0;

        //gettimeofday(&current_time_val, (struct timezone *)0);
        //syslog(LOG_CRIT, "Sequencer thread prior to delay @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
        do
        {
            rc=nanosleep(&delay_time, &remaining_time);

            if(rc == EINTR)
            {
                residual = remaining_time.tv_sec + ((double)remaining_time.tv_nsec / (double)NANOSEC_PER_SEC);

                if(residual > 0.0) printf("residual=%lf, sec=%d, nsec=%d\n", residual, (int)remaining_time.tv_sec, (int)remaining_time.tv_nsec);

                delay_cnt++;
            }
            else if(rc < 0)
            {
                perror("Sequencer nanosleep");
                exit(-1);
            }

        } while((residual > 0.0));

        seqCnt++;
        gettimeofday(&current_time_val, (struct timezone *)0);
        syslog(LOG_CRIT, "Sequencer cycle %llu @ sec=%d, msec=%d\n", seqCnt, (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);


        if(delay_cnt > 1) printf("Sequencer looping delay %d\n", delay_cnt);
   
    
    // Release each service at a sub-rate of the generic sequencer rate

       
        // Servcie_1 = RT_MAX-1	@ 30 Hz
        
        if(seqCnt%4==0 ) 
			sem_post(&semS1);
/*
        // Service_2 = RT_MAX-2	@ 1 Hz
        if((seqCnt % 10) == 0) 
			sem_post(&semS2);
*/

//        gettimeofday(&current_time_val, (struct timezone *)0);
//        syslog(LOG_CRIT, "Sequencer release all sub-services @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
//
//    } while(!abortTest && (seqCnt < threadParams->sequencePeriods));

        //cout<<"Inside Sequencer"<<endl;
	} while(!abortTest);


    pthread_exit((void *)0);
}



void *Service_1(void *threadp)
{
    struct timeval current_time_val,start_service,end_service;
    double current_time;
    unsigned long long S1Cnt=0;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    struct timespec start_s1,end_s1;

    gettimeofday(&current_time_val, (struct timezone *)0);
    
    while(1)
    {
        
		int c=0;
        
		while( c != 27)
		{
        sem_wait(&semS1);

        clock_gettime(CLOCK_REALTIME,&start_s1); //start_s1,end_s1

        //cout<<"Camera starting"<<endl;
        
		CvSize sz = cvGetSize(cvQueryFrame( capture));
		cout << "Height & width of captured frame: " << sz.height <<" x " << sz.width<<endl;
		src    = cvCreateImage( sz,8, 3 );
		gray   = cvCreateImage( cvSize(270,270),8, 1 );
		src = cvQueryFrame(capture);
		cvSetImageROI(src, cvRect(340,100,270,270));		//set ROI
		cvCvtColor(src,gray,CV_BGR2GRAY);					//convert to grayscale
		cvSmooth(gray,gray,CV_BLUR,(12,12),0);				//smoothing uses linear convolution over it and blurs the image
		cvNamedWindow( "Blur",1);cvShowImage( "Blur",gray);   // blur-not-clear
		cvThreshold(gray,gray,0,255,(CV_THRESH_BINARY_INV+CV_THRESH_OTSU));			//thresolding converts the image into black and white
		cvNamedWindow( "Threshold",1);cvShowImage( "Threshold",gray);  // black-white
		CvMemStorage* storage = cvCreateMemStorage();
		CvSeq* first_contour = NULL;
		CvSeq* maxitem=NULL;
		int cn=cvFindContours(gray,storage,&first_contour,sizeof(CvContour),CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE,cvPoint(0,0));
		double area,max_area=0.0;
		CvSeq* ptr=0;
		//int maxn=0,n=0;
		
		// loop to get the contour with maximum area and ignore the frames if maximum area of contour
		// is less than 1000
		
		if(cn>0)
		{
			for(ptr=first_contour;ptr!=NULL;ptr=ptr->h_next)
			{
				area=fabs(cvContourArea(ptr,CV_WHOLE_SEQ,0));
				if(area>max_area)
				{
					max_area=area;
					maxitem=ptr;
					//maxn=n;
				}
				// n++;
			}
			if(max_area > 1000)
			{
				CvPoint pt0;
				CvMemStorage* storage1 = cvCreateMemStorage();
				CvMemStorage* storage2 = cvCreateMemStorage(0);
				CvSeq* ptseq = cvCreateSeq( CV_SEQ_KIND_GENERIC|CV_32SC2, sizeof(CvContour),sizeof(CvPoint), storage1 );
				CvSeq* hull;
				CvSeq* defects;
				for(int i = 0; i < maxitem->total; i++ )
				{
					CvPoint* p = CV_GET_SEQ_ELEM( CvPoint, maxitem, i );
					pt0.x = p->x;
					pt0.y = p->y;
					cvSeqPush( ptseq, &pt0 );
				}
				hull = cvConvexHull2( ptseq, 0, CV_CLOCKWISE, 0 );   // find convex hull of the sequence from the contours
				int hullcount = hull->total;
				defects= cvConvexityDefects(ptseq,hull,storage2  );  			//find the convex defects
				// pt0 = **CV_GET_SEQ_ELEM( CvPoint*, hull, hullcount - 1 );
				// printf("** : %d :**",hullcount);
				CvConvexityDefect* defectArray;
				// int j=0;
				for(int i = 1; i <= hullcount; i++ )
				{
					CvPoint pt = **CV_GET_SEQ_ELEM( CvPoint*, hull, i );
					cvLine( src, pt0, pt, CV_RGB( 255, 0, 0 ), 1, CV_AA, 0 );
					pt0 = pt;
				}
				for( ; defects; defects = defects->h_next)  
				{
					int nomdef = defects->total; // defect amount
					// outlet_float( m_nomdef, nomdef );
					// printf(" defect no %d \n",nomdef);
					if(nomdef == 0)
					continue;
					// Alloc memory for defect set.
					// fprintf(stderr,"malloc\n");
					defectArray = (CvConvexityDefect*)malloc(sizeof(CvConvexityDefect)*nomdef);
					// Get defect set.
					// fprintf(stderr,"cvCvtSeqToArray\n");
					cvCvtSeqToArray(defects,defectArray, CV_WHOLE_SEQ);
					// Draw marks for all defects.
					int con=0;
					for(int i=0; i<nomdef; i++)
					{
						if(defectArray[i].depth > 40 )
						{
							con=con+1;
							// printf(" defect depth for defect %d %f \n",i,defectArray[i].depth);
							cvLine(src, *(defectArray[i].start), *(defectArray[i].depth_point),CV_RGB(255,255,0),1, CV_AA, 0 );  
							cvCircle( src, *(defectArray[i].depth_point), 5, CV_RGB(0,0,255), 2, 8,0);
							cvCircle( src, *(defectArray[i].start), 5, CV_RGB(0,255,0), 2, 8,0);  
							cvLine(src, *(defectArray[i].depth_point), *(defectArray[i].end),CV_RGB(0,255,255),1, CV_AA, 0 );  
							cvDrawContours(src,defects,CV_RGB(0,0,0),CV_RGB(255,0,0),-1,CV_FILLED,8);
						}
					}
					// cout<<con<<"\n";
					
					//based on number of fingers detected generate the serial signal to be sent
					
					char txt[40]="";
					if(con==1)
					{	
						serial_sig = '1';
						char txt1[]="2 Forward";
						strcat(txt,txt1);
					}
					else if(con==2)
					{
						serial_sig = '2';
						char txt1[]="3 Backward";
						strcat(txt,txt1);
					}
					else if(con==3)
					{
						serial_sig = '3';
						char txt1[]="4 Left";
						strcat(txt,txt1);
					}
					else if(con==4)
					{
						serial_sig = '4';
						char txt1[]="5 Right";
						strcat(txt,txt1);
					}
					else
					{
						serial_sig = '5';
						char txt1[]="Stop"; 
						strcat(txt,txt1);
					}
					cvNamedWindow( "contour",1);cvShowImage( "contour",src);
					cvResetImageROI(src);
					CvFont font;
					cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.5, 1.5, 0, 5, CV_AA);
					cvPutText(src, txt, cvPoint(50, 50), &font, cvScalar(0, 0, 255, 0));
					// j++;  
					// Free memory.
					free(defectArray);
				} 
				cvReleaseMemStorage( &storage1 );
				cvReleaseMemStorage( &storage2 );
			}
		}
		cvReleaseMemStorage( &storage );
		cvNamedWindow( "threshold",1);cvShowImage( "threshold",src);
  
        clock_gettime(CLOCK_REALTIME,&end_s1); 
        //gettimeofday(&current_time_val, (struct timezone *)0);
		gettimeofday(&end_service, (struct timezone *)0);
        cout << "Camera Service execution time: sec :"
        <<(end_s1.tv_sec-start_s1.tv_sec)<<" nsec:"<<(end_s1.tv_nsec-start_s1.tv_nsec)<<endl;
    	
    
    	//cout<<"Camera ending"<<endl;
    	sem_post(&semS2);

    c = cvWaitKey(100);
	}
	cvReleaseCapture( &capture);
	cvDestroyAllWindows();
	}
	
	
    pthread_exit((void *)0);
}


void *Service_2(void *threadp)
{
    struct timeval current_time_val,start_service,end_service;
    double current_time;
    unsigned long long S2Cnt=0;

    struct timespec start_s2,end_s2;

    //char sent=49;

	char received;

    threadParams_t *threadParams = (threadParams_t *)threadp;

    int port=open_port(ARDUINO_SERIAL);			//open serial port

    if(port<0) 
        {
            cout<<"Motor Not found !!!!"<<endl;
            exit(0);
        }

    syslog(LOG_CRIT, "Time-stamp with Motor Service thread @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
    //printf("Time-stamp with Motor Service thread @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);

    while(1)
    {
        sem_wait(&semS2);
	    clock_gettime(CLOCK_REALTIME,&start_s2); 
        S2Cnt++;
        
        //if(sent>53)  sent=49;
         

        write(port,&serial_sig,1);			//serial write
        
       // read(port,&received,1); 
        cout<<"Sent : "<<serial_sig<<endl;
	//<<" Motor Ack : "<<received<<endl;

        clock_gettime(CLOCK_REALTIME,&end_s2); 
        cout << "Motor thread execution time: sec :"
        <<(end_s2.tv_sec-start_s2.tv_sec)<<" nsec:"<<(end_s2.tv_nsec-start_s2.tv_nsec)<<endl;
    	//sem_post(&semS1);
        
	}

    pthread_exit((void *)0);
}


double getTimeMsec(void)
{
  struct timespec event_ts = {0, 0};

  clock_gettime(CLOCK_MONOTONIC, &event_ts);
  return ((event_ts.tv_sec)*1000.0) + ((event_ts.tv_nsec)/1000000.0);
}


//function to print out the scheduler
void print_scheduler(void)
{
   int schedType;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
       case SCHED_FIFO:
           printf("Pthread Policy is SCHED_FIFO\n");
           break;
       case SCHED_OTHER:
           printf("Pthread Policy is SCHED_OTHER\n"); exit(-1);
         break;
       case SCHED_RR:
           printf("Pthread Policy is SCHED_RR\n"); exit(-1);
           break;
       default:
           printf("Pthread Policy is UNKNOWN\n"); exit(-1);
   }
}


