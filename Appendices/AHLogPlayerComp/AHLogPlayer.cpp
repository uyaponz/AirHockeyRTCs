// -*- C++ -*-
/*!
 * @file  AHLogPlayer.cpp
 * @brief ModuleDescription
 * @date $Date$
 *
 * $Id$
 */

#include "AHLogPlayer.h"

#include <cstdio>
#include <sys/time.h>
#include <string>

#include <opencv2/opencv.hpp>

static const double g_playSpeed = 0.5;

double gettimeofday_sec() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1e-6;
}

static FILE *fpPMlt = NULL;
static FILE *fpRMlt = NULL;
static FILE *fpPuck = NULL;
static double beginTime;

int readLogData(FILE *fp, CvPoint2D64f &pos) {
    double now = gettimeofday_sec();

    bool isFirst = true;
    while (true) {
        long fpPos = ftell(fp); if (fpPos==-1) return -1;

        double tm; CvPoint2D64f p;
        int ret = fscanf(fp, "%lf %lf %lf", &tm, &(p.x), &(p.y));
        if (ret == EOF) return -1;

        if (tm >= now-beginTime) {
            if (isFirst) pos = p;
            fseek(fp, fpPos, SEEK_SET);
            break;
        }

        isFirst = false;
        pos = p;
    }

    return 1;
}

// Module specification
// <rtc-template block="module_spec">
static const char* ahlogplayer_spec[] =
  {
    "implementation_id", "AHLogPlayer",
    "type_name",         "AHLogPlayer",
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
AHLogPlayer::AHLogPlayer(RTC::Manager* manager)
    // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager),
    m_pMalletXY_outOut("PMalletXY_out", m_pMalletXY_out),
    m_rMalletXY_outOut("RMalletXY_out", m_rMalletXY_out),
    m_puckXY_outOut("PuckXY_out", m_puckXY_out)

    // </rtc-template>
{
}

/*!
 * @brief destructor
 */
AHLogPlayer::~AHLogPlayer()
{
}



RTC::ReturnCode_t AHLogPlayer::onInitialize()
{
  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers
  
  // Set OutPort buffer
  addOutPort("PMalletXY_out", m_pMalletXY_outOut);
  addOutPort("RMalletXY_out", m_rMalletXY_outOut);
  addOutPort("PuckXY_out", m_puckXY_outOut);
  
  // Set service provider to Ports
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  
  // </rtc-template>

  m_pMalletXY_out.data.length(2);
  m_rMalletXY_out.data.length(2);
  m_puckXY_out.data.length(2);

  m_pMalletXY_out.data[0] = m_pMalletXY_out.data[1] = 0.0;
  m_rMalletXY_out.data[0] = m_rMalletXY_out.data[1] = 0.0;
  m_puckXY_out.data[0]    = m_puckXY_out.data[1]    = 0.0;

  return RTC::RTC_OK;
}


RTC::ReturnCode_t AHLogPlayer::onFinalize()
{
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t AHLogPlayer::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHLogPlayer::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

bool openFileAsReadable(FILE **fp, std::string filename) {
    *fp = fopen(filename.c_str(), "r");
    return (!(*fp)) ? false : true;
}

RTC::ReturnCode_t AHLogPlayer::onActivated(RTC::UniqueId ec_id)
{
    if (!fpPMlt) {
        if (!openFileAsReadable(&fpPMlt, "/home/admin/HkyLogs/HkyLogPMallet.log")) {
            std::cerr << "cannot open file" << std::endl;
            return RTC::RTC_ERROR;
        }
    }
    if (!fpRMlt) {
        if (!openFileAsReadable(&fpRMlt, "/home/admin/HkyLogs/HkyLogRMallet.log")) {
            std::cerr << "cannot open file" << std::endl;
            return RTC::RTC_ERROR;
        }
    }
    if (!fpPuck) {
        if (!openFileAsReadable(&fpPuck, "/home/admin/HkyLogs/HkyLogPuck.log")) {
            std::cerr << "cannot open file" << std::endl;
            return RTC::RTC_ERROR;
        }
    }

    beginTime = gettimeofday_sec();

    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHLogPlayer::onDeactivated(RTC::UniqueId ec_id)
{
    if (fpPMlt) { fclose(fpPMlt); fpPMlt=NULL; }
    if (fpRMlt) { fclose(fpRMlt); fpRMlt=NULL; }
    if (fpPuck) { fclose(fpPuck); fpPuck=NULL; }

    return RTC::RTC_OK;
}

void writePos(CvPoint2D64f pos, OutPort<TimedDoubleSeq> &data_outOut, TimedDoubleSeq &data_out) {
    coil::TimeValue tm = coil::gettimeofday() * g_playSpeed;
    data_out.tm.sec  = tm.sec();
    data_out.tm.nsec = tm.usec() * 1000;

    data_out.data[0] = pos.x;
    data_out.data[1] = pos.y;

    data_outOut.write();
}


RTC::ReturnCode_t AHLogPlayer::onExecute(RTC::UniqueId ec_id)
{
    CvPoint2D64f pMltPos;
    int retPMlt = readLogData(fpPMlt, pMltPos);
    CvPoint2D64f rMltPos;
    int retRMlt = readLogData(fpRMlt, rMltPos);
    CvPoint2D64f pPuckPos;
    int retPuck = readLogData(fpPuck, pPuckPos);

    if (retPMlt >= 1) writePos(pMltPos,  m_pMalletXY_outOut, m_pMalletXY_out);
    if (retRMlt >= 1) writePos(rMltPos,  m_rMalletXY_outOut, m_rMalletXY_out);
    if (retPuck >= 1) writePos(pPuckPos, m_puckXY_outOut,    m_puckXY_out);

    return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t AHLogPlayer::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHLogPlayer::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHLogPlayer::onReset(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHLogPlayer::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHLogPlayer::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/



extern "C"
{
 
  void AHLogPlayerInit(RTC::Manager* manager)
  {
    coil::Properties profile(ahlogplayer_spec);
    manager->registerFactory(profile,
                             RTC::Create<AHLogPlayer>,
                             RTC::Delete<AHLogPlayer>);
  }
  
};


