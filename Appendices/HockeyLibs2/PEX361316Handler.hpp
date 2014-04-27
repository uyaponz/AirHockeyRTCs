#ifndef PEX361316_HANDLER_HPP_22C50099_D52A_47AD_99EF_BD162E2C80F0
#define PEX361316_HANDLER_HPP_22C50099_D52A_47AD_99EF_BD162E2C80F0

#include "MotorHandlerBase.hpp"

#include <boost/thread.hpp>


class PEX361316Handler : public MotorHandlerBase
{
private: // PEX-361316関連
    int          devID;     // デバイス番号
    unsigned int devOpened; // 開いたポート(下位3ビットを利用、DA:4,AD:2,Penc:1)
private: // モーター固有パラメータ
    signed long evalN; // 逓倍数
    double motorRPM6V; // 6V出力(モータードライバ許容限界)時のモーター回転数[rpm]
private: // 指令値
    signed long targetCount; // 回転先のエンコーダ値
    double      motorRPM;    // モーター回転数[rpm]
    signed long encPulse;    // モーター1回転あたりのパルス数
    signed long slowCount;   // 減速しはじめるパルス数の差
private: // 状態
    bool inited;   // 初期化が完了したか
    bool pausing;  // 一時停止中か
    bool moved;    // 指定した位置まで移動したか
    bool running;  // スレッドが動作中か
    bool stopping; // スレッド停止要求
private: // その他
    boost::thread *thre; // スレッドハンドル

private:
    // モーターへの指令値をセットする
    signed long moveMotor(signed long _target, double _rpm=-1.0, bool _disablePause=false);
public:
    void setSlowCount(signed long _sc);

/* --- pure virtual functions on the MotorHandlerBase --- */
private:
    void run();
public:
    void start();
    void stop();
    void pause();
    void resume();
public: // setter
    void moveTo(signed long _target);
    void setMotorRPM(double _rpm);
public: // getter
    signed long getEncoderPosition();
    signed long getPulsePerRev();
    bool isMoved();
    bool isInitialized();

public:
    PEX361316Handler(int         _deviceID,     // デバイス番号
                     signed long _encoderPulse, // モーター1回転のパルス数
                     signed long _evaluateN,    // 逓倍数
                     double      _rpm6v);       // 6Vのときの回転数
    ~PEX361316Handler();
};


#endif // PEX361316_HANDLER_HPP
