/**
 * @file monitor.cpp
 * @brief Manages the device monitoring thread (status LEDs, etc.).
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "monitor.hpp"

#include "error.hpp"
#include "runstatus.hpp"

#include "hal.h"

__attribute__((section(".stacks")))
std::array<char, THD_WORKING_AREA_SIZE(256)> Monitor::m_thread_stack = {};

void Monitor::begin()
{
    chThdCreateStatic(m_thread_stack.data(),
                      m_thread_stack.size(),
                      LOWPRIO,
                      threadMonitor,
                      nullptr);
}

void Monitor::threadMonitor(void *)
{
    palSetLineMode(LINE_BUTTON, PAL_MODE_INPUT_PULLUP);
    auto readButton = [] {
#ifdef TARGET_PLATFORM_L4
        return !palReadLine(LINE_BUTTON);
#else
        return palReadLine(LINE_BUTTON);
#endif
    };

    palSetLine(LINE_LED_RED);
    palSetLine(LINE_LED_GREEN);
    palSetLine(LINE_LED_BLUE);

    while (1) {
        bool isidle = run_status == RunStatus::Idle;
        auto led = isidle ? LINE_LED_GREEN : LINE_LED_BLUE;
        auto delay = isidle ? 500 : 250;

        palToggleLine(led);
        chThdSleepMilliseconds(delay);
        palToggleLine(led);
        chThdSleepMilliseconds(delay);

        if (isidle && readButton()) {
            palClearLine(LINE_LED_GREEN);
            palClearLine(LINE_LED_BLUE);
            chSysLock();
            while (readButton())
                asm("nop");
            while (!readButton())
                asm("nop");
            chSysUnlock();
            palSetLine(LINE_LED_GREEN);
            palSetLine(LINE_LED_BLUE);
            chThdSleepMilliseconds(500);
        }

        static bool erroron = false;
        if (auto err = EM.hasError(); err ^ erroron) {
            erroron = err;
            if (err)
                palClearLine(LINE_LED_RED);
            else
                palSetLine(LINE_LED_RED);
        }
    }
}

