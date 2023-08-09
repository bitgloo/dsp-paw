/**
 * @file samplebuffer.hpp
 * @brief Manages ADC/DAC buffer data.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SAMPLEBUFFER_HPP_
#define SAMPLEBUFFER_HPP_

#include <array>
#include <cstdint>

using Sample = uint16_t;

constexpr unsigned int MAX_SAMPLE_BUFFER_BYTESIZE = sizeof(Sample) * 8192;
constexpr unsigned int MAX_SAMPLE_BUFFER_SIZE = MAX_SAMPLE_BUFFER_BYTESIZE / sizeof(Sample);

class SampleBuffer
{
public:
    SampleBuffer(Sample *buffer);

    void clear();

    void modify(Sample *data, unsigned int srcsize);
    void midmodify(Sample *data, unsigned int srcsize);
    void setModified();
    void setMidmodified();
    Sample *modified();

    Sample *data();
    Sample *middata();
    uint8_t *bytedata();

    void setSize(unsigned int size);
    unsigned int size() const;
    unsigned int bytesize() const;

private:
    Sample *m_buffer = nullptr;
    unsigned int m_size = MAX_SAMPLE_BUFFER_SIZE;
    Sample *m_modified = nullptr;
};

#endif // SAMPLEBUFFER_HPP_

