#include "PuckTrackerVariables.hpp"

#include <string>
#include <opencv2/opencv.hpp>


/* CameraVariables() - コンストラクタ */
CameraVariables::CameraVariables() :
    intrinsic(0), rotation(0), translation(0), distortion(0),
    homo(0), invHomo(0)
{
}


/* ~CameraVariables() - デストラクタ */
CameraVariables::~CameraVariables()
{
    releaseCameraParameters();
    releaseHomography();
}


/* isReadCameraParameters() - パラメータが読み込まれているか */
bool CameraVariables::isReadCameraParameters() const
{
    if (intrinsic==0 || rotation==0 || translation==0 || distortion==0) return false;
    return true;
}


/* releaseCameraParameters() - 読み込んだカメラパラメータのメモリ領域を解放する */
void CameraVariables::releaseCameraParameters()
{
    if (intrinsic)   cvReleaseMat(&intrinsic);
    if (rotation)    cvReleaseMat(&rotation);
    if (translation) cvReleaseMat(&translation);
    if (distortion)  cvReleaseMat(&distortion);
}


/* readCameraParameters() - カメラパラメータを読み込む */
bool CameraVariables::readCameraParameters(std::string filename)
{
    releaseCameraParameters();

    bool retval = false;

    CvFileStorage *fs = cvOpenFileStorage(filename.c_str(), 0, CV_STORAGE_READ);
    if (!fs) return false;

    intrinsic   =
        static_cast<CvMat*>(cvRead(fs, cvGetFileNodeByName(fs, 0, "intrinsic")));
    rotation    =
        static_cast<CvMat*>(cvRead(fs, cvGetFileNodeByName(fs, 0, "rotation")));
    translation =
        static_cast<CvMat*>(cvRead(fs, cvGetFileNodeByName(fs, 0, "translation")));
    distortion  =
        static_cast<CvMat*>(cvRead(fs, cvGetFileNodeByName(fs, 0, "distortion")));

    if (!isReadCameraParameters()) goto close;

    retval = true;
close:
    cvReleaseFileStorage(&fs);
    return retval;
}


/* isInitializedHomography() - 初期化されたか */
bool CameraVariables::isInitializedHomography() const
{
    if (!homo || !invHomo) return false;
    return true;
}


/* releaseHomography() - メモリ領域の解放 */
void CameraVariables::releaseHomography()
{
    if (homo)    cvReleaseMat(&homo);
    if (invHomo) cvReleaseMat(&invHomo);
}


/* initHomography() - 初期化 */
bool CameraVariables::initHomography()
{
    bool retval = false;

    /* --- カメラパラメータが読み込まれていない場合は計算できないので終了 --- */
    if (!isReadCameraParameters()) return false;

    releaseHomography();

    CvMat *rotMat = cvCreateMat(3,3, CV_32FC1);

    homo    = cvCreateMat(3,3, CV_32FC1);
    invHomo = cvCreateMat(3,3, CV_32FC1);
    if (homo==0 || invHomo==0) goto close;

    cvRodrigues2(rotation, rotMat);
    *((float*)(rotMat->data.ptr)+2) = *((float*)(translation->data.ptr)+0);
    *((float*)(rotMat->data.ptr)+5) = *((float*)(translation->data.ptr)+1);
    *((float*)(rotMat->data.ptr)+8) = *((float*)(translation->data.ptr)+2);
    cvGEMM(intrinsic, rotMat, 1.0, 0, 0.0, homo, 0);
    cvInvert(homo, invHomo);

    retval = true;
close:
    cvReleaseMat(&rotMat);
    return retval;
}


/* ------ 専用関数 ------ */
bool isInitialized(const CameraVariables& cam)
{
    bool retval = true;
    retval &= cam.isReadCameraParameters();
    retval &= cam.isInitializedHomography();
    return retval;
}
bool initialize(CameraVariables& cam, std::string filename)
{
    bool retval = true;
    retval &= cam.readCameraParameters(filename);
    retval &= cam.initHomography();
    return retval;
}
void release(CameraVariables& cam)
{
    cam.releaseHomography();
    cam.releaseCameraParameters();
}
