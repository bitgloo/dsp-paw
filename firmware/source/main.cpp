/**
 * @file main.cpp
 * @brief Program entry point.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "ch.h"
#include "hal.h"

#include "adc.hpp"
#include "cordic.hpp"
#include "dac.hpp"
#include "error.hpp"
#include "sclock.hpp"
#include "usbserial.hpp"

#include "runstatus.hpp"
RunStatus run_status = RunStatus::Idle;

// Other variables
//
//static char userMessageBuffer[128];
//static unsigned char userMessageSize = 0;

#include "conversion.hpp"
#include "communication.hpp"
#include "monitor.hpp"

int main()
{
    // Initialize ChibiOS
    halInit();
    chSysInit();

    // Init peripherials
    ADC::begin();
    DAC::begin();
    SClock::begin();
    USBSerial::begin();
    cordic::init();

    SClock::setRate(SClock::Rate::R32K);
    ADC::setRate(SClock::Rate::R32K);

    // Start our threads.
    ConversionManager::begin();
    CommunicationManager::begin();
    Monitor::begin();

    chThdExit(0);
    return 0;
}

