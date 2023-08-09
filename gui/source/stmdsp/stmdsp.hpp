/**
 * @file stmdsp.hpp
 * @brief Interface for communication with stmdsp device over serial.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STMDSP_HPP_
#define STMDSP_HPP_

#include <serial/serial.h>

#include <cstdint>
#include <forward_list>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>

namespace stmdsp
{
    /**
     * The largest possible size of an ADC or DAC sample buffer, as a sample count.
     * Maximum byte size would be `SAMPLES_MAX * sizeof(XXXsample_t)`.
     */
    constexpr unsigned int SAMPLES_MAX = 4096;

    /**
     * ADC samples on all platforms are stored as 16-bit unsigned integers.
     */
    using adcsample_t = uint16_t;
    /**
     * DAC samples on all platforms are stored as 16-bit unsigned integers.
     */
    using dacsample_t = uint16_t;

    /**
     * List of all available platforms.
     * Note that some platforms in this list may not have complete support.
     */
    enum class platform {
        Unknown,
        H7, /* Some feature support */
        L4, /* Complete feature support */
        G4  /* Unsupported, but planned */
    };

    /**
     * Run status states, valued to match what the stmdsp firmware reports.
     */
    enum class RunStatus : char {
        Idle = '1', /* Device ready for commands or execution. */
        Running,    /* Device currently executing its algorithm. */
        Recovering  /* Device recovering from fault caused by algorithm. */
    };

    /**
     * Error messages that are reported by the firmware.
     */
    enum class Error : char {
        None = 0,
        BadParam,            /* An invalid parameter was passed for a command. */
        BadParamSize,        /* An invaild param. size was given for a command. */
        BadUserCodeLoad,     /* Device failed to load the given algorithm. */
        BadUserCodeSize,     /* The given algorithm is too large for the device. */
        NotIdle,             /* An idle-only command was received while not Idle. */
        ConversionAborted,   /* A conversion was aborted due to a fault. */
        NotRunning,          /* A running-only command was received while not Running. */

        GUIDisconnect = 100  /* The GUI lost connection with the device. */
    };

    /**
     * Provides functionality to scan the system for stmdsp devices.
     * A list of devices is returned, though the GUI only interacts with one
     * device at a time.
     */
    class scanner
    {
    public:
        /**
         * Scans for connected devices, returning a list of ports with
         * connected stmdsp devices.
         */
        const std::forward_list<std::string>& scan();

        /**
         * Retrieves the results of the last scan().
         */
        const std::forward_list<std::string>& devices() const noexcept {
            return m_available_devices;
        }

    private:
        constexpr static const char *STMDSP_USB_ID =
#ifndef STMDSP_WIN32
            "USB VID:PID=0483:5740";
#else
            "USB\\VID_0483&PID_5740";
#endif

        std::forward_list<std::string> m_available_devices;
    };

    class device
    {
    public:
        device(const std::string& file);
        ~device();

        bool connected();
        void disconnect();

        auto get_platform() const { return m_platform; }

        void continuous_set_buffer_size(unsigned int size);
        unsigned int get_buffer_size() const { return m_buffer_size; }

        void set_sample_rate(unsigned int rate);
        unsigned int get_sample_rate();

        void continuous_start();
        void continuous_stop();

        void measurement_start();
        uint32_t measurement_read();

        std::vector<adcsample_t> continuous_read();
        std::vector<adcsample_t> continuous_read_input();

        bool siggen_upload(dacsample_t *buffer, unsigned int size);
        void siggen_start();
        void siggen_stop();

        bool is_siggening() const { return m_is_siggening; }
        bool is_running() const { return m_is_running; }

        // buffer is ELF binary
        void upload_filter(unsigned char *buffer, size_t size);
        void unload_filter();

        std::pair<RunStatus, Error> get_status();

    private:
        std::unique_ptr<serial::Serial> m_serial;
        platform m_platform = platform::Unknown;
        unsigned int m_buffer_size = SAMPLES_MAX;
        unsigned int m_sample_rate = 0;
        bool m_is_siggening = false;
        bool m_is_running = false;
        bool m_disconnect_error_flag = false;

        std::mutex m_lock;

        bool try_command(std::basic_string<uint8_t> data);
        bool try_read(std::basic_string<uint8_t> cmd, uint8_t *dest, unsigned int dest_size);
        void handle_disconnect();
    };
}

#endif // STMDSP_HPP_

