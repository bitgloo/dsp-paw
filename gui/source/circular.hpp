/**
 * @file circular.hpp
 * @brief Small utility for filling a buffer in a circular manner.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CIRCULAR_HPP
#define CIRCULAR_HPP

#include <iterator>

template<template<typename> class Container, typename T>
class CircularBuffer
{
public:
    CircularBuffer(Container<T>& container) :
        m_begin(std::begin(container)),
        m_end(std::end(container)),
        m_current(m_begin) {}

    void put(const T& value) noexcept {
        *m_current = value;
        if (++m_current == m_end)
            m_current = m_begin;
    }

    std::size_t size() const noexcept {
        return std::distance(m_begin, m_end);
    }

    void reset(const T& fill) noexcept {
        std::fill(m_begin, m_end, fill);
        m_current = m_begin;
    }

private:
    Container<T>::iterator m_begin;
    Container<T>::iterator m_end;
    Container<T>::iterator m_current;
};

#endif // CIRCULAR_HPP

