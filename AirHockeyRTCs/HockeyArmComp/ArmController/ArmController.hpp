#ifndef ARM_CONTROLLER_HPP_B47671DE_5002_480F_8553_3ABC3D15E8D0
#define ARM_CONTROLLER_HPP_B47671DE_5002_480F_8553_3ABC3D15E8D0

#include "MotorController.hpp"


/* ArmPosition : アームの手先座標を格納する構造体 */
typedef struct {
    double x; // X座標[mm]
    double y; // Y座標[mm]
} ArmPosition;


/* ArmAngles : アームの関節角度を格納する構造体 */
typedef struct {
    double th1; // 肩(X)の角度[rad]
    double th2; // 肘(Y)の角度[rad]
} ArmAngles;


/* ArmController : アームを制御するクラス */
class ArmController {
private: // クラス固有
    bool isInitialized_; // 初期化が完了したか
private: // モーター制御クラス
    MotorController *motor1_; // 肩(X)軸モーター
    MotorController *motor2_; // 肘(Y)軸モーター
private: // アーム固有の定数
    double armLen1_;    // 長さ(肩-肘)[mm]
    double armLen2_;    // 長さ(肘-手先)[mm]
    bool   isRightArm_; // 右手側か

public:
    ArmController(MotorController *motor1, MotorController *motor2,
                  double armLen1, double armLen2, bool isRightArm);
    ~ArmController();

//private: // 計算
public:
    ArmPosition getFK(const ArmAngles &angles); // FKを求める
    ArmAngles   getIK(const ArmPosition &pos);  // IKを求める

public: // 制御処理
    bool start(); // アーム制御を開始する
    bool stop();  // アーム制御を終了する
    bool moveTo(const ArmPosition &pos); // アーム手先を移動する
    ArmPosition getArmPos(); // 現在のアーム手先座標を取得する
};

#endif // ARM_CONTROLLER_HPP_B47671DE_5002_480F_8553_3ABC3D15E8D0
