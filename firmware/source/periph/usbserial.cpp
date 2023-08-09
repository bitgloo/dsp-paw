/**
 * @file usbserial.cpp
 * @brief Wrapper for ChibiOS's SerialUSBDriver.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "usbserial.hpp"

SerialUSBDriver *USBSerial::m_driver = &SDU1;

void USBSerial::begin()
{
    palSetPadMode(GPIOA, 11, PAL_MODE_ALTERNATE(10));
    palSetPadMode(GPIOA, 12, PAL_MODE_ALTERNATE(10));

    sduObjectInit(m_driver);
    sduStart(m_driver, &serusbcfg);

    // Reconnect bus so device can re-enumerate on reset
    usbDisconnectBus(serusbcfg.usbp);
    chThdSleepMilliseconds(1500);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);
}

bool USBSerial::isActive()
{
    if (auto config = m_driver->config; config != nullptr) {
        if (auto usbp = config->usbp; usbp != nullptr)
            return usbp->state == USB_ACTIVE && !ibqIsEmptyI(&m_driver->ibqueue);
    }

    return false;
}

size_t USBSerial::read(unsigned char *buffer, size_t count)
{
    auto bss = reinterpret_cast<BaseSequentialStream *>(m_driver);
    return streamRead(bss, buffer, count);
}

size_t USBSerial::write(const unsigned char *buffer, size_t count)
{
    auto bss = reinterpret_cast<BaseSequentialStream *>(m_driver);
    return streamWrite(bss, buffer, count);
}

