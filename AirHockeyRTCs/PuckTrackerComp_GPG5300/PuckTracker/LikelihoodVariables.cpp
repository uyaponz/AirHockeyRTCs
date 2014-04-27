#include "PuckTrackerVariables.hpp"

#include <opencv2/legacy/compat.hpp>


static const double VEL_BOUND      = 50.0;


/* LikelihoodVariables() - コンストラクタ */
LikelihoodVariables::LikelihoodVariables() :
    limitLikelihood(0.0),
    cond(0),
    lowerBound(0), upperBound(0)
{
}


/* ~LikelihoodVariables() - デストラクタ */
LikelihoodVariables::~LikelihoodVariables()
{
    releaseConDensation();
}


/* isInitializedConDensation() - 初期化されたか */
bool LikelihoodVariables::isInitializedConDensation() const
{
    if (!cond || !lowerBound || !upperBound) return false;
    return true;
}


/* releaseConDensation() - 解放 */
void LikelihoodVariables::releaseConDensation()
{
    if (cond) cvReleaseConDensation(&cond);
    if (lowerBound) cvReleaseMat(&lowerBound);
    if (upperBound) cvReleaseMat(&upperBound);
}


/* initSamples() - パーティクルの位置の初期化 */
void LikelihoodVariables::initSamples(PuckTracker::PuckTrackerParameters& paras)
{
    if (!cond || !lowerBound || !upperBound) return;

    cvConDensInitSampleSet(cond, lowerBound, upperBound);
    cvRandInit(&(cond->RandS[0]),
               -paras.p_noise, paras.p_noise, static_cast<int>(cvGetTickCount()));
    cvRandInit(&(cond->RandS[1]),
               -paras.p_noise, paras.p_noise, static_cast<int>(cvGetTickCount()));
    cvRandInit(&(cond->RandS[2]),
               -paras.v_noise, paras.v_noise, static_cast<int>(cvGetTickCount()));
    cvRandInit(&(cond->RandS[3]),
               -paras.v_noise, paras.v_noise, static_cast<int>(cvGetTickCount()));
}


/* initConDensation() - 初期化 */
bool LikelihoodVariables::initConDensation(PuckTracker::PuckTrackerParameters& paras)
{
    releaseConDensation();

    bool retval = false;

    if (paras.color_system == "HSV") {
        limitLikelihood = (1.0 / (sqrt(2.0*CV_PI)*pow(paras.sigmaH,2))) * expf(-2.0)
                        * (1.0 / (sqrt(2.0*CV_PI)*pow(paras.sigmaS,2))) * expf(-2.0)
                        * (1.0 / (sqrt(2.0*CV_PI)*pow(paras.sigmaV,2))) * expf(-2.0);
    }
    else if (paras.color_system == "RGB") {
        limitLikelihood = (1.0 / (sqrt(2.0*CV_PI)*pow(paras.sigmaR,2))) * expf(-2.0)
                        * (1.0 / (sqrt(2.0*CV_PI)*pow(paras.sigmaG,2))) * expf(-2.0)
                        * (1.0 / (sqrt(2.0*CV_PI)*pow(paras.sigmaB,2))) * expf(-2.0);
    }
    else goto close;
    limitLikelihood /= 3.0;

    cond = cvCreateConDensation(4, 0, paras.n_particle);
    if (!cond) goto close;

    lowerBound = cvCreateMat(4, 1, CV_32FC1);
    upperBound = cvCreateMat(4, 1, CV_32FC1);
    if (!lowerBound || !upperBound) goto close;
    cvmSet(lowerBound, 0, 0, 0.0);
    cvmSet(lowerBound, 1, 0, 0.0);
    cvmSet(lowerBound, 2, 0, -VEL_BOUND);
    cvmSet(lowerBound, 3, 0, -VEL_BOUND);
    cvmSet(upperBound, 0, 0, static_cast<double>(paras.captureWidth));
    cvmSet(upperBound, 1, 0, static_cast<double>(paras.captureHeight));
    cvmSet(upperBound, 2, 0, VEL_BOUND);
    cvmSet(upperBound, 3, 0, VEL_BOUND);
    initSamples(paras);

    cond->DynamMatr[0]  = 1.0;
    cond->DynamMatr[1]  = 0.0;
    cond->DynamMatr[2]  = 1.0;
    cond->DynamMatr[3]  = 0.0;
    cond->DynamMatr[4]  = 0.0;
    cond->DynamMatr[5]  = 1.0;
    cond->DynamMatr[6]  = 0.0;
    cond->DynamMatr[7]  = 1.0;
    cond->DynamMatr[8]  = 0.0;
    cond->DynamMatr[9]  = 0.0;
    cond->DynamMatr[10] = 1.0;
    cond->DynamMatr[11] = 0.0;
    cond->DynamMatr[12] = 0.0;
    cond->DynamMatr[13] = 0.0;
    cond->DynamMatr[14] = 0.0;
    cond->DynamMatr[15] = 1.0;

    retval = true;
close:
    return retval;
}


/* ------ 専用関数 ------ */
bool isInitialized(const LikelihoodVariables& like)
{
    bool retval = true;
    retval &= like.isInitializedConDensation();
    return retval;
}
bool initialize(LikelihoodVariables& like, PuckTracker::PuckTrackerParameters& paras)
{
    bool retval = true;
    retval &= like.initConDensation(paras);
    return retval;
}
void release(LikelihoodVariables& like)
{
    like.releaseConDensation();
}
