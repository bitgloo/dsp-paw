#ifndef CORDIC_HPP_
#define CORDIC_HPP_

namespace cordic {
    constexpr double PI = 3.1415926535L;

    void init();

#if !defined(TARGET_PLATFORM_L4)
    double mod(double n, double d);

    double cos(double x);
    double sin(double x);
    double tan(double x);
#else
    float mod(float n, float d);

    float cos(float x);
    float sin(float x);
    float tan(float x);
#endif
}

#endif // CORDIC_HPP_

