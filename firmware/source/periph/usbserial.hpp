/**
 * @file usbserial.hpp
 * @brief Wrapper for ChibiOS's SerialUSBDriver.
 *
 * Copyright (C) 2021 Clyne Sullivan
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
    static void begin();

    static bool isActive();

    static size_t read(unsigned char *buffer, size_t count);
    static size_t write(const unsigned char *buffer, size_t count);

private:
    static SerialUSBDriver *m_driver;
};

#endif // STMDSP_USBSERIAL_HPP_

