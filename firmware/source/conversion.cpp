/**
 * @file conversion.cpp
 * @brief Manages algorithm application (converts input samples to output).
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "conversion.hpp"

#include "periph/adc.hpp"
#include "periph/dac.hpp"
#include "elfload.hpp"
#include "error.hpp"
#include "runstatus.hpp"
#include "samples.hpp"

// MSG_* things below are macros rather than constexpr
// to ensure inlining.

#define MSG_CONVFIRST          (1)
#define MSG_CONVSECOND         (2)
#define MSG_CONVFIRST_MEASURE  (3)
#define MSG_CONVSECOND_MEASURE (4)

#define MSG_FOR_FIRST(msg)   (msg & 1)
#define MSG_FOR_MEASURE(msg) (msg > 2)

__attribute__((section(".convdata")))
thread_t *ConversionManager::m_thread_monitor = nullptr;
thread_t *ConversionManager::m_thread_runner = nullptr;

__attribute__((section(".stacks")))
std::array<char, 1024> ConversionManager::m_thread_monitor_stack = {};
__attribute__((section(".stacks")))
std::array<char, THD_WORKING_AREA_SIZE(128)> ConversionManager::m_thread_runner_entry_stack = {};
__attribute__((section(".convdata")))
std::array<char, CONVERSION_THREAD_STACK_SIZE> ConversionManager::m_thread_runner_stack = {};

std::array<msg_t, 2> ConversionManager::m_mailbox_buffer;
mailbox_t ConversionManager::m_mailbox = _MAILBOX_DATA(m_mailbox, m_mailbox_buffer.data(), m_mailbox_buffer.size());

void ConversionManager::begin()
{
    m_thread_monitor = chThdCreateStatic(m_thread_monitor_stack.data(),
                                         m_thread_monitor_stack.size(),
                                         NORMALPRIO + 1,
                                         threadMonitor,
                                         nullptr);
    auto runner_stack_end = &m_thread_runner_stack[CONVERSION_THREAD_STACK_SIZE];
    m_thread_runner = chThdCreateStatic(m_thread_runner_entry_stack.data(),
                                        m_thread_runner_entry_stack.size(),
                                        HIGHPRIO,
                                        threadRunnerEntry,
                                        runner_stack_end);
}

void ConversionManager::start()
{
    Samples::Out.clear();
    ADC::start(Samples::In.data(), Samples::In.size(), adcReadHandler);
    DAC::start(0, Samples::Out.data(), Samples::Out.size());
}

void ConversionManager::startMeasurement()
{
    ADC::setOperation(adcReadHandlerMeasure);
}

void ConversionManager::stop()
{
    DAC::stop(0);
    ADC::stop();
}

thread_t *ConversionManager::getMonitorHandle()
{
    return m_thread_monitor;
}

void ConversionManager::abort(bool fpu_stacked)
{
    ELFManager::unload();
    EM.add(Error::ConversionAborted);
    //run_status = RunStatus::Recovering;

    // Confirm that the exception return thread is the algorithm...
    uint32_t *psp;
	asm("mrs %0, psp" : "=r" (psp));

    bool isRunnerStack = 
           (uint32_t)psp >= reinterpret_cast<uint32_t>(m_thread_runner_stack.data()) &&
           (uint32_t)psp <= reinterpret_cast<uint32_t>(m_thread_runner_stack.data() +
                                                       m_thread_runner_stack.size());

    if (isRunnerStack)
    {
        // If it is, we can force the algorithm to exit by "resetting" its thread.
        // We do this by rebuilding the thread's stacked exception return.
        auto newpsp = reinterpret_cast<uint32_t *>(m_thread_runner_stack.data() + 
                                                   m_thread_runner_stack.size() -
                                                   (fpu_stacked ? 26 : 8) * sizeof(uint32_t));
        // Set the LR register to the thread's entry point.
        newpsp[5] = reinterpret_cast<uint32_t>(threadRunner);
        // Overwrite the instruction we'll return to with "bx lr" (jump to address in LR).
        newpsp[6] = psp[6];
        *reinterpret_cast<uint16_t *>(newpsp[6]) = 0x4770; // "bx lr"
        // Keep PSR contents (bit set forces Thumb mode, just in case).
        newpsp[7] = psp[7] | (1 << 24);
        // Set the new stack pointer.
	    asm("msr psp, %0" :: "r" (newpsp));
    }
}

void ConversionManager::threadMonitor(void *)
{
    while (1) {
        msg_t message;
        msg_t fetch = chMBFetchTimeout(&m_mailbox, &message, TIME_INFINITE);
        if (fetch == MSG_OK)
            chMsgSend(m_thread_runner, message);
    }
}

void ConversionManager::threadRunnerEntry(void *stack)
{
    ELFManager::unload();
    port_unprivileged_jump(reinterpret_cast<uint32_t>(threadRunner),
                           reinterpret_cast<uint32_t>(stack));
}

__attribute__((section(".convcode")))
void ConversionManager::threadRunner(void *)
{
    while (1) {
        // Sleep until we receive a mailbox message.
        msg_t message;
        asm("svc 0; mov %0, r0" : "=r" (message));

        if (message != 0) {
            auto samples = MSG_FOR_FIRST(message) ? Samples::In.data()
                                                  : Samples::In.middata();
            auto size = Samples::In.size() / 2;

            auto entry = ELFManager::loadedElf();
            if (entry) {
                // Below, we remember the stack pointer just in case the
                // loaded algorithm messes things up.
                uint32_t sp;

                if (!MSG_FOR_MEASURE(message)) {
                    asm("mov %0, sp" : "=r" (sp));
                    samples = entry(samples, size);
                    asm("mov sp, %0" :: "r" (sp));
                    volatile auto testRead = *samples;
                } else {
                    // Start execution timer:
                    asm("mov %0, sp; eor r0, r0; svc 2" : "=r" (sp));
                    samples = entry(samples, size);
                    // Stop execution timer:
                    asm("mov r0, #1; svc 2; mov sp, %0" :: "r" (sp));
                    volatile auto testRead = *samples;
                } 
            }

            // Update the sample out buffer with the transformed samples.
            if (samples != nullptr) {
                if (MSG_FOR_FIRST(message))
                    Samples::Out.modify(samples, size);
                else
                    Samples::Out.midmodify(samples, size);
            }
        }
    }
}

void ConversionManager::adcReadHandler(adcsample_t *buffer, size_t)
{
    chSysLockFromISR();

    // If previous request hasn't been handled, then we're going too slow.
    // We'll need to abort.
    if (chMBGetUsedCountI(&m_mailbox) > 1) {
        chMBResetI(&m_mailbox);
        chMBResumeX(&m_mailbox);
        chSysUnlockFromISR();
        abort();
    } else {
        // Mark the modified samples as 'fresh' or ready for manipulation.
        if (buffer == Samples::In.data()) {
            Samples::In.setModified();
            chMBPostI(&m_mailbox, MSG_CONVFIRST);
        } else {
            Samples::In.setMidmodified();
            chMBPostI(&m_mailbox, MSG_CONVSECOND);
        }
        chSysUnlockFromISR();
    }
}

void ConversionManager::adcReadHandlerMeasure(adcsample_t *buffer, size_t)
{
    chSysLockFromISR();
    if (buffer == Samples::In.data()) {
        Samples::In.setModified();
        chMBPostI(&m_mailbox, MSG_CONVFIRST_MEASURE);
    } else {
        Samples::In.setMidmodified();
        chMBPostI(&m_mailbox, MSG_CONVSECOND_MEASURE);
    }
    chSysUnlockFromISR();

    ADC::setOperation(adcReadHandler);
}

