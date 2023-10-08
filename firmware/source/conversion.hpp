/**
 * @file conversion.hpp
 * @brief Manages algorithm application (converts input samples to output).
 *
 * Copyright (C) 2023 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STMDSP_CONVERSION_HPP
#define STMDSP_CONVERSION_HPP

#include "ch.h"
#include "hal.h"

#include <array>

constexpr unsigned int CONVERSION_THREAD_STACK_SIZE = 
#if defined(TARGET_PLATFORM_H7)
                                                  62 * 1024;
#else
                                                  15 * 1024;
#endif

class ConversionManager
{
public:
    /**
     * Starts two threads: the privileged monitor thread and the unprivileged
     * algorithm execution thread.
     */
    static void begin();

    // Begins sample conversion.
    static void start();
    // Prepare to measure execution time of next conversion.
    static void startMeasurement();
    // Stops conversion.
    static void stop();

    static thread_t *getMonitorHandle();

    // Internal only: Aborts a running conversion.
    static void abort(bool fpu_stacked = true);

private:
    static void threadMonitor(void *);
    static void threadRunnerEntry(void *stack);

    static void threadRunner(void *);
    static void adcReadHandler(adcsample_t *buffer, size_t);
    static void adcReadHandlerMeasure(adcsample_t *buffer, size_t);

    static thread_t *m_thread_monitor;
    static thread_t *m_thread_runner;

    static std::array<char, 1024> m_thread_monitor_stack;
    static std::array<char, THD_WORKING_AREA_SIZE(128)> m_thread_runner_entry_stack;
    static std::array<char, CONVERSION_THREAD_STACK_SIZE> m_thread_runner_stack;

    static std::array<msg_t, 2> m_mailbox_buffer;
    static mailbox_t m_mailbox;
};

#endif // STMDSP_CONVERSION_HPP

