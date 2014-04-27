#ifndef PEX361316_FUNC_HPP_C4F46466_6789_440C_85F7_9D6ADEE644B8
#define PEX361316_FUNC_HPP_C4F46466_6789_440C_85F7_9D6ADEE644B8

namespace PEX361316Func {
    /* DA */
    int openDeviceDA(int devID);
    int closeDeviceDA(int devID);

    /* AD */
    int openDeviceAD(int devID);
    int closeDeviceAD(int devID);

    /* Penc */
    int openDevicePenc(int devID);
    int closeDevicePenc(int devID);

    /* 電圧値をDaOutputData()用の値に変換する */
    unsigned short getVoltageDA_10V(double v);

    /* 逓倍数が有効な値であるか */
    bool isValidEvaluateNum(int ev);

    double rpm2voltage(double rpm, double rpm6V=1000.0);
}

#endif // PEX361316_FUNC_HPP
