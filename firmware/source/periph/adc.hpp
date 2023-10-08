/**
 * @file adc.hpp
 * @brief Manages signal reading through the ADC.
 *
 * Copyright (C) 2023 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STMDSP_ADC_HPP_
#define STMDSP_ADC_HPP_

#include "hal.h"
#include "sclock.hpp"

#include <array>

class ADC
{
public:
    using Operation = void (*)(adcsample_t *buffer, size_t count);

    /**
     * Initializes analog input pins and the microcontoller's ADC peripheral.
     */
    static void begin();

    /**
     * Begins continuous ADC sampling triggered by SClock at the set sampling
     * rate.
     * @param buffer Pointer to buffer for sample data.
     * @param count Number of samples that the buffer can hold.
     * @param operation Handler function to operate on filled half-buffers.
     */
    static void start(adcsample_t *buffer, size_t count, Operation operation);

    /**
     * Stops the continuous ADC sampling.
     */
    static void stop();

    /**
     * Runs a single conversion on the "alt" inputs (parameter knobs).
     * @param id The ID of the desired "alt" input (zero or one).
     * @return The sampled value for the given input.
     */
    static adcsample_t readAlt(unsigned int id);

    /**
     * Sets the desired sampling rate for the ADC to operate at.
     */
    static void setRate(SClock::Rate rate);

    /**
     * Used to override the handler function for the currently running
     * conversion.
     */
    static void setOperation(Operation operation);

private:
    // ADC driver for signal input.
    static ADCDriver *m_driver;
    // ADC driver for "alt" inputs.
    static ADCDriver *m_driver2;

    static const ADCConfig m_config;
    static const ADCConfig m_config2;
    static ADCConversionGroup m_group_config;
    static ADCConversionGroup m_group_config2;

    static adcsample_t *m_current_buffer;
    static size_t m_current_buffer_size;
    static Operation m_operation;

public:
    static void conversionCallback(ADCDriver *);
};

#endif // STMDSP_ADC_HPP_

