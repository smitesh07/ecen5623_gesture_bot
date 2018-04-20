/*
 *
 *  Example by Sam Siewert 
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

struct timespec start_tp,end_tp;

#define HRES 160
#define VRES 120

// Transform display window
char timg_window_name[] = "Edge Detector Transform";

int lowThreshold=0;
int const max_lowThreshold = 100;
int kernel_size = 3;
int edgeThresh = 1;
int ratio = 3;
Mat canny_frame, cdst, timg_gray, timg_grad;

IplImage * frame;

void CannyThreshold(int, void*)
{
    Mat mat_frame(frame);

    cvtColor(mat_frame, timg_gray, CV_RGB2GRAY);

    /// Reduce noise with a kernel 3x3
    blur( timg_gray, canny_frame, Size(3,3) );

    /// Canny detector
    Canny( canny_frame, canny_frame, lowThreshold, lowThreshold*ratio, kernel_size );

    /// Using Canny's output as a mask, we display our result
    timg_grad = Scalar::all(0);

    mat_frame.copyTo( timg_grad, canny_frame);

    imshow( timg_window_name, timg_grad );

}


int main( int argc, char** argv )
{
    CvCapture* capture;
    int dev=0;

    if(argc > 1)
    {
        sscanf(argv[1], "%d", &dev);
        printf("using %s\n", argv[1]);
    }
    else if(argc == 1)
        printf("using default\n");

    else
    {
        printf("usage: capture [dev]\n");
        exit(-1);
    }

    namedWindow( timg_window_name, CV_WINDOW_AUTOSIZE );
    // Create a Trackbar for user to enter threshold
    createTrackbar( "Min Threshold:", timg_window_name, &lowThreshold, max_lowThreshold, CannyThreshold );

    //capture = (CvCapture *)cvCreateCameraCapture(dev);
    //cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
    // cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);

    capture = (CvCapture *)cvCreateCameraCapture(dev);
    
    double dWidth = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
    double dHeight = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
    cout << "Frame size : " << dWidth << " x " << dHeight << endl;
    
   cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
   cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);

    dWidth = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
    dHeight = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
    cout << "Frame size After: " << dWidth << " x " << dHeight << endl;

    while(1)
    {
        clock_gettime(CLOCK_REALTIME,&start_tp); 

        frame=cvQueryFrame(capture);
        if(!frame) break;


        CannyThreshold(0, 0);

        clock_gettime(CLOCK_REALTIME,&end_tp); 
        printf("Time diff s:%ld ns:%ld\n",end_tp.tv_sec-start_tp.tv_sec,end_tp.tv_nsec-start_tp.tv_nsec);

        char q = cvWaitKey(33);
        if( q == 'q' )
        {
            printf("got quit\n"); 
            break;
        }
    }

    cvReleaseCapture(&capture);
    
};
