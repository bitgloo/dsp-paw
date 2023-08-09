/**
 * @file samplebuffer.cpp
 * @brief Manages ADC/DAC buffer data.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "samplebuffer.hpp"

SampleBuffer::SampleBuffer(Sample *buffer) :
    m_buffer(buffer) {}

void SampleBuffer::clear() {
    std::fill(m_buffer, m_buffer + m_size, 2048);
}
__attribute__((section(".convcode")))
void SampleBuffer::modify(Sample *data, unsigned int srcsize) {
    auto size = srcsize < m_size ? srcsize : m_size;
    size = (size + 15) & (~15);

    m_modified = m_buffer;
    const int *src = reinterpret_cast<const int *>(data);
    const int * const srcend = src + (size / 2);
    int *dst = reinterpret_cast<int *>(m_buffer);
    do {
        int a = src[0];
        int b = src[1];
        int c = src[2];
        int d = src[3];
        int e = src[4];
        int f = src[5];
        int g = src[6];
        int h = src[7];
        dst[0] = a;
        dst[1] = b;
        dst[2] = c;
        dst[3] = d;
        dst[4] = e;
        dst[5] = f;
        dst[6] = g;
        dst[7] = h;
        src += 8;
        dst += 8;
    } while (src < srcend);
}
__attribute__((section(".convcode")))
void SampleBuffer::midmodify(Sample *data, unsigned int srcsize) {
    auto size = srcsize < m_size / 2 ? srcsize : m_size / 2;
    size = (size + 15) & (~15);

    m_modified = middata();
    const int *src = reinterpret_cast<const int *>(data);
    const int * const srcend = src + (size / 2);
    int *dst = reinterpret_cast<int *>(middata());
    do {
        int a = src[0];
        int b = src[1];
        int c = src[2];
        int d = src[3];
        int e = src[4];
        int f = src[5];
        int g = src[6];
        int h = src[7];
        dst[0] = a;
        dst[1] = b;
        dst[2] = c;
        dst[3] = d;
        dst[4] = e;
        dst[5] = f;
        dst[6] = g;
        dst[7] = h;
        src += 8;
        dst += 8;
    } while (src < srcend);
}

void SampleBuffer::setModified() {
    m_modified = m_buffer;
}

void SampleBuffer::setMidmodified() {
    m_modified = middata();
}

void SampleBuffer::setSize(unsigned int size) {
    m_size = size < MAX_SAMPLE_BUFFER_SIZE ? size : MAX_SAMPLE_BUFFER_SIZE;
}

__attribute__((section(".convcode")))
Sample *SampleBuffer::data() {
    return m_buffer;
}
__attribute__((section(".convcode")))
Sample *SampleBuffer::middata() {
    return m_buffer + m_size / 2;
}
uint8_t *SampleBuffer::bytedata() {
    return reinterpret_cast<uint8_t *>(m_buffer);
}

Sample *SampleBuffer::modified() {
    auto m = m_modified;
    m_modified = nullptr;
    return m;
}
__attribute__((section(".convcode")))
unsigned int SampleBuffer::size() const {
    return m_size;
}
unsigned int SampleBuffer::bytesize() const {
    return m_size * sizeof(Sample);
}

