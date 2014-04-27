#ifndef MOTOR_HANDLER_BASE_HPP_4FF56071_7715_4532_8E1B_E3523F7A6F4D
#define MOTOR_HANDLER_BASE_HPP_4FF56071_7715_4532_8E1B_E3523F7A6F4D

#include <boost/utility.hpp>

class MotorHandlerBase : boost::noncopyable
{
public:
    virtual void start()  = 0; // モーター制御開始
    virtual void stop()   = 0; // モーター制御終了
    virtual void pause()  = 0; // 一時停止
    virtual void resume() = 0; // 再開

public: // setter
    // 指定したエンコーダの値まで回転させる
    virtual void moveTo(signed long target) = 0;
    // モーターの回転数を設定する
    virtual void setRPM(double rpm) = 0;

public: // getter
    // 初期化が正常に完了したか
    virtual bool isInitialized() = 0;
    // 現在のエンコーダの値を返す
    virtual signed long getEncoderPosition() = 0;
    // モーター1回転あたりのエンコーダカウント数を返す
    virtual signed long getPulsePerRev() = 0;
    // 移動が完了したか
    virtual bool isMoved() = 0;

public:
    MotorHandlerBase();
    virtual ~MotorHandlerBase();
};

#endif // MOTOR_HANDLER_BASE_HPP
