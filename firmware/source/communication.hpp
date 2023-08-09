/**
 * @file communication.hpp
 * @brief Manages communication with the host computer.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STMDSP_COMMUNICATION_HPP
#define STMDSP_COMMUNICATION_HPP

#include <array>

class CommunicationManager
{
public:
    static void begin();

private:
    static void threadComm(void *);

    static std::array<char, 4096> m_thread_stack;
};

#endif // STMDSP_COMMUNICATION_HPP

