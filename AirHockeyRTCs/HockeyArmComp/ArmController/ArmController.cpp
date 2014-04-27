#include "ArmController.hpp"

#include "MotorController.hpp"

#include <cmath>


ArmController::ArmController(MotorController *motor1, MotorController *motor2,
                             double armLen1, double armLen2, bool isRightArm) :
    isInitialized_(false),
    motor1_(motor1), motor2_(motor2),
    armLen1_(armLen1), armLen2_(armLen2),
    isRightArm_(isRightArm)
{
    /* --- 引数の確認 --- */
    if (motor1 == 0  ||  motor2 == 0) return;
    if (armLen1 <= 0.0  ||  armLen2 <= 0.0) return;

    isInitialized_ = true;
}


ArmController::~ArmController()
{
    stop();
}


/* getFK() : FKを求める */
ArmPosition ArmController::getFK(const ArmAngles &angles)
{
    /* --- 変数を置き換える --- */
    double th1 = angles.th1;
    double th2 = angles.th2;
    double l1  = armLen1_;
    double l2  = armLen2_;

    /* --- FKの計算 --- */
    ArmPosition retval;
    retval.x = (l1 * cos(th1)) + (l2 * cos(th1+th2));
    retval.y = (l1 * sin(th1)) + (l2 * sin(th1+th2));

    return retval;
}


/* getIK() : IKを求める */
ArmAngles ArmController::getIK(const ArmPosition &pos)
{
    /* --- 変数を置き換える --- */
    double x  = pos.x;
    double y  = pos.y;
    double l1 = armLen1_;
    double l2 = armLen2_;

    /* --- 事前の計算 --- */
    double xxyy = (x*x + y*y); // x^2 + y^2
    double l1_2 = l1 * l1;     // l1^2
    double l2_2 = l2 * l2;     // l2^2

    /* --- 不可能な位置を示されていたら例外を投げる --- */
    double maxL = (l1 + l2) * 0.95;
    double minL = fabs(l1 - l2) * 1.05;
    double dist = sqrt(xxyy);
    if (!(minL < dist  &&  dist < maxL)) {
        throw "cannot calculate angles.\n";
    }

    /* --- IKの計算 --- */
    ArmAngles retval;
    double a = (-xxyy + l1_2 + l2_2) / (2.0 * l1 * l2);
    double b = ( xxyy + l1_2 - l2_2) / (2.0 * l1 * sqrt(xxyy));
    a = acos(a);
    b = acos(b);

    if (isRightArm_) {
        retval.th1 = atan2(y,x) - b;
        retval.th2 = M_PI - a;
    }
    else {
        retval.th1 = atan2(y,x) + b;
        retval.th2 = a - M_PI;
    }

    return retval;
}


/* start() : アーム制御を開始する */
bool ArmController::start()
{
    if (!motor1_->start() || motor2_->start()) {
        stop();
        return false;
    }
    return true;
}


/* stop() : アーム制御を終了する */
bool ArmController::stop()
{
    bool m1 = motor1_->stop();
    bool m2 = motor2_->stop();
    return (m1 && m2) ? true : false;
}


/* moveTo() : アーム手先を移動する */
bool ArmController::moveTo(const ArmPosition &pos)
{
    ArmAngles angles;
    try {
        angles = getIK(pos);
    } catch(...) { return false; }

    motor1_->moveTo(angles.th1);
    motor2_->moveTo(angles.th2);

    return true;
}


/* getArmPos() : 現在のアーム手先座標を取得する */
ArmPosition ArmController::getArmPos()
{
    ArmAngles angles = {motor1_->getArmAngle(),
                        motor2_->getArmAngle()};
    return getFK(angles);
}
