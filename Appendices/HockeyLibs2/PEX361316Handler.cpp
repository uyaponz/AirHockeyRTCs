#include "PEX361316Handler.hpp"

#include "PEX361316Func.hpp"
using namespace PEX361316Func;

#include <iostream>
#include <cstring>

#include <fbida.h>
#include <fbiad.h>
#include <fbipenc.h>


/* モーターの回転を停止する */
void stopMotor(int _devID)
{
    // 出力電圧を0に設定する
    unsigned short vZero = getVoltageDA_10V(0.0);
    DaOutputDAEx(_devID, &vZero);
}


/* ### コンストラクタ ### */
PEX361316Handler::PEX361316Handler(int         _deviceID,     // デバイス番号
                                   signed long _encoderPulse, // モーター1回転のパルス数
                                   signed long _evaluateN,    // 逓倍数
                                   double      _rpm6v)        // 6Vのときの回転数
    :
    devID(_deviceID), devOpened(0),
    evalN(_evaluateN), motorRPM6V(_rpm6v),
    targetCount(0), motorRPM(100.0), encPulse(_encoderPulse), slowCount(10000),
    inited(false), pausing(false), moved(true), running(false), stopping(true),
    thre(NULL)
{
    /* --- 変数の異常値チェック、デバイスオープン --- */
    if ( (devID <= 0)                                 ||
         (motorRPM6V < 1000.0 || motorRPM6V > 5000.0) ||
         (encPulse <= 0)                              ||
         (!isValidEvaluateNum(evalN))                 ) // (1,2,4)以外の場合
    {
        std::cerr <<  "Handler() - invalid value" << std::endl;
        return;
    }

    devOpened |= (!openDeviceDA(devID)) << 2; // DA  : bit2
    devOpened |= (!openDevicePenc(devID));    // Penc: bit0

    if (0x05 != devOpened) { // bit2 + bit0 = 4 + 1 = 0x05
        std::cerr <<  "Handler() - cannot open devices" << std::endl;
        return;
    }

    /* --- アナログ入出力ボードの設定 --- */
    { // D/Aの使用準備
        DASMPLCHREQ daSmplChReq;
        daSmplChReq.ulChNo  = 1;      // D/Aのチャンネル番号
        daSmplChReq.ulRange = DA_10V; // 出力レンジ(±10[V])
        if (DaSetOutputDAEx(devID, 1, &daSmplChReq) != DA_ERROR_SUCCESS) {
            std::cerr << "Handler() - DaSetOutputDAEx() error" << std::endl;
            return;
        }
        stopMotor(devID);
    }

    { // Pencの使用準備(0x04:位相差パルスカウントモード,非同期クリア)
        // 動作モードの設定
        int evFlag = static_cast<int>(log2(evalN)); // 逓倍数1,2,4 -> 設定値0,1,2
        if (PencSetMode(devID, 1, 0x04+evFlag, 0, 0, 0) != PENC_ERROR_SUCCESS) {
            std::cerr << "Handler() - PencSetMode() error" << std::endl;
            return;
        }

        // Z相の設定
        if (PencSetZMode(devID, 1, 0x00) != PENC_ERROR_SUCCESS) {
            std::cerr << "Handler() - PencSetZMode() error" << std::endl;
            return;
        }
    }

    isInited = true;
}


/* ### デストラクタ ### */
PEX361316Handler::~PEX361316Handler()
{
    stop();
    stopMotor(devID);

    /* --- デバイスクローズ --- */
    if (0x04 & devOpened) closeDeviceDA(devID);
    if (0x01 & devOpened) closeDevicePenc(devID);
}


/* モーターへの指令値をセットする */
signed long PEX361316Handler::moveMotor(signed long _target, double _rpm, bool _disablePause)
{
    while (!stopping) {
        // 一時停止の要求がきたらprevCountを現在値に更新して抜ける
        if (isPausing && !disablePause) {
            return getEncoderPosition();
        }

        // 現在のエンコーダカウント値を取得する
        signed long nowCount = getEncoderPosition();
        // 回転方向(オーバー/アンダーフローしない前提)
        int dir = (nowCount <= target)*2 - 1;
        // 指定位置に到達していたら抜ける
        if ( (dir==1 && nowCount>=target) || (dir==-1 && nowCount<=target) ) {
            m_isMoved = true;
            usleep(10);
            return target;
        }

        // 回転させる
        double v = rpm2voltage((rpm>0.0)?rpm:motorRPM, motorRPM6V);

        double diffMax = slowCount;
        double diff = fabs(target - nowCount);
        double vMin = rpm2voltage(10, motorRPM6V) / v;
        double c = ((1.0 - vMin) / diffMax) * diff + vMin;

        unsigned short vOut = getVoltageDA_10V(dir * std::min(v, v*c));
        DaOutputDAEx(devID, &vOut);

        usleep(10);
    }

    return getEncoderPosition();
}

void PEX361316Handler::setSlowCount(signed long _sc)
{
    if (_sc >= 0) slowCount = _sc;
}

/* ### モーター制御スレッド本体 ### */
void PEX361316Handler::run()
{
    isRunning = true;
    std::cerr << "PEX361316Handler start!!!" << std::endl;

    signed long prevCount = 0;
    while (!isStopping) {
        if (!isPausing) {
            if (prevCount != targetCount) {
                m_isMoved = false;
                prevCount = moveMotor(targetCount);
                continue;
            }
        }

        // サーボロックのみ
        moveMotor(prevCount, 100.0, true);
    }

    std::cerr << "PEX361316Handler end!!!" << std::endl;
    isRunning = false;
}

/* ### スレッド起動 ### */
void PEX361316Handler::start()
{
    if (!isInited || isRunning || thre!=NULL) return;

    moveTo(0);
    resume();

    isStopping = false;
    thre = new boost::thread(boost::bind(&PEX361316Handler::run, this));
    if (NULL == thre) isStopping = true;
}

/* ### スレッド終了 ### */
void PEX361316Handler::stop()
{
    if (!isInited || !isRunning || thre==NULL) return;

    isStopping = true;
    thre->join();
    delete thre; thre=NULL;
}

/* ### 一時停止 ### */
void PEX361316Handler::pause()
{
    isPausing = true;
}

/* ### 再開 ### */
void PEX361316Handler::resume()
{
    isPausing = false;
}

/* ### 指定したエンコーダ値まで回転する ### */
void PEX361316Handler::moveTo(signed long target)
{
    if (target != targetCount) {
        targetCount = target;
        m_isMoved = false;
    }
}

/* ### モーターの回転数を設定する ### */
void PEX361316Handler::setMotorRPM(double rpm)
{
    if (0.0 <= rpm && rpm <= 5000.0)
        motorRPM = rpm;
}

/* ### 現在の実際のエンコーダ値を返す ### */
signed long PEX361316Handler::getEncoderPosition()
{
    if (!isInited) return 0;

    signed long retVal;
    PencGetCounter(devID, 1, reinterpret_cast<unsigned long*>(&retVal));
    return retVal;
}

/* ### モーター1回転あたりのパルス数を返す ### */
signed long PEX361316Handler::getPulsePerRev()
{
    return encPulse * evalN; // エンコーダのパルス数 * 逓倍数
}

/* ### 移動が完了したか ### */
bool PEX361316Handler::isMoved()
{
    return m_isMoved;
}

/* ### 初期化が正常に完了したか ### */
bool PEX361316Handler::isInitialized()
{
    return isInited;
}
