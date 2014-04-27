#include "MC_PEX361316.hpp"

#include <iostream>
#include <unistd.h>
#include <cmath>
#include <time.h>
#include <sys/time.h>

#include <fbiad.h>   // GPG-3100 (A/D in, Digital in/out)
#include <fbida.h>   // GPG-3300 (D/A out)
#include <fbipenc.h> // GPG-6204 (Penc)


enum {
    AD   = 0x01,
    DA   = 0x02,
    Penc = 0x04,
};


/* 回転方向の計算 */
inline int getDirection(double x) { return (x >= 0.0) ? +1 : -1; }

/* 現在時刻[sec]を取得する */
inline double getTimeOfDay()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec
         + tv.tv_usec * 1e-6;
}


MC_PEX361316::MC_PEX361316(int devNo,              // PEX-361316のデバイス番号
                           signed long enc1rot,    // モーター1回転あたりのカウント数(逓倍は加味しない)
                           int evaluate,           // 逓倍数
                           double motorRPM6V,      // 6V流したときの回転数[rpm]
                           int ratio,              // モーター -> アーム の減速比
                           signed long zeroOffset, // Z=0からのオフセットカウント数
                           double zeroAngle,       // 初期アーム角度[rad]
                           double targetArmRPM,    // 目標アーム回転数[rpm]
                           double accelArmRPM,     // アーム加速度[rpm/sec]
                           double decelArmRPM) :   // アーム減速度[rpm/sec]
    /* --- 初期化関連 --- */
    isInitialized_(false),
    opened_(0),
    /* --- スレッド関連 --- */
    isRunning_(false),
    isStopping_(true),
    thread_(0),
    /* --- 指令値/現在値 --- */
    targetAngle_(zeroAngle),
    targetArmRPM_(targetArmRPM),
    accelArmRPM_(accelArmRPM),
    decelArmRPM_(decelArmRPM),
    armAngle_(zeroAngle),
    armRPM_(0.0),
    /* --- 設定値 --- */
    devNo_(devNo),
    ev_(evaluate),
    motorRPM6V_(motorRPM6V),
    enc1rot_(enc1rot),
    ratio_(ratio),
    zeroOffset_(zeroOffset),
    zeroAngle_(zeroAngle)
{
    /* --- 引数の確認 --- */
    if (devNo < 0)       return;
    if (evaluate!=1 && evaluate!=2 && evaluate!=4) return;
    if (motorRPM6V <= 0) return;
    if (enc1rot <= 0)    return;
    if (ratio <= 0)      return;
    if (zeroOffset < 0)  return;
    if (targetArmRPM <= 0.0  ||  accelArmRPM <= 0.0  ||  decelArmRPM <= 0.0) return;

    /* 各種初期化処理 */
    if (!openDevices()) { closeDevices(); return; }       // デバイス初期化
    if (!clearEncoderCount()) { closeDevices(); return; } // エンコーダの値を初期化する

    isInitialized_ = true;
}


MC_PEX361316::~MC_PEX361316()
{
    /* 解放処理 */
    stop();         // スレッドの停止
    closeDevices(); // デバイスクローズ
}


/* openDevices() : デバイスオープン */
bool MC_PEX361316::openDevices()
{
    if (!(opened_ & DA))   if (!initDA())   return false; else opened_ |= DA;
    if (!(opened_ & Penc)) if (!initPenc()) return false; else opened_ |= Penc;
    return true;
}


/* closeDevices() : デバイスクローズ */
void MC_PEX361316::closeDevices()
{
    if (opened_ & DA)   DaClose(devNo_);
    if (opened_ & Penc) PencClose(devNo_);
    opened_ = 0;
}


/* initDA() : D/Aの初期化 */
bool MC_PEX361316::initDA()
{
    DASMPLCHREQ    daSCR;

    /* --- 初期化 --- */
    if (DaOpen(devNo_)) {
        std::cerr << "D/A - DaOpen() error." << std::endl;
        return false;
    }

    /* --- 出力設定(ch1, バイポーラ±10V) --- */
    daSCR.ulChNo  = 1;
    daSCR.ulRange = DA_10V;
    if (DaSetOutputDAEx(devNo_, 1, &daSCR)) {
        std::cerr << "D/A - DaSetOutputDAEx() error." << std::endl;
        goto deviceClose;
    }

    /* --- 出力を0Vにする --- */
    if (!outputDA(0.0)) goto deviceClose;

    return true;

deviceClose:
    DaClose(devNo_);
    return false;
}


/* initPenc() : Pencの初期化 */
bool MC_PEX361316::initPenc()
{
    /* --- 初期化 --- */
    if (PencOpen(devNo_, PENC_FLAG_NORMAL)) {
        std::cerr << "Penc - PencOpen() error." << std::endl;
        return false;
    }

    /* --- カウントの設定 --- */
    int evFlag = static_cast<int>(log2(ev_));
    if (PencSetMode(devNo_, 1, 0x04+evFlag, 0, 0, 0)) {
        std::cerr << "Penc - PencSetMode() error." << std::endl;
        goto deviceClose;
    }

    /* --- Z相でクリアしない設定にする --- */
    if (!disableZClear()) goto deviceClose;

    return true;

deviceClose:
    PencClose(devNo_);
    return false;
}


/* enableZClear() : Z相でカウンタをクリアする */
bool MC_PEX361316::enableZClear()
{
    if (PencSetZMode(devNo_, 1, 0x01)) {
        std::cerr << "Penc - PencSetZMode error." << std::endl;
        return false;
    }
    return true;
}


/* disableZClear() : Z相でカウンタをクリアしない */
bool MC_PEX361316::disableZClear()
{
    if (PencSetZMode(devNo_, 1, 0x00)) {
        std::cerr << "Penc - PencSetZMode error." << std::endl;
        return false;
    }
    return true;
}


/* outputDA() : 指定した電圧を出力する */
bool MC_PEX361316::outputDA(double volt)
{
    unsigned short value = volt2value(volt);
    if (DaOutputDAEx(devNo_, &value)) {
        std::cerr << "D/A - DaOutputDAEx() error." << std::endl;
        return false;
    }
    return true;
}


/* getCount() : エンコーダカウント値を取得する */
signed long MC_PEX361316::getEncoderCount()
{
    unsigned long count = 0;
    PencGetCounter(devNo_, 1, &count);
    return static_cast<signed long>(count) - zeroOffset_;
}


/* volt2value() : 電圧[V] -> 出力設定値 */
unsigned short MC_PEX361316::volt2value(double volt)
{
    static const double maxV = 10.0; // 入出力ボードの出力限界電圧
    static const unsigned short maxValue = 0xFFFF; // 最大設定値

    if (volt >=  maxV) volt =  maxV;
    if (volt <= -maxV) volt = -maxV;

    return static_cast<unsigned short>((volt+maxV) / ((2.0*maxV)/maxValue));
}


/* armrpm2motorrpm() : アーム回転数[rpm] -> モーター回転数[rpm] */
double MC_PEX361316::armrpm2motorrpm(double armrpm)
{
    return armrpm * ratio_;
}


/* motorrpm2volt() : モーター回転数[rpm] -> 電圧[V] */
double MC_PEX361316::motorrpm2volt(double motorrpm)
{
    return (6.0 * motorrpm) / motorRPM6V_;
}


/* enc2armangle() : カウント値 -> アームの角度[rad] */
double MC_PEX361316::enc2armangle(signed long count)
{
    double baseAngle = (count * 2.0 * M_PI)
                     / (enc1rot_ * ev_ * ratio_);
    return baseAngle + zeroAngle_;
}


/* armangle2enc() : アームの角度[rad] -> カウント値 */
signed long MC_PEX361316::armangle2enc(double armangle)
{
    double baseAngle = armangle - zeroAngle_;
    signed long baseEncCount
        = static_cast<signed long>((baseAngle * enc1rot_ * ev_ * ratio_)
        / (2.0 * M_PI));
    return baseEncCount;
}


/* canStop() : 現在の速度から減速して目標値で止まれるか */
bool MC_PEX361316::canStop()
{
    double v0 = armRPM_;
    double a  = -(decelArmRPM_ * 60.0);
    if (v0 < 0.0) a = -a;

    double t = -(v0 / a);
    double d = v0*t + 0.5*a*t*t;

    double stoppt = armAngle_ + (d * 2.0*M_PI);

    if (v0 >= 0.0) return (stoppt >= targetAngle_) ? false : true;
    else           return (stoppt <= targetAngle_) ? false : true;
}


/* clearEncoderCount() : モーターをゆっくり回転させ、Z相で原点補正する */
bool MC_PEX361316::clearEncoderCount()
{
    if (!enableZClear()) return false; // Z相クリアを有効にする
    {
        /* --- モーターの回転を止めておく --- */
        outputDA(0.0);

        /* --- カウント値にあり得ない値を設定する --- */
        unsigned long initialCount = 2 * enc1rot_ * ev_;
        PencSetCounter(devNo_, 1, initialCount);

        /* --- カウント値がクリアされるまでゆっくり回す --- */
        outputDA(-0.15); // ゆっくり回転
        while (getEncoderCount() >= 0) { usleep(100); }
        outputDA(0.0);  // 停止
    }
    if (!disableZClear()) return false; // Z相クリアを無効にする

    return true;
}


/* run() : モーター制御メインスレッド */
void MC_PEX361316::run()
{
    isRunning_ = true;
    outputDA(0.0); // 停止

    /* --- 初期化 --- */
    armRPM_ = 0.0;

    double prevTime = -1.0;
    while (!isStopping_) {
        /* --- データ集め --- */
        double targetAngle = targetAngle_;
        signed long targetCount = armangle2enc(targetAngle);
        signed long nowCount    = getEncoderCount();
        armAngle_ = enc2armangle(nowCount);
        int direction = getDirection(targetCount - nowCount); // 目標回転方向

        /* モーター制御処理 */
        double nowTime  = getTimeOfDay();
        double diffTime = nowTime - prevTime;
        if (prevTime > 0.0  &&  diffTime > 0.0) {
            if (getDirection(armRPM_) == direction) {
                if (canStop()) armRPM_ += (accelArmRPM_ * diffTime) * direction;
                else           armRPM_ -= ((decelArmRPM_*1.10) * diffTime) * direction;
            }
            else armRPM_ += (accelArmRPM_ * diffTime) * direction;

            if (armRPM_ >=  targetArmRPM_) armRPM_ =  targetArmRPM_;
            if (armRPM_ <= -targetArmRPM_) armRPM_ = -targetArmRPM_;

            double mul = fabs((double)targetCount - (double)nowCount) / 1500.0;
            if (mul >= 1.0)  mul = 1.0;
            if (mul <= 0.05) mul = 0.05;

            outputDA(motorrpm2volt(armrpm2motorrpm(armRPM_ * mul)));
        }

        prevTime = nowTime;
        usleep(1);
    }

    outputDA(0.0); // 停止
    targetAngle_ = enc2armangle(getEncoderCount());

    isRunning_ = false;

}


/* start() : モーター制御を開始する */
bool MC_PEX361316::start()
{
    if (!isInitialized_ || isRunning_ || thread_.get()!=0) return false;

    isStopping_ = false;
    thread_ = std::auto_ptr<boost::thread>
        (new boost::thread(boost::bind(&MC_PEX361316::run, this)));

    return true;
}


/* stop() : モーター制御を停止する */
bool MC_PEX361316::stop()
{
    if (!isInitialized_ || !isRunning_ || thread_.get()==0) return false;

    isStopping_ = true;
    thread_->join();
    delete thread_.release();

    return true;
}


/* moveTo() : アームを指定角度まで回転する */
bool MC_PEX361316::moveTo(double rad)
{
    if (rad-zeroAngle_ >=  M_PI) return false;
    if (rad-zeroAngle_ <= -M_PI) return false;

    targetAngle_ = rad;
    return true;
}


/* getArmAngle() : アームの現在の角度[rad]を取得する */
double MC_PEX361316::getArmAngle()
{
    return armAngle_;
}


/* setArmRPM() : アームの回転数[rpm]を設定する */
bool MC_PEX361316::setArmRPM(double rpm)
{
    if (rpm <= 0.0) return false;
    if (rpm > motorRPM6V_ / ratio_) return false;

    targetArmRPM_ = rpm;
    return true;
}


/* setAccelArmRPM() : アームの加速度[rpm/sec]を設定する */
bool MC_PEX361316::setAccelArmRPM(double acrpm)
{
    if (acrpm <= 0.0) return false;
    
    accelArmRPM_ = acrpm;
    return true;
}


/* setDecelArmRPM() : アームの減速度[rpm/sec]を設定する */
bool MC_PEX361316::setDecelArmRPM(double dcrpm)
{
    if (dcrpm <= 0.0) return false;
    
    decelArmRPM_ = dcrpm;
    return true;
}
