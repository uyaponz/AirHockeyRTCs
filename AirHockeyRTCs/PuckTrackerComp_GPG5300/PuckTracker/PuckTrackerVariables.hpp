#ifndef PUCK_TRACKER_VARIABLES_HPP_7E0A7BE7_6BF4_4721_AEF2_1230A802B3C3
#define PUCK_TRACKER_VARIABLES_HPP_7E0A7BE7_6BF4_4721_AEF2_1230A802B3C3


#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/legacy/legacy.hpp>

#include "PuckTracker.hpp"


struct CameraVariables
{
public:
    /* --- カメラパラメータ関連 --- */
    CvMat *intrinsic;   // 内部パラメータ
    CvMat *rotation;    // 回転
    CvMat *translation; // 並進
    CvMat *distortion;  // 歪み
    bool isReadCameraParameters() const;             // 読み込みが完了しているか
    bool readCameraParameters(std::string filename); // カメラパラメータの読み込み
    void releaseCameraParameters();                  // 解放

    /* --- ホモグラフィ行列関連 --- */
    CvMat *homo;    // ホモグラフィ行列
    CvMat *invHomo; // homoの逆行列
    bool isInitializedHomography() const; // 初期化されているか
    bool initHomography();                // 行列の初期化
    void releaseHomography();             // 解放

public:
    CameraVariables();
    ~CameraVariables();
};
bool isInitialized(const CameraVariables& cam);
bool initialize(CameraVariables& cam, std::string filename);
void release(CameraVariables& cam);


struct LikelihoodVariables
{
public:
    /* --- ConDensationアルゴリズム関連 --- */
    double limitLikelihood;         // 尤度の最低値
    CvConDensation *cond;           // ConDensationの状態
    CvMat *lowerBound, *upperBound; // 位置や速度の最小・最大値
    bool isInitializedConDensation() const; // 初期化されているか
    bool initConDensation(PuckTracker::PuckTrackerParameters& paras); // 初期化
    void releaseConDensation(); // 解放
    void initSamples(PuckTracker::PuckTrackerParameters& paras); // パーティクル位置の初期化

public:
    LikelihoodVariables();
    ~LikelihoodVariables();
};
bool isInitialized(const LikelihoodVariables& like);
bool initialize(LikelihoodVariables& like,
                PuckTracker::PuckTrackerParameters& paras);
void release(LikelihoodVariables& like);


#endif // PUCK_TRACKER_VARIABLES_HPP_7E0A7BE7_6BF4_4721_AEF2_1230A802B3C3
