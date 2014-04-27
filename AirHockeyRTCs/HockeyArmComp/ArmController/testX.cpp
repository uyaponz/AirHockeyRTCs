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

    MC_PEX361316 mp(1,          // devNo
                    2500,       // 1��]�̃J�E���g��
                    1,          // ���{��
                    rpm6v,      // 6V�������Ƃ��̉�]��[rpm]
                    25,         // ������
                    225,        // Z=0����̃I�t�Z�b�g
                    torad(0.0), // �����A�[���p�x[rad]
                    rpm6v/25,   // �ڕW��]��[rpm]
                    150.0);     // �������x[rpm/sec]

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
