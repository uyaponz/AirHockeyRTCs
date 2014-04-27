#ifndef AH_CAMERA_GPG_5300_HPP_C8F9662A_F085_42DA_AC47_202F61EDB273
#define AH_CAMERA_GPG_5300_HPP_C8F9662A_F085_42DA_AC47_202F61EDB273

#include <string>
#include <opencv2/opencv.hpp>

namespace AHCamera
{
    const int camW = 640; // キャプチャ横サイズ
    const int camH = 480; // キャプチャ縦サイズ

    bool initCamera(int devNo, const std::string& confFile);
    void releaseCamera();
    bool getCaptureImage(IplImage*& dst);
}

#endif // AH_CAMERA_GPG_5300_HPP_C8F9662A_F085_42DA_AC47_202F61EDB273
