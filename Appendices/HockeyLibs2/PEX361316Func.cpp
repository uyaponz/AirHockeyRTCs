#include "PEX361316Func.hpp"

#include <fbiad.h>
#include <fbida.h>
#include <fbipenc.h>

namespace PEX361316Func {
    /* DA */
    int openDeviceDA(int devID) {
        if (DaOpen(devID) != DA_ERROR_SUCCESS) return -1;
        return 0;
    }
    int closeDeviceDA(int devID) {
        if (DaClose(devID) != DA_ERROR_SUCCESS) return -1;
        return 0;
    }

    /* AD */
    int openDeviceAD(int devID) {
        if (AdOpen(devID) != AD_ERROR_SUCCESS) return -1;
        return 0;
    }
    int closeDeviceAD(int devID) {
        if (AdClose(devID) != AD_ERROR_SUCCESS) return -1;
        return 0;
    }

    /* Penc */
    int openDevicePenc(int devID) {
        if (PencOpen(devID, PENC_FLAG_NORMAL) != PENC_ERROR_SUCCESS) return -1;
        return 0;
    }
    int closeDevicePenc(int devID) {
        if (PencClose(devID) != PENC_ERROR_SUCCESS) return -1;
        return 0;
    }

    /* 電圧値をDaOutputData()用の値に変換する */
    unsigned short getVoltageDA_10V(double v) {
        static const int boardMaxV = 10; // ボードの出力限界電圧
        static const unsigned short maxVal = 0xFFFF; // 最大設定値

        if (v >=  boardMaxV) v= boardMaxV;
        if (v <= -boardMaxV) v=-boardMaxV;

        return static_cast<unsigned short>((v+boardMaxV) / ((2.0*boardMaxV)/maxVal));
    }

    /* てい倍数が有効な値であるか */
    bool isValidEvaluateNum(int ev) {
        if (ev==1 || ev==2 || ev==4) return true;
        return false;
    }

    double rpm2voltage(double rpm, double rpm6V)
    {
        return 6.0 * (rpm / rpm6V);
    }
}
