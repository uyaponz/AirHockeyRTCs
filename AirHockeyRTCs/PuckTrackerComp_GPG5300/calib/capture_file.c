/* ***************************************************************
   capture.c - Source Code of GPG-5300 sample program
   ---------------------------------------------------------------
   Version 1.00-01
   ---------------------------------------------------------------
   Date 2006/09/19
   ---------------------------------------------------------------
   Copyright 2006 Interface Corporation. All rights reserved.
   *************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <opencv/cv.h>
// #include <opencv/cvaux.h>
// #include <opencv/cvwimage.h>
#include <opencv/cxcore.h>
// #include <opencv/cxmisc.h>
#include <opencv/highgui.h>
// #include <ml.h>
#include <opencv2/legacy/legacy.hpp>
#include <ctype.h>
#include <math.h>

#include "ifcml.h"

#include <time.h>
#include <sys/time.h>

static inline void *_aligned_malloc(size_t size, size_t alignment)
{
    void *p;
    int ret = posix_memalign(&p, alignment, size);
    return (ret == 0) ? p : 0;
}

#define ImgWidth 640
#define ImgHeight 480

#define		 	FRAMECNT	1
int             ret;
int             DeviceNum;
unsigned long   MemHandle;
void*			SrcPtr;
void*			DataPtr;
IFCMLCAPFMT     CapFmt;
IplImage* src_img = 0;
IplImage* rgb_img = 0;

char file_set_name[50];
int f_no=0;

int count;
int ch;

double gettimeofday_sec() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec+tv.tv_usec*1e-6;
}

void my_mouse_callback(int event, int x, int y, int flags, void* param);

void RETPRINT(long ret){
	
	switch(ret){
//	case IFCML_ERROR_SUCCESS:
//	    printf("--ret : IFCML_ERROR_SUCCESS [%#x]\n",ret);	
//		break;
	case IFCML_ERROR_NOT_DEVICE:
	    printf("--ret : IFCML_ERROR_NOT_DEVICE [%#x]\n",ret);	
		break;
	case IFCML_ERROR_NOT_OPEN:
	    printf("--ret : IFCML_ERROR_NOT_OPEN [%#x]\n",ret);	
		break;
	case IFCML_ERROR_INVALID_DEVICE_NUMBER:
	    printf("--ret : IFCML_ERROR_INVALID_DEVICE_NUMBER [%#x]\n",ret);	
		break;
	case IFCML_ERROR_ALREADY_OPEN:
	    printf("--ret : IFCML_ERROR_ALREADY_OPEN [%#x]\n",ret);	
		break;
	case IFCML_ERROR_NOT_SUPPORTED:
	    printf("--ret : IFCML_ERROR_NOT_SUPPORTED [%#x]\n",ret);	
		break;
	case IFCML_ERROR_INVALID_PARAMETER:
	    printf("--ret : IFCML_ERROR_INVALID_PARAMETER [%#x]\n",ret);	
		break;
	case IFCML_ERROR_NOT_ALLOCATE_MEMORY:
	    printf("--ret : IFCML_ERROR_NOT_ALLOCATE_MEMORY [%#x]\n",ret);	
		break;
	case IFCML_ERROR_NOW_CAPTURING:
	    printf("--ret : IFCML_ERROR_NOW_CAPTURING [%#x]\n",ret);	
		break;
	case IFCML_ERROR_NOW_STOP:
	    printf("--ret : IFCML_ERROR_NOW_STOP [%#x]\n",ret);	
		break;
	case IFCML_ERROR_MEM_NOW_CAPTURING:
	    printf("--ret : IFCML_ERROR_MEM_NOW_CAPTURING [%#x]\n",ret);	
		break;
	case IFCML_ERROR_MEM_NOW_STOP:
	    printf("--ret : IFCML_ERROR_MEM_NOW_STOP [%#x]\n",ret);	
		break;
	case IFCML_ERROR_NULL_POINTER:
	    printf("--ret : IFCML_ERROR_NULL_POINTER [%#x]\n",ret);	
		break;
	case IFCML_ERROR_FIFO_FULL:
	    printf("--ret : IFCML_ERROR_FIFO_FULL [%#x]\n",ret);	
		break;
	case IFCML_ERROR_NOBUFFER:
	    printf("--ret : IFCML_ERROR_NOBUFFER [%#x]\n",ret);	
		break;
	case IFCML_ERROR_INVALID_BUF_HANDLE:
	    printf("--ret : IFCML_ERROR_INVALID_BUF_HANDLE [%#x]\n",ret);	
		break;
	case IFCML_ERROR_SERIAL_TIMEOUT:
	    printf("--ret : IFCML_ERROR_SERIAL_TIMEOUT [%#x]\n",ret);	
		break;
	case IFCML_ERROR_INVALID_FILE_PRAM:
	    printf("--ret : IFCML_ERROR_INVALID_FILE_PRAM [%#x]\n",ret);	
		break;
	case IFCML_ERROR_NOTSET_CAMFILE:
	    printf("--ret : IFCML_ERROR_NOTSET_CAMFILE [%#x]\n",ret);	
		break;
	default:
	    printf("--ret : ANOTHER_ERROR [%#x]\n",ret);	
		break;
	}
}

void Init()
{
	DeviceNum = 1;
	MemHandle = -1;
	SrcPtr = NULL;
	DataPtr = NULL;

	cvNamedWindow( "Display", CV_WINDOW_AUTOSIZE);
	src_img = cvCreateImage(cvSize(ImgWidth, ImgHeight), IPL_DEPTH_8U, 1);
	rgb_img = cvCreateImage(cvSize(ImgWidth, ImgHeight), IPL_DEPTH_8U, 3);
	DataPtr = src_img->imageData;

	//cvSetMouseCallback("Display", my_mouse_callback, (void*) rgb_img);
	cvSetMouseCallback("Display", my_mouse_callback, (void*) rgb_img);
}

void Exit()
{
	if(DeviceNum != -1){
		CmlClose(DeviceNum);
	}
	if(SrcPtr != NULL) free(SrcPtr);
	if(DataPtr != NULL) free(DataPtr);
}

 void my_mouse_callback(int event, int x, int y, int flags, void* param){
   IplImage* img = (IplImage*)param;
   char file_name[100];
   switch(event) {
   case CV_EVENT_LBUTTONDOWN: {
     sprintf(file_name,"%s%02d.png",file_set_name,f_no);
     cvSaveImage(file_name,img,0);
     printf("image has been saved in %s\n",file_name);
     f_no++;
     //    cvWaitKey(20);
    cvWaitKey(2);

   }
   break;
   }
 }

void CallBackDisplay(unsigned long IntFlg, unsigned long User) {

  int i;
  count++;
  cvCvtColor(src_img,rgb_img,CV_BayerBG2BGR);
  cvShowImage( "Display", rgb_img );
  ch = cvWaitKey(2);
  //	printf("0\n");
}




int main(void)
{

	int type;
	int yn;
	unsigned long  DataKind;
	unsigned long  AllocBufSize;
	unsigned long  StartMode;

	double start,end;

	Init();

	ret = CmlOpen(DeviceNum);
	if(ret != IFCML_ERROR_SUCCESS){
	  RETPRINT(ret);
	  return 0;
	}


	// Open Camera ConfigFile
	// Teli  CS6910CL
	// RGB24 640~480
	ret = CmlReadCamConfFile(DeviceNum,"./mv_d640.cfg");
	if(ret != IFCML_ERROR_SUCCESS){
		RETPRINT(ret);
		Exit();
		return 0;
	}

	CapFmt.Rect.XStart = 0;
	CapFmt.Rect.YStart = 0;
	CapFmt.Rect.XLength = ImgWidth;
	CapFmt.Rect.YLength = ImgHeight;
	CapFmt.Scale.PixelCnt = 0;
	CapFmt.Scale.LineCnt = 0;

	CapFmt.CapFormat = IFCML_CAPFMT_CAM;

	//	if(CapFmt.CapFormat) DataKind |= 0x01;
	if(CapFmt.CapFormat) DataKind = 0x01;

	CapFmt.OptionFormat = IFCML_OPTFMT_NON;
	
	ret = CmlSetCaptureFormatInfo(DeviceNum,&CapFmt);
	if(ret != IFCML_ERROR_SUCCESS){
		RETPRINT(ret);
		Exit();
		return 0;
	}

	// Allocate the buffer for storing the image data.
	// Size of 1 farame
//	printf("MemType: \n");
//	printf(" 0 - main memory \n");
//	printf(" 1 - board memory \n");
//	scanf("%d", &type);
	type = 0;
	printf("mem size %ld,  no. of line %ld\n", CapFmt.FrameSize_Mem, CapFmt.FrameSize_Mem/ImgWidth);

	if(type){
		AllocBufSize = FRAMECNT * CapFmt.FrameSize_Mem;
		SrcPtr = NULL;
		StartMode = IFCML_CAM_MEM;
	}else{
		AllocBufSize = FRAMECNT * CapFmt.FrameSize_Buf;
		SrcPtr = _aligned_malloc(AllocBufSize, 2048*2);
		//SrcPtr = malloc(AllocBufSize * 2);
		//printf("SrcPtr0=%x\n",SrcPtr);
		//SrcPtr = ((int)SrcPtr/2048 + 1) * 2048;
		printf("SrcPtr=%x\n",SrcPtr);
		StartMode = IFCML_CAM_DMA;
		src_img->imageData = (char *)SrcPtr;
	}
	/*
	DataPtr = malloc(CapFmt.ImageSize);
	if(DataPtr == NULL){
		printf("DataPtr is NULL \n");
		Exit();
		return 0;
	}
	*/
	ret = CmlRegistMemInfo(DeviceNum,SrcPtr,AllocBufSize,&MemHandle);
	if(ret != IFCML_ERROR_SUCCESS){
		RETPRINT(ret);
		Exit();
		return 0;
	}

	// Set Capture Configration
	ret = CmlSetCapConfig(DeviceNum,MemHandle,&CapFmt);
	if(ret != IFCML_ERROR_SUCCESS){
		RETPRINT(ret);
		Exit();
		return 0;
	}

	ret = CmlStartCapture(DeviceNum, 0 , StartMode | IFCML_CAP_ASYNC);
	if(ret != IFCML_ERROR_SUCCESS){
		RETPRINT(ret);
		Exit();
		return 0;
	}
	printf("save filename? :");
	scanf("%s",file_set_name);

	printf("Start Capture !!\n");

	ch = 0;
	count=0;
	start=gettimeofday_sec();
	CmlSetEventMask(DeviceNum, 0x11);
	CmlSetEvent(DeviceNum, (PIFCMLCALLBACK)CallBackDisplay, 1234);
	if(ret != IFCML_ERROR_SUCCESS){
		RETPRINT(ret);
		Exit();
		return 0;
	}
	while(ch != 27) {
	// count++;
	  //	ch = cvWaitKey(2);       // wait time >= 2 msec.
	//	printf("0\n");
	} // end of while
	end=gettimeofday_sec();
	printf("time= %f, count= %ld, average=%f\n",end-start,count,(end-start)/count);
	/*
	CmlSetEventMask(DeviceNum, 0x00);
	if(ret != IFCML_ERROR_SUCCESS){
		RETPRINT(ret);
		Exit();
		return 0;
	}
	ret = CmlStopCapture(DeviceNum, IFCML_MEM_STOP);
	if(ret != IFCML_ERROR_SUCCESS){
		RETPRINT(ret);
		Exit();
		return 0;
	}
	printf("Stop Capture !!\n");



	printf("1\n");
	cvDestroyWindow ("Display");
	printf("2\n");
	cvReleaseImage (&src_img);
	printf("3\n");
	// Free MemHandle
	ret = CmlFreeMemInfo(DeviceNum,MemHandle);
	if(ret != IFCML_ERROR_SUCCESS){
		RETPRINT(ret);
	}
	printf("4\n");
	//	Exit();
	*/
	return 0;
}
