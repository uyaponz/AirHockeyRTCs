// -*- C++ -*-
/*!
 * @file  AHLogger.cpp
 * @brief ModuleDescription
 * @date $Date$
 *
 * $Id$
 */

#include "AHLogger.h"

#include <cstdio>
#include <sys/time.h>

double gettimeofday_sec() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1e-6;
}

static FILE *fpPMlt = NULL;
static FILE *fpRMlt = NULL;
static FILE *fpPuck = NULL;
static double beginTime = 0.0;

void writeLogData(FILE *fp,
                  InPort<TimedDoubleSeq> &data_inIn,
                  TimedDoubleSeq &data_in)
{
    if (!data_inIn.isNew()) return;
    data_inIn.read();

    double now = gettimeofday_sec();
    fprintf(fp, "%lf %lf %lf\n",
            now - beginTime, data_in.data[0], data_in.data[1]);
}

void writePMallet(InPort<TimedDoubleSeq> &data_inIn, TimedDoubleSeq &data_in) {
    writeLogData(fpPMlt, data_inIn, data_in);
}
void writeRMallet(InPort<TimedDoubleSeq> &data_inIn, TimedDoubleSeq &data_in) {
    writeLogData(fpRMlt, data_inIn, data_in);
}
void writePuck(InPort<TimedDoubleSeq> &data_inIn, TimedDoubleSeq &data_in) {
    writeLogData(fpPuck, data_inIn, data_in);
}


// Module specification
// <rtc-template block="module_spec">
static const char* ahlogger_spec[] =
  {
    "implementation_id", "AHLogger",
    "type_name",         "AHLogger",
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
AHLogger::AHLogger(RTC::Manager* manager)
    // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager),
    m_playerMalletXY_inIn("PlayerMalletXY", m_playerMalletXY_in),
    m_robotMalletXY_inIn("RobotMalletXY", m_robotMalletXY_in),
    m_puckXY_inIn("PuckXY", m_puckXY_in)

    // </rtc-template>
{
}

/*!
 * @brief destructor
 */
AHLogger::~AHLogger()
{
}



RTC::ReturnCode_t AHLogger::onInitialize()
{
  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers
  addInPort("PlayerMalletXY", m_playerMalletXY_inIn);
  addInPort("RobotMalletXY", m_robotMalletXY_inIn);
  addInPort("PuckXY", m_puckXY_inIn);
  
  // Set OutPort buffer
  
  // Set service provider to Ports
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  
  // </rtc-template>

  m_playerMalletXY_in.data.length(2);
  m_robotMalletXY_in.data.length(2);
  m_puckXY_in.data.length(2);

  m_playerMalletXY_in.data[0] = m_playerMalletXY_in.data[1] = 0.0;
  m_robotMalletXY_in.data[0]  = m_robotMalletXY_in.data[1]  = 0.0;
  m_puckXY_in.data[0]         = m_puckXY_in.data[1]         = 0.0;

  return RTC::RTC_OK;
}


RTC::ReturnCode_t AHLogger::onFinalize()
{
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t AHLogger::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHLogger::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

RTC::ReturnCode_t AHLogger::onActivated(RTC::UniqueId ec_id)
{
    static int numLog = 0;
    const char basename[] = "./HkyLogs";
    char filename[256]; memset(filename, 0, sizeof(char)*256);

    sprintf(filename, "%s/HkyLogPMallet%03d.log", basename, numLog);
    if (NULL==fpPMlt) fpPMlt=fopen(filename, "w");

    sprintf(filename, "%s/HkyLogRMallet%03d.log", basename, numLog);
    if (NULL==fpRMlt) fpRMlt=fopen(filename, "w");

    sprintf(filename, "%s/HkyLogPuck%03d.log", basename, numLog);
    if (NULL==fpPuck) fpPuck=fopen(filename,    "w");

    numLog++;

    // Wait for new data
    while (true) {
        if (m_playerMalletXY_inIn.isNew() ||
            m_robotMalletXY_inIn.isNew()  ||
            m_puckXY_inIn.isNew()         )
        {
            break;
        }
    }
    beginTime = gettimeofday_sec();

    writePMallet(m_playerMalletXY_inIn, m_playerMalletXY_in);
    writeRMallet(m_robotMalletXY_inIn,  m_robotMalletXY_in);
    writePuck   (m_puckXY_inIn,         m_puckXY_in);

    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHLogger::onDeactivated(RTC::UniqueId ec_id)
{
    if (fpPMlt) { fclose(fpPMlt); fpPMlt=NULL; }
    if (fpRMlt) { fclose(fpRMlt); fpRMlt=NULL; }
    if (fpPuck) { fclose(fpPuck); fpPuck=NULL; }

    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHLogger::onExecute(RTC::UniqueId ec_id)
{
    writePMallet(m_playerMalletXY_inIn, m_playerMalletXY_in);
    writeRMallet(m_robotMalletXY_inIn,  m_robotMalletXY_in);
    writePuck   (m_puckXY_inIn,         m_puckXY_in);

    return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t AHLogger::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHLogger::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHLogger::onReset(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHLogger::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t AHLogger::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/



extern "C"
{
 
  void AHLoggerInit(RTC::Manager* manager)
  {
    coil::Properties profile(ahlogger_spec);
    manager->registerFactory(profile,
                             RTC::Create<AHLogger>,
                             RTC::Delete<AHLogger>);
  }
  
};


