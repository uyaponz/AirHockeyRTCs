#include "PuckTracker.hpp"

#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <opencv2/legacy/legacy.hpp>
#include <opencv2/legacy/compat.hpp>

#include "PuckTrackerVariables.hpp"


static const int    PARTICLE_LIMIT = 40;


/* convertRGB2HSV() - RGB値をHSV値に変換する */
static inline bool convertRGB2HSV(double  r, double  g, double  b,
                                  double& h, double& s, double &v)
{
    double hh, ss, vv;
    h=s=v=0;

    int type = 0;
    double max = r;
    if (g > max) { type=1; max=g; }
    if (b > max) { type=2; max=b; }

    if (max <= 0.0) return false;
    vv = max;

    double min = r;
    if (g < min) min=g;
    if (b < min) min=b;

    double diff = max - min;
    if (diff <= 0.0) return false;
    ss = 255.0 * (diff / max);

    hh = 60.0;
    if      (type == 0) { hh *=        (b-g) / diff;  }
    else if (type == 1) { hh *= 2.0 + ((r-b) / diff); }
    else                { hh *= 4.0 + ((g-r) / diff); }

    if (hh < 0) { hh += 360.0; }

    h=hh; s=ss; v=vv;
    return true;
}


/* getDifferenceOfDegree() - 2つの角度差(p2-p1)を取得する */
static inline double getDifferenceOfDegree(double p1, double p2)
{
    double sub = p2 - p1;
    sub -= floor(sub/360.0) * 360.0;
    if (sub > 180.0) sub -= 360.0;
    return sub;
}


/* getLikelihood_Gaussian() - ガウス分布に基づく尤度計算を行う */
static inline double getLikelihood_Gaussian(double dist, double sigma2)
{
    double retval = 1.0 / sqrt(2.0 * CV_PI * sigma2);
    retval *= expf(-(dist * dist) / (2.0 * sigma2));
    return retval;
}


/* PuckTrackerクラスのpimpl */
struct PuckTracker::Impl
{
    PuckTracker::PuckTrackerParameters paras; // 各種パラメータ
    CameraVariables     cam;  // カメラ関連の変数
    LikelihoodVariables like; // 尤度計算用変数

public:
    Impl() :
        paras(),
        cam(),
        like()
    {}
};


/* PuckTracker() - コンストラクタ */
PuckTracker::PuckTracker(PuckTracker::PuckTrackerParameters& paras)
    :
    pimpl_(new Impl)
{
    /* --- パラメータを設定する --- */
    if (!setParameters(paras)) return;
}


/* ~PuckTracker() - デストラクタ */
PuckTracker::~PuckTracker()
{
    release();
}


/* initialize() - 初期化 */
bool PuckTracker::initialize()
{
    bool retval = true;
    retval &= ::initialize(pimpl_->cam,  pimpl_->paras.calibrationData);
    retval &= ::initialize(pimpl_->like, pimpl_->paras);
    return retval;
}


/* release() - 解放 */
void PuckTracker::release()
{
    ::release(pimpl_->cam);
    ::release(pimpl_->like);
}


/* setParameters() - パラメータを設定する --- */
bool PuckTracker::setParameters(PuckTracker::PuckTrackerParameters& paras)
{
    PuckTracker::PuckTrackerParameters bkupParas = getParameters();
    pimpl_->paras = paras;

    if (!initialize()) {
        pimpl_->paras = bkupParas;
        initialize();
        return false;
    }

    return true;
}


/* getParameters() - 現在使用中のパラメータを取得する --- */
PuckTracker::PuckTrackerParameters PuckTracker::getParameters()
{
    return pimpl_->paras;
}


/* calcLikelihood() - 尤度計算 */
double PuckTracker::calcLikelihood(IplImage*& img, int particleX, int particleY)
{
    if (!isInitialized()) return 0.0;

    double b = static_cast<double>(img->imageData[img->widthStep*particleY + particleX*3]);
    double g = static_cast<double>(img->imageData[img->widthStep*particleY + particleX*3+1]);
    double r = static_cast<double>(img->imageData[img->widthStep*particleY + particleX*3+2]);

    if (pimpl_->paras.color_system == "RGB") {
        r -= pimpl_->paras.meanR;
        g -= pimpl_->paras.meanG;
        b -= pimpl_->paras.meanB;

        double result = 1.0;
        result *= getLikelihood_Gaussian(sqrt(r*r), pow(pimpl_->paras.sigmaR,2));
        result *= getLikelihood_Gaussian(sqrt(g*g), pow(pimpl_->paras.sigmaG,2));
        result *= getLikelihood_Gaussian(sqrt(b*b), pow(pimpl_->paras.sigmaB,2));
        return result;
    }

    if (pimpl_->paras.color_system == "HSV") {
        double h, s, v;
        convertRGB2HSV(r,g,b, h,s,v);
        h  = getDifferenceOfDegree(h, pimpl_->paras.meanH);
        s -= pimpl_->paras.meanS;
        v -= pimpl_->paras.meanV;

        double result = 1.0;
        result *= getLikelihood_Gaussian(sqrt(h*h), pow(pimpl_->paras.sigmaH,2));
        result *= getLikelihood_Gaussian(sqrt(s*s), pow(pimpl_->paras.sigmaS,2));
        result *= getLikelihood_Gaussian(sqrt(v*v), pow(pimpl_->paras.sigmaV,2));
        return result;
    }

    return 0.0;
}


/* isInitialized() - 初期化が完了しているか */
bool PuckTracker::isInitialized()
{
    bool retval = true;
    retval &= ::isInitialized(pimpl_->cam);
    retval &= ::isInitialized(pimpl_->like);
    return retval;
}


/* getPuckPosition() - パックの座標を取得する */
PuckTracker::PuckPosition PuckTracker::getPuckPosition(IplImage*& img)
{
    PuckPosition pos = {{0.0, 0.0}, 0.0};
    if (!isInitialized()) return pos;

    CvConDensation *cond = pimpl_->like.cond;
    int    n_match = 0;
    double match   = 0.0;
    double a_x=0.0, a_y=0.0, a_vx=0.0, a_vy=0.0;
    double maxLikelihood = 0.0;
    for (int i=0; i<pimpl_->paras.n_particle; i++) {
        int particleX = static_cast<int>(cond->flSamples[i][0]);
        int particleY = static_cast<int>(cond->flSamples[i][1]);

        if (particleX<0 || particleX>pimpl_->paras.captureWidth ||
            particleY<0 || particleY>pimpl_->paras.captureHeight)
        {
            cond->flConfidence[i] = 0.0;
        }
        else {
            cond->flConfidence[i] = calcLikelihood(img, particleX, particleY);
            if (cond->flConfidence[i] > pimpl_->like.limitLikelihood) {
                n_match += 1;
                match += cond->flConfidence[i];
                a_x  += cond->flSamples[i][0] * cond->flConfidence[i];
                a_y  += cond->flSamples[i][1] * cond->flConfidence[i];
                a_vx += cond->flSamples[i][2] * cond->flConfidence[i];
                a_vy += cond->flSamples[i][3] * cond->flConfidence[i];
                cvCircle(img, cvPoint(particleX,particleY), 2, CV_RGB(0,255,255), -1, 8);
            }
            else {
                cvCircle(img, cvPoint(particleX,particleY), 2, CV_RGB(0,0,255), -1, 8);
            }
            if (cond->flConfidence[i] > maxLikelihood) maxLikelihood = cond->flConfidence[i];
        }
    }

    if (n_match > pimpl_->paras.n_particle/PARTICLE_LIMIT) {
        a_x  /= match;
        a_y  /= match;
        a_vx /= match;
        a_vy /= match;
        cvCircle(img,
                 cvPoint(static_cast<int>(a_x),static_cast<int>(a_y)),
                 5, CV_RGB(255,255,255), -1, 8);
    }
    else if (maxLikelihood < pimpl_->like.limitLikelihood) {
        n_match = 0;
        match = a_x = a_y = a_vx = a_vy = 0.0;

        pimpl_->like.initSamples(pimpl_->paras);
        for (int i=0; i<pimpl_->paras.n_particle; i++) {
            int particleX = static_cast<int>(cond->flSamples[i][0]);
            int particleY = static_cast<int>(cond->flSamples[i][1]);

            if (particleX<0 || particleX>pimpl_->paras.captureWidth ||
                particleY<0 || particleY>pimpl_->paras.captureHeight)
            {
                cond->flConfidence[i] = 0.0;
            }
            else {
                cond->flConfidence[i] = 1.0;
            }
        }
    }
    else {
        a_x  /= match;
        a_y  /= match;
        a_vx /= match;
        a_vy /= match;
        cvCircle(img,
                 cvPoint(static_cast<int>(a_x),static_cast<int>(a_y)),
                 5, CV_RGB(0,255,0), -1, 8);
    }

    cvConDensUpdateByTime(cond);
    double uv[2]   = {a_x, a_y};
    double u_uv[2] = {0.0, 0.0};
    double u_xy[2] = {0.0, 0.0};
    double t_xy[2] = {0.0, 0.0};
    undist(uv, u_uv, u_xy);
    transHomo(pimpl_->cam.invHomo, u_uv, t_xy);

    pos.position.x = t_xy[0];
    pos.position.y = t_xy[1];
    pos.likelihood = static_cast<double>(n_match);
    return pos;
}


/* undist() - uv座標にカメラパラメータを適用する */
void PuckTracker::undist(const double* uv, double* u_uv, double* u_xy)
{
    if (!uv || !u_uv || !u_xy) return;

    /* --- 歪みパラメータ --- */
    double k1, k2, p1, p2;
    k1 = *((float*)(pimpl_->cam.distortion->data.ptr) + 0);
    k2 = *((float*)(pimpl_->cam.distortion->data.ptr) + 1);
    p1 = *((float*)(pimpl_->cam.distortion->data.ptr) + 2);
    p2 = *((float*)(pimpl_->cam.distortion->data.ptr) + 3);

    /* --- カメラ行列 --- */
    double intr_ax, intr_cx, intr_ay, intr_cy;
    intr_ax = *(((float*)pimpl_->cam.intrinsic->data.ptr) + 0);
    intr_cx = *(((float*)pimpl_->cam.intrinsic->data.ptr) + 2);
    intr_ay = *(((float*)pimpl_->cam.intrinsic->data.ptr) + 3+1);
    intr_cy = *(((float*)pimpl_->cam.intrinsic->data.ptr) + 3+2);

    u_uv[0] = uv[0];
    u_uv[1] = uv[1];

    for (;;) {
        u_xy[0] = (u_uv[0] - intr_cx) / intr_ax;
        u_xy[1] = (u_uv[1] - intr_cy) / intr_ay;
        double x  = u_xy[0];
        double y  = u_xy[1];
        double xx = x * x;
        double yy = y * y;
        double xy = x * y;
        double rr   = x*x + y*y;
        double rrrr = rr * rr;

        double dist_xy[2] = {x*(1 + k1*rr + k2*rrrr) + 2*p1*xy + p2*(rr + 2*xx),
                             y*(1 + k1*rr + k2*rrrr) + 2*p2*xy + p1*(rr + 2*yy)};

        double dist_uv[2] = {intr_ax*dist_xy[0] + intr_cx,
                             intr_ay*dist_xy[1] + intr_cy};

        double duv[2] = {dist_uv[0] - uv[0],
                         dist_uv[1] - uv[1]};

        double dd = sqrt(duv[0]*duv[0] + duv[1]*duv[1]);
        if (dd < 0.01) break;

        u_uv[0] -= duv[0];
        u_uv[1] -= duv[1];
    }
}


/* transHomo() - 指定された座標にホモグラフィ行列を適用して返す */
void PuckTracker::transHomo(const CvMat* homo, const double* src, double* dst)
{
    if (!homo || !src || !dst) return;

    double h00, h01, h02;
    double h10, h11, h12;
    double h20, h21, h22;
    h00 = *(((float*)homo->data.ptr) + 0);
    h01 = *(((float*)homo->data.ptr) + 1);
    h02 = *(((float*)homo->data.ptr) + 2);
    h10 = *(((float*)homo->data.ptr) + 3);
    h11 = *(((float*)homo->data.ptr) + 4);
    h12 = *(((float*)homo->data.ptr) + 5);
    h20 = *(((float*)homo->data.ptr) + 6);
    h21 = *(((float*)homo->data.ptr) + 7);
    h22 = *(((float*)homo->data.ptr) + 8);

    dst[0] = h00*src[0] + h01*src[1] + h02;
    dst[1] = h10*src[0] + h11*src[1] + h12;

    double w = h20*src[0] + h21*src[1] + h22;
    dst[0] /= w;
    dst[1] /= w;
}
