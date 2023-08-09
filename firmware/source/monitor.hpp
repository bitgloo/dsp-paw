/**
 * @file monitor.hpp
 * @brief Manages the device monitoring thread (status LEDs, etc.).
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STMDSP_MONITOR_HPP
#define STMDSP_MONITOR_HPP

#include "ch.h"

#include <array>

class Monitor
{
public:
    static void begin();

private:
    static void threadMonitor(void *);

    static std::array<char, THD_WORKING_AREA_SIZE(256)> m_thread_stack;
};

#endif // STMDSP_MONITOR_HPP

