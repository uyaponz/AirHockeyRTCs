#include <iostream>
#include <unistd.h>

#include "ArmController.hpp"
#include "MC_PEX361316.hpp"

double torad(double deg)
{
    return (deg / 360.0) * (2.0 * M_PI);
}

int main()
{
    double rpm6v = 3000.0;
    MC_PEX361316 mp_x(1,
                      2500,
                      1,
                      rpm6v,
                      25,
                      225,
                      torad(90.0),
                      rpm6v/25,
                      300.0,
                      200.0);
    MC_PEX361316 mp_y(2,
                      2500,
                      1,
                      rpm6v,
                      15,
                      1000,
                      torad(0.0),
                      rpm6v/15,
                      550.0,
                      500.0);

    ArmController ac(&mp_x, &mp_y, 558.5, 319.45, true);

    ac.start();
    for (;;) {
        double x, y;
        std::cout << "input positions (x,y)." << std::endl;
        std::cout << "x = ? > ";
        std::cin >> x;
        std::cout << "y = ? > ";
        std::cin >> y;

        ArmPosition pos = {x, y};
        if (!ac.moveTo(pos)) {
            std::cout << "cannot move." << std::endl;
        }
    }
//    for (;;) {
//        ArmPosition pos = ac.getArmPos();
//        std::cout << "pos : " << pos.x << ", " << pos.y << std::endl;
//        usleep(100000);
//    }
    ac.stop();

    return 0;
}
