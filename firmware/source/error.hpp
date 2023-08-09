/**
 * @file error.hpp
 * @brief Tracks and reports non-critical errors.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STMDSP_ERROR_HPP
#define STMDSP_ERROR_HPP

#include <array>

enum class Error : char
{
    None = 0,
    BadParam,
    BadParamSize,
    BadUserCodeLoad,
    BadUserCodeSize,
    NotIdle,
    ConversionAborted,
    NotRunning
};

class ErrorManager
{
    constexpr static unsigned int MAX_ERROR_QUEUE_SIZE = 8;

public:
    void add(Error error);
    bool assert(bool condition, Error error);
    bool hasError();
    Error pop();

private:
    std::array<Error, MAX_ERROR_QUEUE_SIZE> m_queue;
    unsigned int m_index = 0;
};

extern ErrorManager EM;

#endif // STMDSP_ERROR_HPP

