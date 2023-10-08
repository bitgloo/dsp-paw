/**
 * @file dac.hpp
 * @brief Manages signal creation using the DAC.
 *
 * Copyright (C) 2023 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STMDSP_DAC_HPP_
#define STMDSP_DAC_HPP_

#include "hal.h"
#undef DAC

class DAC
{
public:
    /**
     * Initializes DAC output pins and peripheral.
     */
    static void begin();

    /**
     * Begins continuous DAC conversion on the given channel, running at the
     * current SClock sampling rate.
     * @param channel Selected output channel (0 = sig. out, 1 = sig. gen.).
     * @param buffer Buffer of sample data to output.
     * @param count Number of samples in sample buffer.
     */
    static void start(int channel, dacsample_t *buffer, size_t count);

    /**
     * Stops DAC conversion on the given channel.
     */
    static void stop(int channel);

    /**
     * Determines if signal generator needs more sample data for streamed
     * output (i.e. audio file streaming).
     * @return >0 if samples needed, <0 on initial run.
     */
    static int sigGenWantsMore();

    /**
     * Returns true if signal generator is currently running.
     */
    static int isSigGenRunning();

private:
    static DACDriver *m_driver[2];

    static const DACConfig m_config;
    static const DACConversionGroup m_group_config;
};

#endif // STMDSP_DAC_HPP_

