/* ----------------------------------------------------
 * MotorController.hpp
 *
 * <概要>
 * モーター制御の仮想クラス(MotorController)の宣言。
 * モーター制御処理は本クラスに基づいて継承し作成する。
 * 作成したクラスはArmControllerクラスで使用される。
 *
 * Copyright (c) 2012-2013 ysatoh
 *
 * This code is licensed under the MIT License.
 *
 * ---------------------------------------------------- */

#ifndef MOTOR_CONTROLLER_HPP_BAF7C809_2EBD_48C2_BC73_D7284E6573AE
#define MOTOR_CONTROLLER_HPP_BAF7C809_2EBD_48C2_BC73_D7284E6573AE

class MotorController {
public:
    virtual bool start() = 0; // モーター制御を開始する
    virtual bool stop()  = 0; // モーター制御を停止する
    virtual bool moveTo(double rad)    = 0; // アームを指定角度[rad]まで回転する
    virtual double getArmAngle()       = 0; // アームの現在の角度[rad]を取得する
    virtual bool setArmRPM(double rpm) = 0; // アームの回転数[rpm]を設定する
    virtual bool setAccelArmRPM(double acrpm) = 0; // アームの加速回転数[rpm/sec]を設定する

public:
    MotorController();
    virtual ~MotorController();
};

#endif // MOTOR_CONTROLLER_HPP_BAF7C809_2EBD_48C2_BC73_D7284E6573AE
