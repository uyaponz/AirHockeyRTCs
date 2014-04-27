// -*- C++ -*-
/*!
 * @file  HockeyArm.cpp
 * @brief ${rtcParam.description}
 * @date $Date$
 *
 * $Id$
 */

#include "HockeyArm.h"

#include <cmath>
#include "ArmController/ArmController.hpp"
#include "ArmController/MC_PEX361316.hpp"

static const double HKY_W = 1407.75;
static const double HKY_H = 2288.975;

static const double ARM_OFFSET = 211.0125;
static const double ARM_LEN1   = 558.5;
static const double ARM_LEN2   = 319.45;
static const double ARM_MLT_D  = 100.0;

// Module specification
// <rtc-template block="module_spec">
static const char* hockeyarm_spec[] =
{
    "implementation_id", "HockeyArm",
    "type_name",         "HockeyArm",
    "description",       "${rtcParam.description}",
    "version",           "0.0.1",
    "vendor",            "ysatoh",
    "category",          "Category",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    ""
};
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
HockeyArm::HockeyArm(RTC::Manager* manager)
    // <rtc-template block="initializer">
    : RTC::DataFlowComponentBase(manager),
      m_armXY_inIn("ArmXY_in", m_armXY_in),
      m_armXY_outOut("ArmXY_out", m_armXY_out),
      m_AHServicePort("AHService"),

      // </rtc-template>
      mp_x_(0), mp_y_(0), ac_(0)
{
    double rpm6v = 3000.0;
    mp_x_ = new MC_PEX361316(1,        // デバイス番号
                             2500,     // モーター1回転あたりのカウント数
                             1,        // 逓倍数
                             rpm6v,    // 6V流したときのモーター回転数[rpm]
                             25,       // 減速比
                             225,      // 0度モーターカウント数
                             M_PI/2,   // 0度の角度[rad]
                             rpm6v/25, // アーム最高回転数[rpm]
                             500.0,    // アーム加速度[rpm/sec]
                             500.0);   // アーム減速度[rpm/sec]
//                             440.0,    // アーム加速度[rpm/sec]
//                             440.0);   // アーム減速度[rpm/sec]

    mp_y_ = new MC_PEX361316(2,        // デバイス番号
                             2500,     // モーター1回転あたりのカウント数
                             1,        // 逓倍数
                             rpm6v,    // 6V流したときのモーター回転数[rpm]
                             15,       // 減速比
                             1000,     // 0度モーターカウント数
                             0.0,      // 0度の角度[rad]
                             rpm6v/15, // アーム最高回転数[rpm]
                             750.0,    // アーム加速度[rpm/sec]
                             750.0);   // アーム減速度[rpm/sec]
//                             520.0,    // アーム加速度[rpm/sec]
//                             520.0);   // アーム減速度[rpm/sec]

    ac_ = new ArmController(mp_x_, mp_y_, // MotorController
                            ARM_LEN1,     // 肩-肘の長さ[mm]
                            ARM_LEN2,     // 肘-手先の長さ[mm]
                            true);        // 右腕？

    ac_->start();

    ArmPosition pos = {0.0, 400.0};
    ac_->moveTo(pos);
}

/*!
 * @brief destructor
 */
HockeyArm::~HockeyArm()
{
    ac_->stop();
    delete ac_;
    delete mp_x_;
    delete mp_y_;
}



RTC::ReturnCode_t HockeyArm::onInitialize()
{
    // Registration: InPort/OutPort/Service
    // <rtc-template block="registration">
    // Set InPort buffers
    addInPort("ArmXY_in", m_armXY_inIn);

    // Set OutPort buffer
    addOutPort("ArmXY_out", m_armXY_outOut);

    // Set service provider to Ports

    // Set service consumers to Ports
    m_AHServicePort.registerConsumer("AHCommonDataService", "AHCommon::AHCommonDataService", m_ahCommonDataService);

    // Set CORBA Service Ports
    addPort(m_AHServicePort);

    // </rtc-template>

    // 入力の初期化
    m_armXY_in.data.length(2);
    m_armXY_in.data[0] = 0.0;
    m_armXY_in.data[1] = -HKY_H/2.0 - ARM_OFFSET + 400.0;
    // 出力の初期化
    m_armXY_out.data.length(2);
    m_armXY_out.data[0] = 0.0;
    m_armXY_out.data[1] = -HKY_H/2.0 - ARM_OFFSET + 400.0;
    m_armXY_outOut.write();

    return RTC::RTC_OK;
}


RTC::ReturnCode_t HockeyArm::onFinalize()
{
    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t HockeyArm::onStartup(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t HockeyArm::onShutdown(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/


RTC::ReturnCode_t HockeyArm::onActivated(RTC::UniqueId ec_id)
{
    return RTC::RTC_OK;
}


RTC::ReturnCode_t HockeyArm::onDeactivated(RTC::UniqueId ec_id)
{
    return RTC::RTC_OK;
}


bool isArmMovable(ArmPosition ap)
{
    double x = ap.x;
    double y = ap.y;

    if ((-HKY_W/2.0+ARM_MLT_D/2.0 < x&&x < HKY_W/2.0-ARM_MLT_D/2.0) &&
        (-HKY_H/2.0+ARM_MLT_D/2.0 < y&&y < 0.0))
    {
        return true;
    }
    return false;
}

RTC::ReturnCode_t HockeyArm::onExecute(RTC::UniqueId ec_id)
{
    /* --- アームの移動処理 --- */
    if (m_armXY_inIn.isNew()) {
        m_armXY_inIn.read();
        ArmPosition ap = {m_armXY_in.data[0], m_armXY_in.data[1]};
        if (isArmMovable(ap)) {
            ap.y += HKY_H/2.0 + ARM_OFFSET;
            ac_->moveTo(ap);
        }
    }

    /* --- アームの現在地取得処理 --- */
    ArmPosition pos = ac_->getArmPos();
    pos.y -= HKY_H/2.0 + ARM_OFFSET;
    m_armXY_out.data[0] = pos.x;
    m_armXY_out.data[1] = pos.y;
    m_armXY_outOut.write();

    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t HockeyArm::onAborting(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t HockeyArm::onError(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t HockeyArm::onReset(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t HockeyArm::onStateUpdate(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t HockeyArm::onRateChanged(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/



extern "C"
{

    void HockeyArmInit(RTC::Manager* manager)
    {
        coil::Properties profile(hockeyarm_spec);
        manager->registerFactory(profile,
                                 RTC::Create<HockeyArm>,
                                 RTC::Delete<HockeyArm>);
    }

};


