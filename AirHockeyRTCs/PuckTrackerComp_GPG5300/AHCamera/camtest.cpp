#include "AHCamera_GPG5300.hpp"

#include <iostream>
#include <opencv2/opencv.hpp>

static const char *winname = "test";

#include <time.h>
#include <sys/time.h>
static inline double gettimeofday_sec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

int main()
{
    IplImage *srcImg  = cvCreateImage(cvSize(AHCamera::camW, AHCamera::camH), IPL_DEPTH_8U, 3);
    IplImage *showImg = cvCreateImage(cvSize(AHCamera::camW, AHCamera::camH), IPL_DEPTH_8U, 3);
    {
        AHCamera::initCamera(1, "../mv_d640.cfg");
        {
            cvNamedWindow(winname, CV_WINDOW_AUTOSIZE);
            cvShowImage(winname, showImg);
            for (;;) {
                if (AHCamera::getCaptureImage(srcImg)) {
                    double t1 = gettimeofday_sec();
//                    cvCopy(srcImg, showImg);
                    cvSmooth(srcImg, showImg, CV_MEDIAN, 3,3,0,0);
//                    cvSmooth(showImg, showImg, CV_MEDIAN, 3,3,0,0);
                    cvShowImage(winname, showImg);
                    double t2 = gettimeofday_sec();
                    std::cout << "smoothing time : " << (t2-t1) << std::endl;
                }

                char key = cvWaitKey(2);
                if (key == 'q') break;
            }
            cvDestroyAllWindows();
        }
        AHCamera::releaseCamera();
    }
    cvReleaseImage(&showImg);
    cvReleaseImage(&srcImg);

    return 0;
}
