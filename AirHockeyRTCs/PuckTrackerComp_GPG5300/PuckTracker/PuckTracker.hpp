#ifndef PUCK_TRACKER_HPP_D729B71C_17FC_4FBA_B146_C0BB253DF20F
#define PUCK_TRACKER_HPP_D729B71C_17FC_4FBA_B146_C0BB253DF20F


#include <string>
#include <opencv2/opencv.hpp>
#include <memory>

class PuckTracker
{
public:
    struct Position { double x, y; };
    struct PuckPosition { Position position; double likelihood; };
    struct PuckTrackerParameters
    {
    public:
        std::string calibrationData;     // カメラパラメータのファイルパス
        int captureWidth, captureHeight; // キャプチャ画像のサイズ

        int n_particle; // パーティクルの数
        double p_noise; // 位置のノイズ
        double v_noise; // 速度のノイズ
        double sigma;   // 分散

        /* --- HSV判定用の平均と標準偏差 --- */
        double meanH, sigmaH;
        double meanS, sigmaS;
        double meanV, sigmaV;

        /* --- RGB判定用の平均と標準偏差 --- */
        double meanR, sigmaR;
        double meanG, sigmaG;
        double meanB, sigmaB;

        std::string color_system; // "RGB" or "HSV"

    public:
        PuckTrackerParameters();
    };

public:
    PuckTracker(PuckTrackerParameters& paras);
    ~PuckTracker();
private:
    PuckTracker(const PuckTracker&);
    PuckTracker& operator = (const PuckTracker&);

private:
    bool initialize();
    void release();

public:
    bool setParameters(PuckTrackerParameters& paras);
    PuckTrackerParameters getParameters();

private:
    double calcLikelihood(IplImage*& img, int particleX, int particleY);
    void undist(const double* uv, double* u_uv, double* u_xy);
    void transHomo(const CvMat* homo, const double* src, double* dst);

public:
    bool isInitialized();
    PuckPosition getPuckPosition(IplImage*& img);

private:
    struct Impl;
    std::auto_ptr<Impl> pimpl_;
};


#endif // PUCK_TRACKER_HPP_D729B71C_17FC_4FBA_B146_C0BB253DF20F
