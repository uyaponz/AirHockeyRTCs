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
                    2500,       // 1��]�̃J�E���g��
                    1,          // ���{��
                    rpm6v,      // 6V�������Ƃ��̉�]��[rpm]
                    15,         // ������
                    1000,       // Z=0����̃I�t�Z�b�g
                    torad(0.0), // �����A�[���p�x[rad]
                    rpm6v/15,   // �ڕW��]��[rpm]
                    550.0);     // �������x[rpm/sec]

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
