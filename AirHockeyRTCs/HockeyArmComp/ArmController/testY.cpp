#include <iostream>
#include <cmath>

#include "MC_PEX361316.hpp"

double torad(double deg)
{
    return (deg / 360.0) * (2.0 * M_PI);
}

int main(void)
{
    double rpm6v = 3000.0;

    MC_PEX361316 mp(2,          // devNo
                    2500,       // 1回転のカウント数
                    1,          // 逓倍数
                    rpm6v,      // 6V流したときの回転数[rpm]
                    15,         // 減速比
                    1000,       // Z=0からのオフセット
                    torad(0.0), // 初期アーム角度[rad]
                    rpm6v/15,   // 目標回転数[rpm]
                    550.0);     // 加減速度[rpm/sec]

    mp.start();

    for (;;) {
        double deg = 0.0;
        std::cout << "input arm angle [deg] > ";
        std::cin >> deg;
        if (deg > 45.0  ||  deg < -45.0)
            std::cout << "cannot set." << std::endl;
        else if (!mp.moveTo(torad(deg)))
            std::cout << "cannot set." << std::endl;
    }

    return 0;
}
