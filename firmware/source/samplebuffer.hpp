/**
 * @file samplebuffer.hpp
 * @brief Manages ADC/DAC buffer data.
 *
 * Copyright (C) 2023 Clyne Sullivan
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

/**
 * Manages a buffer of sample data from the ADC or DAC with facilities to
 * work with separate halves of the total buffer (which is necessary when
 * streaming data to or from the buffer).
 */
class SampleBuffer
{
public:
    // Manage the sample data memory at 'buffer'.
    explicit SampleBuffer(Sample *buffer);

    /**
     * Fill the current buffer with midpoint (2048/0V) values.
     */
    void clear();

    /**
     * Copy 'srcsize' samples from 'data' into the first half of the current
     * buffer. Also do equivalent of setModified().
     */
    void modify(Sample *data, unsigned int srcsize);

    /**
     * Copy 'srcsize' samples from 'data' into the second half of the current
     * buffer. Also do equivalent of setMidmodified().
     */
    void midmodify(Sample *data, unsigned int srcsize);

    /**
     * Set modified buffer pointer to first half of the current buffer.
     */
    void setModified();

    /**
     * Set modified buffer pointer to second half of the current buffer.
     */
    void setMidmodified();

    /**
     * Return pointer to most recently modified buffer portion.
     * Clears this internal pointer when called.
     */
    Sample *modified();

    /**
     * Returns pointer to first half of current buffer.
     */
    Sample *data();

    /**
     * Returns pointer to second half of current buffer.
     */
    Sample *middata();

    /**
     * Returns uint8_t-casted pointer to the current buffer.
     */
    uint8_t *bytedata();

    /**
     * Sets the total working size of the current buffer. Current buffer must
     * be able to accommodate.
     */
    void setSize(unsigned int size);

    /**
     * Returns the current total working size (number of samples).
     */
    unsigned int size() const;

    /**
     * Returns the current total working size (number of bytes).
     */
    unsigned int bytesize() const;

private:
    Sample *m_buffer = nullptr;
    unsigned int m_size = MAX_SAMPLE_BUFFER_SIZE;
    Sample *m_modified = nullptr;
};

#endif // SAMPLEBUFFER_HPP_

