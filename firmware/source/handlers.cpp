#include "handlers.hpp"

#include "adc.hpp"
#include "conversion.hpp"
#include "cordic.hpp"
#include "runstatus.hpp"

extern "C" {

time_measurement_t conversion_time_measurement;

__attribute__((naked))
void port_syscall(struct port_extctx *ctxp, uint32_t n)
{
    switch (n) {

    // Sleeps the current thread until a message is received.
    // Used the algorithm runner to wait for new data.
    case 0:
        {
            chSysLock();
            chMsgWaitS();
            auto monitor = ConversionManager::getMonitorHandle();
            auto msg = chMsgGet(monitor);
            chMsgReleaseS(monitor, MSG_OK);
            chSysUnlock();
            ctxp->r0 = msg;
        }
        break;

    // Provides access to advanced math functions.
    // A service call like this is required for some hardware targets that
    // provide hardware-accelerated math computations (e.g. CORDIC).
    case 1:
        {
            using mathcall = void (*)();
            static mathcall funcs[3] = {
                reinterpret_cast<mathcall>(cordic::sin),
                reinterpret_cast<mathcall>(cordic::cos),
                reinterpret_cast<mathcall>(cordic::tan),
            };
#if defined(PLATFORM_H7)
            asm("vmov.f64 d0, %0, %1" :: "r" (ctxp->r1), "r" (ctxp->r2));
            if (ctxp->r0 < 3) {
                funcs[ctxp->r0]();
                asm("vmov.f64 %0, %1, d0" : "=r" (ctxp->r1), "=r" (ctxp->r2));
            } else {
                asm("eor r0, r0; vmov.f64 d0, r0, r0");
            }
#else
            asm("vmov.f32 s0, %0" :: "r" (ctxp->r1));
            if (ctxp->r0 < 3) {
                funcs[ctxp->r0]();
                asm("vmov.f32 %0, s0" : "=r" (ctxp->r1));
            } else {
                asm("eor r0, r0; vmov.f32 s0, r0");
            }
#endif
        }
        break;

    // Starts or stops precise cycle time measurement.
    // Used to measure algorithm execution time.
    case 2:
        if (ctxp->r0 == 0) {
            chTMStartMeasurementX(&conversion_time_measurement);
        } else {
            chTMStopMeasurementX(&conversion_time_measurement);
            // Subtract measurement overhead from the result.
            // Running an empty algorithm ("bx lr") takes 196 cycles as of 2/4/21.
            // Only measures algorithm code time (loading args/storing result takes 9 cycles).
            constexpr rtcnt_t measurement_overhead = 196 - 1;
            if (conversion_time_measurement.last > measurement_overhead)
                conversion_time_measurement.last -= measurement_overhead;
        }
        break;

    // Reads one of the analog inputs made available for algorithm run-time input.
    case 3:
        ctxp->r0 = ADC::readAlt(ctxp->r0);
        break;

    //case 4:
    //    {
    //        const char *str = reinterpret_cast<const char *>(ctxp->r0);
    //        auto src = str;
    //        auto dst = userMessageBuffer;
    //        while (*src)
    //            *dst++ = *src++;
    //        *dst = '\0';
    //        userMessageSize = src - str;
    //    }
    //    break;
    default:
        while (1);
        break;
    }

    asm("svc 0");
    while (1);
}

__attribute__((naked))
void MemManage_Handler()
{
    // 1. Get the stack pointer.
    uint32_t lr;
    asm("mov %0, lr" : "=r" (lr));

    // 2. Recover from the fault.
    ConversionManager::abort((lr & (1 << 4)) ? false : true);

    // 3. Return.
    asm("mov lr, %0; bx lr" :: "r" (lr));
}

__attribute__((naked))
void HardFault_Handler()
{
    // Get the stack pointer.
    //uint32_t *stack;
    uint32_t lr;
    asm("mov %0, lr" : "=r" (lr));
    /*asm("\
        tst lr, #4; \
        ite eq; \
        mrseq %0, msp; \
        mrsne %0, psp; \
        mov %1, lr; \
    " : "=r" (stack), "=r" (lr));*/

    // If coming from the algorithm, attempt to recover; otherwise, give up.
    if (run_status != RunStatus::Running && (lr & 4) != 0)
        MemManage_Handler();

    while (1);
}

} // extern "C"

