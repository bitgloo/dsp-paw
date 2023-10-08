/**
 * @file sclock.hpp
 * @brief Manages sampling rate clock speeds.
 *
 * Copyright (C) 2023 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SCLOCK_HPP_
#define SCLOCK_HPP_

#include "hal.h"

#include <array>

class SClock
{
public:
    // These are the supported sampling rates. GUI keeps its own enumeration.
    enum class Rate : unsigned int {
        R8K = 0,
        R16K,
        R20K,
        R32K,
        R48K,
        R96K
    };

    /**
     * Initializes the sample clock hardware.
     */
    static void begin();

    /**
     * Starts the sample rate clock if it is not already running.
     */
    static void start();

    /**
     * Indicate that the caller no longer needs the sample clock.
     * This decrements an internal counter that is incremented by start()
     * calls; if the counter reaches zero, the clock will actually stop.
     */
    static void stop();

    /**
     * Sets the desired sampling rate, used for the next start() call.
     */
    static void setRate(Rate rate);

    /**
     * Gets the desired sampling rate (SClock::Rate value) casted to an
     * unsigned int.
     */
    static unsigned int getRate();

private:
    static GPTDriver *m_timer;
    static unsigned int m_div;
    static unsigned int m_runcount;
    static const GPTConfig m_timer_config;
    static const std::array<unsigned int, 6> m_rate_divs;
};

#endif // SCLOCK_HPP_

