/**
 * @file error.cpp
 * @brief Tracks and reports non-critical errors.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "error.hpp"

ErrorManager EM;

void ErrorManager::add(Error error)
{ 
    if (m_index < m_queue.size())
        m_queue[m_index++] = error;
}

bool ErrorManager::assert(bool condition, Error error)
{
    if (!condition)
        add(error);
    return condition;
}

bool ErrorManager::hasError()
{
    return m_index > 0;
}

Error ErrorManager::pop()
{
    return m_index == 0 ? Error::None : m_queue[--m_index];
}

