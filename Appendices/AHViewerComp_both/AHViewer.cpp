// -*- C++ -*-
/*!
 * @file  AHViewer.cpp
 * @brief ModuleDescription
 * @date $Date$
 *
 * $Id$
 */

#include "AHViewer.h"

#include "MyMath.hpp"
#include "/home/shared/Projects/Releases/AHComp/HockeyLibs2/HockeyDefinitions.hpp"

#include <opencv2/opencv.hpp>

static const char winName[] = "AHViewer";
static const int winW = 600;
static const int winH = 800;
static const int ofsX = 50;
static const int ofsY = 50;
static const double scale = 3.5;

// Module specification
// <rtc-template block="module_spec">
static const char* ahviewer_spec[] =
  {
    "implementation_id", "AHViewer",
    "type_name",         "AHViewer",
    "description",       "ModuleDescription",
    "version",           "1.0.0",
    "vendor",            "ysatoh",
    "category",          "Category",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "0",
    "language",          "C++",
    "lang_type",         "compile",
    ""
  };
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
AHViewer::AHViewer(RTC::Manager* manager)
    // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager),
    m_pMalletXY_inIn("PMalletXY_in", m_pMalletXY_in),
    m_rMalletXY_inIn("RMalletXY_in", m_rMalletXY_in),
    m_puckXY_inIn("PuckXY_in", m_puckXY_in),
    frame(cv::Size(winW,winH), CV_8UC3, cv::Scalar(255,255,255))

    // </rtc-template>
{
}

/*!
 * @brief destructor
 */
AHViewer::~AHViewer()
{
}



RTC::ReturnCode_t AHViewer::onInitialize()
{
  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers
  addInPort("PMalletXY_in", m_pMalletXY_inIn);
  addInPort("RMalletXY_in", m_rMalletXY_inIn);
  addInPort("PuckXY_in", m_puckXY_inIn);
  
  // Set OutPort buffer
  
  // Set service provider to Ports
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  
  // </rtc-template>

  m_pMalletXY_in.data.length(2);
  m_rMalletXY_in.data.length(2);
  m_puckXY_in.data.length(2);

  m_pMalletXY_in.data[0] = m_pMalletXY_in.data[1] = 0.0;
  m_rMalletXY_in.data[0] = m_rMalletXY_in.data[1] = 0.0;
  m_puckXY_in.data[0]    = m_puckXY_in.data[1]    = 0.0;

  cv::circle(frame, cv::Point(300,100), 100, cv::Scalar(0,0,200), 3, 4);
  cv::namedWindow(winName, CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
  cv::imshow(winName, frame);
  cv::waitKey(10); cv::waitKey(10);

  return RTC::RTC_OK;
}


RTC::ReturnCode_t AHViewer::onFinalize()
{
    cv::destroyAllWindows();
    return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t AHViewer::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHViewer::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHViewer::onActivated(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHViewer::onDeactivated(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

void drawHockeyBoard(cv::Mat &f)
{
    { // 外枠 + 中央線
        CvPoint from = convH2CV(cvPoint2D64f(-HKY_W/2.0, -HKY_H/2.0),
                                cvPoint2D64f(HKY_W, HKY_H),
                                scale, cvPoint(ofsX,ofsY));
        CvPoint to   = convH2CV(cvPoint2D64f(HKY_W/2.0, HKY_H/2.0),
                                cvPoint2D64f(HKY_W, HKY_H),
                                scale, cvPoint(ofsX,ofsY));
        cv::rectangle(f, from, to, cv::Scalar(255,0,255));
        from = convH2CV(cvPoint2D64f(-HKY_W/2.0, 0.0),
                        cvPoint2D64f(HKY_W, HKY_H),
                        scale, cvPoint(ofsX,ofsY));
        to   = convH2CV(cvPoint2D64f(HKY_W/2.0, 0.0),
                        cvPoint2D64f(HKY_W, HKY_H),
                        scale, cvPoint(ofsX,ofsY));
        cv::line(f, from, to, cv::Scalar(255,0,255));
    }
}

void drawMallet(cv::Mat &f, CvPoint2D64f malletXY, double R, CvScalar color)
{
    CvPoint center = convH2CV(malletXY, cvPoint2D64f(HKY_W,HKY_H),
                              scale, cvPoint(ofsX,ofsY));
    int r_ = static_cast<int>((R/2.0) / scale);
    cv::circle(f, center, r_, color, -1);
}

RTC::ReturnCode_t AHViewer::onExecute(RTC::UniqueId ec_id)
{
    m_pMalletXY_inIn.read();
    m_rMalletXY_inIn.read();
    m_puckXY_inIn.read();

    CvPoint2D64f pMlt = cvPoint2D64f(m_pMalletXY_in.data[0], m_pMalletXY_in.data[1]);
    CvPoint2D64f rMlt = cvPoint2D64f(m_pMalletXY_in.data[2], m_pMalletXY_in.data[3]);
    CvPoint2D64f puck = cvPoint2D64f(m_puckXY_in.data[0],    m_puckXY_in.data[1]);

    frame = cv::Scalar(255,255,255);
    drawHockeyBoard(frame);
    drawMallet(frame, pMlt, PLAYER_MLT_R, cv::Scalar(255,128,0));
    drawMallet(frame, rMlt, ARM_MLT_R,    cv::Scalar(255,0,0));
    drawMallet(frame, puck, PUCK_R,       cv::Scalar(0,0,255));

    cv::imshow(winName, frame);
    cv::waitKey(2);

    return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t AHViewer::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHViewer::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHViewer::onReset(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHViewer::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHViewer::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/



extern "C"
{
 
  void AHViewerInit(RTC::Manager* manager)
  {
    coil::Properties profile(ahviewer_spec);
    manager->registerFactory(profile,
                             RTC::Create<AHViewer>,
                             RTC::Delete<AHViewer>);
  }
  
};


