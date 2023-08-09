#include "cordic.hpp"
#include "hal.h"

#if !defined(TARGET_PLATFORM_L4)
namespace cordic {

void init()
{
    RCC->AHB2ENR |= RCC_AHB2ENR_CORDICEN;
}

static void prepare() {
    while (CORDIC->CSR & CORDIC_CSR_RRDY)
        asm("mov r0, %0" :: "r" (CORDIC->RDATA));
}

static uint32_t dtoq(double) {
    uint32_t res;
    asm("vcvt.s32.f64 d0, d0, #31;"
        "vmov %0, r5, d0"
    : "=r" (res));
    return res;
}
__attribute__((naked))
static double qtod(uint32_t) {
    asm("eor r1, r1;"
        "vmov d0, r0, r1;"
        "vcvt.f64.s32 d0, d0, #31;"
        "bx lr");
    return 0;
}
__attribute__((naked))
double mod(double, double) {
    asm("vdiv.f64   d2, d0, d1;"
        "vrintz.f64 d2;"
        "vmul.f64   d1, d1, d2;"
        "vsub.f64   d0, d0, d1;"
        "bx lr");
    return 0;
}

double cos(double x) {
    x = mod(x, 2 * math::PI) / math::PI;
    auto input = dtoq(x > 1. ? x - 2 : x);

    prepare();
    CORDIC->CSR = CORDIC_CSR_NARGS | CORDIC_CSR_NRES |
                  (6 << CORDIC_CSR_PRECISION_Pos) |
                  (0 << CORDIC_CSR_FUNC_Pos);

    CORDIC->WDATA = input;
    CORDIC->WDATA = input;
    while (!(CORDIC->CSR & CORDIC_CSR_RRDY));

    double cosx = qtod(CORDIC->RDATA) / x;
    [[maybe_unused]] auto sinx = CORDIC->RDATA;
    return cosx;
}

double sin(double x) {
    x = mod(x, 2 * math::PI) / math::PI;
    auto input = dtoq(x > 1. ? x - 2 : x);

    prepare();
    CORDIC->CSR = CORDIC_CSR_NARGS | CORDIC_CSR_NRES |
                  (6 << CORDIC_CSR_PRECISION_Pos) |
                  (1 << CORDIC_CSR_FUNC_Pos);

    CORDIC->WDATA = input;
    CORDIC->WDATA = input;
    while (!(CORDIC->CSR & CORDIC_CSR_RRDY));

    double sinx = qtod(CORDIC->RDATA) / x;
    [[maybe_unused]] auto cosx = CORDIC->RDATA;
    return sinx;
}

double tan(double x) {
    x = mod(x, 2 * math::PI) / math::PI;
    auto input = dtoq(x > 1. ? x - 2 : x);

    prepare();
    CORDIC->CSR = CORDIC_CSR_NARGS | CORDIC_CSR_NRES |
                  (6 << CORDIC_CSR_PRECISION_Pos) |
                  (1 << CORDIC_CSR_FUNC_Pos);

    CORDIC->WDATA = input;
    CORDIC->WDATA = input;
    while (!(CORDIC->CSR & CORDIC_CSR_RRDY));

    double sinx = qtod(CORDIC->RDATA) / x;
    double tanx = sinx * x / qtod(CORDIC->RDATA);
    return tanx;
}

}
#else // L4
#include <cmath>
namespace cordic {

void init() {}

float mod(float a, float b) {
    return a - (b * std::floor(a / b));
}

float cos(float x) { return std::cos(x); }
float sin(float x) { return std::sin(x); }
float tan(float x) { return std::tan(x); }

}
#endif

