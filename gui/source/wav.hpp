#ifndef WAV_HPP_
#define WAV_HPP_

#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace wav
{
    struct header {
        char riff[4];      // "RIFF"
        uint32_t filesize; // Total file size minus eight bytes
        char wave[4];      // "WAVE"

        bool valid() const {
            return strncmp(riff, "RIFF", 4) == 0 && filesize > 8 && strncmp(wave, "WAVE", 4) == 0;
        }
    } __attribute__ ((packed));

    struct format {
        char fmt_[4]; // "fmt "
        uint32_t size;
        uint16_t type;
        uint16_t channelcount;
        uint32_t samplerate;
        uint32_t byterate;
        uint16_t unused;
        uint16_t bps;

        bool valid() const {
            return strncmp(fmt_, "fmt ", 4) == 0;
        }
    } __attribute__ ((packed));

    struct data {
        char data[4]; // "data"
        uint32_t size;

        bool valid() const {
            return strncmp(data, "data", 4) == 0;
        }
    } __attribute__ ((packed));

    class clip {
    public:
        clip(const std::string& path) {
            std::ifstream file (path);
            if (!file.good())
                return;
            {
                header h;
                file.read(reinterpret_cast<char *>(&h), sizeof(header));
                if (!h.valid())
                    return;
            }
            {
                format f;
                file.read(reinterpret_cast<char *>(&f), sizeof(format));
                if (!f.valid() || f.type != 1) // ensure PCM
                    return;
            }
            {
                wav::data d;
                file.read(reinterpret_cast<char *>(&d), sizeof(wav::data));
                if (!d.valid())
                    return;
                m_data.resize(d.size / sizeof(int16_t));
                m_next = m_data.begin();
                file.read(reinterpret_cast<char *>(m_data.data()), d.size);
            }
        }
        clip() = default;

        bool valid() const {
            return !m_data.empty();
        }
        const int16_t *data() const {
            return m_data.data();
        }
        void next(int16_t *buf, unsigned int size) {
            for (unsigned int i = 0; i < size; ++i) {
                if (m_next == m_data.end())
                    m_next = m_data.begin();
                else
                    *buf++ = *m_next++;
            }
        }
    private:
        std::vector<int16_t> m_data;
        decltype(m_data.begin()) m_next;
    };
}

#endif // WAV_HPP_

