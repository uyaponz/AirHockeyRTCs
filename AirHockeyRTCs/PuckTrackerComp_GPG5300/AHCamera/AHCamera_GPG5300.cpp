#include "AHCamera_GPG5300.hpp"

#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <ifcml.h> // GPG-5300


namespace AHCamera
{
    static const bool USE_ONBOARD_MEMORY = false; // オンボードメモリを使う場合はtrueにする

    static int deviceNumber          = 0; // オープンしたデバイス番号
    static unsigned long *buffer     = 0; // キャプチャ先のバッファ
    static unsigned long  bufferSize = 0; // バッファのサイズ
    static unsigned long  memHandle  = 0; // バッファへのハンドル
    static IplImage      *iplBuffer  = 0; // バッファのIplImage版(メモリはbufferと共有)
    static char *bkupIplBufferPtr    = 0; // iplBufferのimageDataのバックアップ
    static IFCMLCAPFMT capFmt;            // キャプチャフォーマット


    /* メモリ操作関数(確保・解放) */
    #if defined(__linux__)
        #include <stdlib.h>
        static inline void *_aligned_malloc(size_t size, size_t alignment)
        {
            void *p = 0;
            int ret = posix_memalign(&p, alignment, size);
            return (ret == 0) ? p : 0;
        }
        static inline void _aligned_free(void *memblock) { free(memblock); }
    #elif defined(_WIN32)
        #include <malloc.h>
    #endif


    /* RETPRINT() - キャプチャボードのエラーメッセージ */
    void RETPRINT(int ret)
    {
        std::cout << "ERROR : " << ret << std::endl;
    }


    /* eventCaptured() - キャプチャイベントで呼び出される */
    void eventCaptured(unsigned long intFlag, unsigned long user)
    { intFlag=intFlag; user=user;
        if (USE_ONBOARD_MEMORY)
            CmlGetMemData(deviceNumber, memHandle, (void*)buffer, 1, &(capFmt.Rect), 0x0001);
    }


    /* releaseMemory() - キャプチャ用に確保したメモリを解放する */
    void releaseMemory()
    {
        // unsigned long *
        if (buffer != 0) {
            _aligned_free(buffer);
            buffer = 0;
        }

        // IplImage *
        if (iplBuffer != 0) {
            // 書き戻し
            if (bkupIplBufferPtr != 0) {
                iplBuffer->imageData = bkupIplBufferPtr;
                bkupIplBufferPtr = 0;
            }
            cvReleaseImage(&iplBuffer);
            iplBuffer = 0;
        }
    }


    /* initCamera() - カメラデバイスの初期化 */
    bool initCamera(int devNo, const std::string& confFile)
    {
        /* --- エラーチェック --- */
        if (deviceNumber != 0) return false; // すでにオープンしている

        int ret;
        capFmt.Rect.XStart  = 0;   // 開始X座標
        capFmt.Rect.YStart  = 0;   // 開始Y座標
        capFmt.Rect.XLength = camW; // 水平方向の長さ
        capFmt.Rect.YLength = camH; // 垂直方向の長さ
        capFmt.Scale.PixelCnt = 0; // 水平間引きピクセル数
        capFmt.Scale.LineCnt  = 0; // 垂直間引きピクセル数
        capFmt.CapFormat    = IFCML_CAPFMT_CAM; // カメラ画像フォーマット
        capFmt.OptionFormat = IFCML_OPTFMT_NON; // オプションデータ

        /* --- デバイスオープン (PEX-530421) --- */
        if ((ret=CmlOpen(devNo))) {
            RETPRINT(ret);
            return false;
        }

        /* --- カメラの設定ファイルを読み込む --- */
        if ((ret=CmlReadCamConfFile(devNo, const_cast<char*>(confFile.c_str())))) {
            RETPRINT(ret);
            goto release;
        }

        /* --- キャプチャフォーマットを設定する --- */
        if ((ret=CmlSetCaptureFormatInfo(devNo, &capFmt))) {
            RETPRINT(ret);
            goto release;
        }

        /* --- キャプチャ先のバッファ領域を作成する --- */
        // キャプチャ先バッファ
        bufferSize = capFmt.FrameSize_Buf;
        buffer     = reinterpret_cast<unsigned long*>(_aligned_malloc(bufferSize, 4096));
        if (USE_ONBOARD_MEMORY)
            ret = CmlRegistMemInfo(devNo, 0, capFmt.FrameSize_Mem, &memHandle);
        else
            ret = CmlRegistMemInfo(devNo, buffer, bufferSize, &memHandle);
        if (ret) { RETPRINT(ret); goto release; }
        // IplImageで扱えるようにする
        iplBuffer = cvCreateImage(cvSize(camW,camH), IPL_DEPTH_8U, 1);
        if (iplBuffer == 0) {
            std::cout << "Failed to create IplImage data." << std::endl;
            goto release;
        }
        bkupIplBufferPtr = iplBuffer->imageData;
        iplBuffer->imageData = reinterpret_cast<char*>(buffer);

        /* --- キャプチャ設定をデバイスにセットする --- */
        if ((ret=CmlSetCapConfig(devNo, memHandle, &capFmt))) {
            RETPRINT(ret);
            goto release;
        }

        /* --- キャプチャしたときのコールバック関数を設定する --- */
        if ((ret=CmlSetEventMask(devNo, (USE_ONBOARD_MEMORY)?0x0010:0x0001))) {
            RETPRINT(ret);
            goto release;
        }
        if ((ret=CmlSetEvent(devNo,
                             reinterpret_cast<PIFCMLCALLBACK>(eventCaptured),
                             0)))
        {
            RETPRINT(ret);
            goto release;
        }

        /* --- キャプチャ開始 --- */
        if (USE_ONBOARD_MEMORY)
            ret = CmlStartCapture(devNo, 0, IFCML_CAM_MEM | IFCML_CAP_ASYNC);
        else
            ret = CmlStartCapture(devNo, 0, IFCML_CAM_DMA | IFCML_CAP_ASYNC);
        if (ret) {
            RETPRINT(ret);
            goto release;
        }

        deviceNumber = devNo;
        return true;

    release:
        CmlClose(devNo);
        releaseMemory();
        return false;
    }


    /* releaseCamera() - カメラデバイスを解放する */
    void releaseCamera()
    {
        if (deviceNumber == 0) return;

        CmlSetEventMask(deviceNumber, 0x0000);
        if (USE_ONBOARD_MEMORY)
            CmlStopCapture(deviceNumber, IFCML_MEM_STOP);
        else
            CmlStopCapture(deviceNumber, IFCML_DMA_STOP);
        CmlFreeMemInfo(deviceNumber, memHandle);

        CmlClose(deviceNumber);

        deviceNumber = 0;
        releaseMemory();
    }


    /* getCaptureImage() - キャプチャした画像を取得する */
    bool getCaptureImage(IplImage*& dst)
    {
        if (deviceNumber == 0) return false;
        if (dst == 0)          return false;

        cvCvtColor(iplBuffer, dst, CV_BayerBG2BGR);
        return true;
    }
}
