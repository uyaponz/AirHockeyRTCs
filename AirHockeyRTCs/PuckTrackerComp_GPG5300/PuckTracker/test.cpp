#include "PuckTracker.hpp"
#include "../AHCamera/AHCamera_GPG5300.hpp"

#include <iostream>

static const char *winname = "PuckTracker Test";
static const bool USE_DEBUG_OUTPUT = false;

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
    IplImage *img = cvCreateImage(cvSize(AHCamera::camW, AHCamera::camH), IPL_DEPTH_8U, 3);
    AHCamera::initCamera(1, "../mv_d640.cfg");
    {
        PuckTracker::PuckTrackerParameters paras;
        PuckTracker pt(paras);

        cvNamedWindow(winname, CV_WINDOW_AUTOSIZE);
        for (;;) {
            if (AHCamera::getCaptureImage(img)) {
                cvSmooth(img, img, CV_MEDIAN, 3,3,0,0);
                PuckTracker::PuckPosition pos = pt.getPuckPosition(img);
                if (pos.likelihood > 0.0) {
                    cvShowImage(winname, img);

                    /* --- debug output --- */
                    if (USE_DEBUG_OUTPUT) {
                        std::cout << "pos: ";
                        std::cout << pos.position.x << ", " << pos.position.y;
                        std::cout << std::endl;
                    }
                }
                else {
                    cvShowImage(winname, img);
                }
            }

            char key = cvWaitKey(2);
            if (key == 'q') break;
        }
        cvDestroyAllWindows();
    }
    AHCamera::releaseCamera();
    cvReleaseImage(&img);

    return 0;
}
