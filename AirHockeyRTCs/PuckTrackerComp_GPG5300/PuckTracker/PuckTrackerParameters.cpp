#include "PuckTracker.hpp"

PuckTracker::PuckTrackerParameters::PuckTrackerParameters()
    :
    calibrationData("./camera.xml"),
    captureWidth(640),
    captureHeight(480),

    n_particle(400),
    p_noise(30.0),
    v_noise(5.0),
    sigma(50.0),

    meanH( 7.5),               sigmaH(20.0),
    meanS(50.0 * 255.0/100.0), sigmaS(10.0),
    meanV(12.5 * 255.0/100.0), sigmaV(10.0),
//    meanH(7.5), sigmaH(15.0),
//    meanS(50.0 * 255.0/100.0), sigmaS(8.0),
//    meanV(12.5 * 255.0/100.0), sigmaV(5.0),

    meanR(0.0), sigmaR(0.0),
    meanG(0.0), sigmaG(0.0),
    meanB(0.0), sigmaB(0.0),

    color_system("HSV")
{
}
