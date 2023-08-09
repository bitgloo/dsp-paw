/**
 * @file stmdsp.cpp
 * @brief Interface for communication with stmdsp device over serial.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "stmdsp.hpp"

#include <serial/serial.h>

#include <algorithm>
#include <array>

extern void log(const std::string& str);

std::array<unsigned int, 6> sampleRateInts {{
    8'000,
    16'000,
    20'000,
    32'000,
    48'000,
    96'000
}};

namespace stmdsp
{
    const std::forward_list<std::string>& scanner::scan()
    {
        auto devices = serial::list_ports();
        auto foundDevicesEnd = std::remove_if(
            devices.begin(), devices.end(),
            [](const auto& dev) {
                return dev.hardware_id.find(STMDSP_USB_ID) == std::string::npos;
            });
        std::transform(devices.begin(), foundDevicesEnd,
            std::front_inserter(m_available_devices),
            [](const auto& dev) { return dev.port; });
        return m_available_devices;
    }

    device::device(const std::string& file)
    {
        // This could throw!
	// Note: Windows needs a not-simple, positive timeout like this to
	// ensure that reads block.
        m_serial.reset(new serial::Serial(file, 921'600 /*8'000'000*/, serial::Timeout(1000, 1000, 1, 1000, 1)));

        // Test the ID command.
        m_serial->flush();
        m_serial->write("i");
        auto id = m_serial->read(7);

        if (id.starts_with("stmdsp")) {
            if (id.back() == 'h')
                m_platform = platform::H7;
            else if (id.back() == 'l')
                m_platform = platform::L4;
            else
                m_serial.release();
        } else {
            m_serial.release();
        }
    }

    device::~device()
    {
        disconnect();
    }

    bool device::connected() {
        if (m_serial && !m_serial->isOpen())
            m_serial.release();

        return m_serial ? true : false;
    }

    void device::disconnect() {
        if (m_serial)
            m_serial.release();
    }

    bool device::try_command(std::basic_string<uint8_t> cmd) {
        bool success = false;

        if (connected()) {
            try {
                std::scoped_lock lock (m_lock);
                m_serial->write(cmd.data(), cmd.size());
                success = true;
            } catch (...) {
                handle_disconnect();
            }
        }

        return success;
    }

    bool device::try_read(std::basic_string<uint8_t> cmd, uint8_t *dest, unsigned int dest_size) {
        bool success = false;

        if (connected() && dest && dest_size > 0) {
            try {
                std::scoped_lock lock (m_lock);
                m_serial->write(cmd.data(), cmd.size());
                m_serial->read(dest, dest_size);
                success = true;
            } catch (...) {
                handle_disconnect();
            }
        }

        return success;
    }

    void device::continuous_set_buffer_size(unsigned int size) {
        if (try_command({
                'B',
                static_cast<uint8_t>(size),
                static_cast<uint8_t>(size >> 8)}))
        {
            m_buffer_size = size;
        }
    }

    void device::set_sample_rate(unsigned int rate) {
        auto it = std::find(
            sampleRateInts.cbegin(),
            sampleRateInts.cend(),
            rate);

        if (it != sampleRateInts.cend()) {
            const auto i = std::distance(sampleRateInts.cbegin(), it);
            try_command({
                'r',
                static_cast<uint8_t>(i)
            });
        }
    }

    unsigned int device::get_sample_rate() {
        if (!is_running()) {
            uint8_t result = 0xFF;
            if (try_read({'r', 0xFF}, &result, 1))
                m_sample_rate = result;
        }

        return m_sample_rate < sampleRateInts.size() ?
            sampleRateInts[m_sample_rate] :
            0;
    }

    void device::continuous_start() {
        if (try_command({'R'}))
            m_is_running = true;
    }

    void device::measurement_start() {
        try_command({'M'});
    }

    uint32_t device::measurement_read() {
        uint32_t count = 0;
        try_read({'m'}, reinterpret_cast<uint8_t *>(&count), sizeof(uint32_t));
        return count / 2;
    }

    std::vector<adcsample_t> device::continuous_read() {
        if (connected()) {
            try {
                m_serial->write("s");
                unsigned char sizebytes[2];
                m_serial->read(sizebytes, 2);
                unsigned int size = sizebytes[0] | (sizebytes[1] << 8);
                if (size > 0) {
                    std::vector<adcsample_t> data (size);
                    unsigned int total = size * sizeof(adcsample_t);
                    unsigned int offset = 0;

                    while (total > 512) {
                        m_serial->read(reinterpret_cast<uint8_t *>(&data[0]) + offset, 512);
                        m_serial->write("n");
                        offset += 512;
                        total -= 512;
                    }
                    m_serial->read(reinterpret_cast<uint8_t *>(&data[0]) + offset, total);
                    m_serial->write("n");
                    return data;

                }
            } catch (...) {
                handle_disconnect();
            }
        }

        return {};
    }

    std::vector<adcsample_t> device::continuous_read_input() {
        if (connected()) {
            try {
                m_serial->write("t");
                unsigned char sizebytes[2];
                m_serial->read(sizebytes, 2);
                unsigned int size = sizebytes[0] | (sizebytes[1] << 8);
                if (size > 0) {
                    std::vector<adcsample_t> data (size);
                    unsigned int total = size * sizeof(adcsample_t);
                    unsigned int offset = 0;

                    while (total > 512) {
                        m_serial->read(reinterpret_cast<uint8_t *>(&data[0]) + offset, 512);
                        m_serial->write("n");
                        offset += 512;
                        total -= 512;
                    }
                    m_serial->read(reinterpret_cast<uint8_t *>(&data[0]) + offset, total);
                    m_serial->write("n");
                    return data;

                }
            } catch (...) {
                handle_disconnect();
            }
        }

        return {};
    }

    void device::continuous_stop() {
        if (try_command({'S'}))
            m_is_running = false;
    }

    bool device::siggen_upload(dacsample_t *buffer, unsigned int size) {
        if (connected()) {
            uint8_t request[3] = {
                'D',
                static_cast<uint8_t>(size),
                static_cast<uint8_t>(size >> 8)
            };

            if (!m_is_siggening) {
                try {
                    m_serial->write(request, 3);
                    m_serial->write((uint8_t *)buffer, size * sizeof(dacsample_t));
                } catch (...) {
                    handle_disconnect();
                }
            } else {
                try {
                    m_serial->write(request, 3);
                    if (m_serial->read(1)[0] == 0)
                        return false;
                    else
                        m_serial->write((uint8_t *)buffer, size * sizeof(dacsample_t));
                } catch (...) {
                    handle_disconnect();
                }
            }

            return true;
        } else {
            return false;
        }
    }

    void device::siggen_start() {
        if (try_command({'W'}))
            m_is_siggening = true;
    }

    void device::siggen_stop() {
        if (try_command({'w'}))
            m_is_siggening = false;
    }

    void device::upload_filter(unsigned char *buffer, size_t size) {
        if (connected()) {
            uint8_t request[3] = {
                'E',
                static_cast<uint8_t>(size),
                static_cast<uint8_t>(size >> 8)
            };

            try {
                m_serial->write(request, 3);
                m_serial->write(buffer, size);
            } catch (...) {
                handle_disconnect();
            }
        }
    }

    void device::unload_filter() {
        try_command({'e'});
    }

    std::pair<RunStatus, Error> device::get_status() {
        std::pair<RunStatus, Error> ret;

        unsigned char buf[2];
        if (try_read({'I'}, buf, 2)) {
            ret = {
                static_cast<RunStatus>(buf[0]),
                static_cast<Error>(buf[1])
            };

            bool running = ret.first == RunStatus::Running;
            if (m_is_running != running)
                m_is_running = running;
        } else if (m_disconnect_error_flag) {
            m_disconnect_error_flag = false;
            return {RunStatus::Idle, Error::GUIDisconnect};
        }

        return ret;
    }

    void device::handle_disconnect()
    {
        m_disconnect_error_flag = true;
        m_serial.release();
        log("Lost connection!");
    }
} // namespace stmdsp

