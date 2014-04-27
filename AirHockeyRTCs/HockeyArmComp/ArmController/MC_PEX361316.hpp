#ifndef MC_PEX361316_HPP_D353FD6B_1D67_495F_9429_8E664D179632
#define MC_PEX361316_HPP_D353FD6B_1D67_495F_9429_8E664D179632

#include "ArmController.hpp"

#include <boost/thread.hpp>


class MC_PEX361316 : public MotorController {
private: // 初期化関連
    bool isInitialized_; // 初期化が完了しているか
    int  opened_;        // 初期化が完了したデバイス
private: // スレッド関連
    bool isRunning_;  // スレッド(run)が動作中か
    bool isStopping_; // スレッド(run)への停止要求
    std::auto_ptr<boost::thread> thread_; // スレッドオブジェクト
private: // 指令値/現在値
    double targetAngle_;  // 目標アーム角度[rad]
    double targetArmRPM_; // 目標アーム回転数[rpm]
    double accelArmRPM_;  // アームの加速度[rpm/sec]
    double decelArmRPM_;  // アームの減速度[rpm/sec]
    double armAngle_;     // 現在のアーム角度[rad]
    double armRPM_;       // 現在のアーム回転数[rpm]
private: // 設定値
    int devNo_; // PEX-361316のデバイス番号
    int ev_;    // 逓倍数
    double motorRPM6V_;      // 6V印加時のモーター回転数[rpm]
    signed long enc1rot_;    // モーター1回転あたりのエンコーダカウント数
    int ratio_;              // モーターの減速比
    signed long zeroOffset_; // アームがまっすぐになるZ=0基準のカウント数
    double zeroAngle_;       // アームがまっすぐのときのアームの角度[rad]

public:
    explicit MC_PEX361316(int devNo,              // PEX-361316のデバイス番号
                          signed long enc1rot,    // モーター1回転あたりのカウント数(逓倍は加味しない)
                          int evaluate,           // 逓倍数
                          double motorRPM6V,      // 6V流したときの回転数[rpm]
                          int ratio,              // モーター -> アーム の減速比
                          signed long zeroOffset, // Z=0からのオフセットカウント数
                          double zeroAngle,       // 初期アーム角度[rad]
                          double targetArmRPM,    // 目標アーム回転数[rpm]
                          double accelArmRPM,     // アーム加速度[rpm/sec]
                          double decelArmRPM);    // アーム減速度[rpm/sec]
    virtual ~MC_PEX361316();

private: // 初期化・後処理
    bool openDevices();  // デバイスオープン
    void closeDevices(); // デバイスクローズ
    bool initDA();   // D/Aの初期化
    bool initPenc(); // Pencの初期化

private: // 設定
    bool enableZClear();  // Z相でカウンタをクリアする
    bool disableZClear(); // Z相でカウンタをクリアしない

private: // デバイス制御
    bool outputDA(double volt); // 指定した電圧を出力する
    signed long getEncoderCount(); // エンコーダカウント値を取得する

private: // 計算・変換
    unsigned short volt2value(double volt); // 電圧[V] -> 出力設定値
    double armrpm2motorrpm(double armrpm);  // アーム回転数[rpm] -> モーター回転数[rpm]
    double motorrpm2volt(double motorrpm);  // モーター回転数[rpm] -> 電圧[V]
    double enc2armangle(signed long count);    // カウント値 -> アームの角度[rad]
    signed long armangle2enc(double armangle); // アームの角度[rad] -> カウント値
    bool canStop(); // 現在の速度から減速して目標値で止まれるか

private: // 処理
    bool clearEncoderCount(); // モーターをゆっくり回転させ、Z相で原点補正する
    void run();   // モーター制御メインスレッド

public: // 仮想クラスのメンバ関数
    bool start(); // モーター制御を開始する
    bool stop();  // モーター制御を停止する
    bool moveTo(double rad);    // アームを指定角度まで回転する
    double getArmAngle();       // アームの現在の角度[rad]を取得する
    bool setArmRPM(double rpm); // アームの回転数[rpm]を設定する
    bool setAccelArmRPM(double acrpm); // アームの加速度[rpm/sec]を設定する
    bool setDecelArmRPM(double dcrpm); // アームの減速度[rpm/sec]を設定する
};

#endif // MC_PEX361316_HPP_D353FD6B_1D67_495F_9429_8E664D179632
