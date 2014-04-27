// -*- C++ -*-
/*!
 * @file  AHAttack.cpp
 * @brief ModuleDescription
 * @date $Date$
 *
 * $Id$
 */

#include "AHAttack.h"

#include "../HockeyLibs2/HockeyDefinitions.hpp"
#include "../HockeyLibs2/MyMath.hpp"

#include <deque>
#include <cmath>

// Module specification
// <rtc-template block="module_spec">
static const char* ahattack_spec[] =
{
    "implementation_id", "AHAttack",
    "type_name",         "AHAttack",
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
AHAttack::AHAttack(RTC::Manager* manager)
    // <rtc-template block="initializer">
    : RTC::DataFlowComponentBase(manager),
      m_puckXYIn("PuckXY", m_puckXY),
      m_realArmXYIn("RealArmXY", m_realArmXY),
      m_armXYOut("ArmXY", m_armXY)

      // </rtc-template>
{
}

/*!
 * @brief destructor
 */
AHAttack::~AHAttack()
{
}



RTC::ReturnCode_t AHAttack::onInitialize()
{
    // Registration: InPort/OutPort/Service
    // <rtc-template block="registration">
    // Set InPort buffers
    addInPort("PuckXY", m_puckXYIn);
    addInPort("RealArmXY", m_realArmXYIn);
  
    // Set OutPort buffer
    addOutPort("ArmXY", m_armXYOut);
  
    // Set service provider to Ports
  
    // Set service consumers to Ports
  
    // Set CORBA Service Ports
  
    // </rtc-template>

    // 入力の初期化
    m_puckXY.data.length(0);
    m_realArmXY.data.length(0);

    // 出力の初期化
    m_armXY.data.length(2);
    m_armXY.data[0] = 0.0;
    m_armXY.data[1] = ARM_BASE_Y - HKY_H/2.0;
    m_armXYOut.write();

    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHAttack::onFinalize()
{
    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t AHAttack::onStartup(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHAttack::onShutdown(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/


RTC::ReturnCode_t AHAttack::onActivated(RTC::UniqueId ec_id)
{
    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHAttack::onDeactivated(RTC::UniqueId ec_id)
{
    return RTC::RTC_OK;
}

static int getProcNum(double puckY) {
    static const double min0 = HKY_H / 2.0 + 400.0 + 200.0;
    static const double min1 = HKY_H / 2.0 + 400.0;
    static const double min2 = HKY_H / 2.0;
    static const double min3 = ARM_BASE_Y + 600.0;
    static const double min4 = 0.0;

    if (min0 <= puckY) return 0;
    if (min1 <= puckY) return 1;
    if (min2 <= puckY) return 2;
    if (min3 <= puckY) return 3;
    if (min4 <= puckY) return 4;

    return 4;
}

RTC::ReturnCode_t AHAttack::onExecute(RTC::UniqueId ec_id)
{
    if (!m_puckXYIn.isNew() && !m_realArmXYIn.isNew()) return RTC::RTC_OK;
    m_puckXYIn.read();
    m_realArmXYIn.read();
    if (m_puckXY.data.length()==0 || m_realArmXY.data.length()==0) return RTC::RTC_OK;

    double realArmX = m_realArmXY.data[0];
    double realArmY = m_realArmXY.data[1] + HKY_H/2.0;

    int procCnt = 0;

    // パックの軌道を記憶する
    static std::deque<MyPos> recentPos;
    double x = m_puckXY.data[0];
    double y = m_puckXY.data[1] + HKY_H/2.0;
    if (recentPos.size() <= 0) { // データを1つも記憶していない場合は記憶して終了
        addDeque<MyPos>(recentPos, myPos(x,y), MEM_MAX_NUM);
        return RTC::RTC_OK;
    }
    else {
        // 近い場合は記憶しない
        MyPos front = recentPos.front();
        double dist = pow(x - front.x, 2) + pow(y - front.y, 2);
        double minR = MEM_MIN_R * MEM_MIN_R;
//        double maxR = MEM_MAX_R * MEM_MAX_R;
        if (minR < dist) {// && dist < maxR) {
            addDeque<MyPos>(recentPos, myPos(x,y), MEM_MAX_NUM);
        }
    }

    // 領域 1〜4 のどこにいるのかを推定する(あとできれいに書きなおす)
    static int prevProc = 0;
    int nowProc = getProcNum(y);
    if      (prevProc==0 && nowProc==1) nowProc = 1; // 0 -> 1 = 1
    else if (prevProc==1 && nowProc==2) nowProc = 2; // 1 -> 2 = 2
    else if (prevProc==2 && nowProc==3) nowProc = 3; // 2 -> 3 = 3
    else if (prevProc==3 && nowProc==4) nowProc = 4; // 3 -> 4 = 4
    else if (prevProc==3 && nowProc==0) nowProc = 0; // 3 -> 0 = 0
    else if (prevProc==4 && nowProc==0) nowProc = 0; // 4 -> 0 = 0
    else                                nowProc = prevProc;

    static bool  didShot   = false;
//    if (didShot) {
//        if ((y-HKY_H/2.0) >= 50.0) {
//            m_armXY.data[0] = 0.0;
//            m_armXY.data[1] = ARM_BASE_Y - HKY_H/2.0;
//            m_armXYOut.write();
//            prevProc = 0;
//            nowProc = 0;
//            didShot = false;
//        }
//    }

    // 2 or 3 : 軌道から直線を求める(XとYを逆にして計算する)
    static MyPos prevSlope = myPos(0.0, 0.0);
    static bool  doShot    = false;
    if (nowProc==2 || nowProc==3) {
        procCnt = 0;
        MyPos lineAB = calcSlope(recentPos, USE_MEM_NUM, true);
        if (lineAB.x > 0 || lineAB.x < 0) {
            double A=lineAB.x, B=lineAB.y;
            double armX = A * ARM_BASE_Y + B;

            // 出力に書き込む
            if (-670.0 < armX && armX < 670.0) {
                m_armXY.data[0] = armX;
                m_armXY.data[1] = ARM_BASE_Y - HKY_H/2.0;
                doShot = true;
                m_armXYOut.write();
                prevSlope = lineAB;
            }
        }
        else {
            doShot = false;
        }
    }

    // 3 -> 4 : 打ちに行く
    if ((nowProc==3 || nowProc==4) && doShot) {
        double distPM = sqrt(pow(realArmX-x, 2) + pow(realArmY-y, 2));
        if (distPM <= 550.0) {
            double baseX = m_armXY.data[0];
            double baseY = m_armXY.data[1];
            while (true) {
                procCnt += 1;
                double A  = prevSlope.x;
                double th = -atan(A) + M_PI/2.0;
                double l  = 500.0 * (double)procCnt / 100.0;
                if (l >= 500.0) {
//                    m_armXY.data[0] = 0.0;
//                    m_armXY.data[1] = ARM_BASE_Y - HKY_H/2.0;
//                    m_armXYOut.write();
                    break;
                }
                if (l <= 0.0)   l = 0.0;
                double vecX = l * cos(th);
                double vecY = l * sin(th);
                m_armXY.data[0] = baseX + vecX;
                m_armXY.data[1] = baseY + vecY;
                m_armXYOut.write();

                usleep(2750);
            }
            doShot  = false;
            didShot = true;
        }
    }
    // (3 or 4) -> 0 : 戻す
    else if ((prevProc==3 || prevProc==4) && nowProc==0) {
        m_armXY.data[0] = 0.0;
        m_armXY.data[1] = ARM_BASE_Y - HKY_H/2.0;
        m_armXYOut.write();
        doShot  = false;
        didShot = false;
    }

    // 状態番号を更新する
    prevProc = nowProc;

    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t AHAttack::onAborting(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHAttack::onError(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/


RTC::ReturnCode_t AHAttack::onReset(RTC::UniqueId ec_id)
{
    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t AHAttack::onStateUpdate(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHAttack::onRateChanged(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/



extern "C"
{
 
    void AHAttackInit(RTC::Manager* manager)
    {
        coil::Properties profile(ahattack_spec);
        manager->registerFactory(profile,
                                 RTC::Create<AHAttack>,
                                 RTC::Delete<AHAttack>);
    }
  
};


