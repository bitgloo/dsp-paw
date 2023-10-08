/**
 * @file usbserial.hpp
 * @brief Wrapper for ChibiOS's SerialUSBDriver.
 *
 * Copyright (C) 2023 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STMDSP_USBSERIAL_HPP_
#define STMDSP_USBSERIAL_HPP_

#include "usbcfg.h"

class USBSerial
{
public:
    /**
     * Prepares for USB serial communication.
     */
    static void begin();

    /**
     * Returns true if input data has been received.
     */
    static bool isActive();

    /**
     * Reads received input data into the given buffer.
     * @param buffer Buffer to store input data.
     * @param count Number of bytes to read.
     * @return Number of bytes actually read.
     */
    static size_t read(unsigned char *buffer, size_t count);

    /**
     * Writes data to serial output.
     * @param buffer Buffer of output data.
     * @param count Number of bytes to write.
     * @return Number of bytes actually written.
     */
    static size_t write(const unsigned char *buffer, size_t count);

private:
    static SerialUSBDriver *m_driver;
};

#endif // STMDSP_USBSERIAL_HPP_

