/**
 * @file adc.cpp
 * @brief Manages signal reading through the ADC.
 *
 * Copyright (C) 2021 Clyne Sullivan
 *
 * Distributed under the GNU GPL v3 or later. You should have received a copy of
 * the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "adc.hpp"

#if defined(TARGET_PLATFORM_L4)
ADCDriver *ADC::m_driver = &ADCD1;
ADCDriver *ADC::m_driver2 = &ADCD3;
#else
ADCDriver *ADC::m_driver = &ADCD3;
//ADCDriver *ADC::m_driver2 = &ADCD1; // TODO
#endif

const ADCConfig ADC::m_config = {
    .difsel = 0,
#if defined(TARGET_PLATFORM_H7)
    .calibration = 0,
#endif
};

const ADCConfig ADC::m_config2 = {
    .difsel = 0,
#if defined(TARGET_PLATFORM_H7)
    .calibration = 0,
#endif
};

ADCConversionGroup ADC::m_group_config = {
    .circular = true,
    .num_channels = 1,
    .end_cb = ADC::conversionCallback,
    .error_cb = nullptr,
    .cfgr = ADC_CFGR_EXTEN_RISING | ADC_CFGR_EXTSEL_SRC(13),  /* TIM6_TRGO */
    .cfgr2 = 0,//ADC_CFGR2_ROVSE | ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSS_0, // Oversampling 2x
#if defined(TARGET_PLATFORM_H7)
    .ccr = 0,
    .pcsel = 0,
    .ltr1 = 0, .htr1 = 4095,
    .ltr2 = 0, .htr2 = 4095,
    .ltr3 = 0, .htr3 = 4095,
#else
    .tr1 = ADC_TR(0, 4095),
    .tr2 = ADC_TR(0, 4095),
    .tr3 = ADC_TR(0, 4095),
    .awd2cr = 0,
    .awd3cr = 0,
#endif
    .smpr = {
        ADC_SMPR1_SMP_AN5(ADC_SMPR_SMP_12P5), 0
    },
    .sqr = {
        ADC_SQR1_SQ1_N(ADC_CHANNEL_IN5),
        0, 0, 0
    },
};

static bool readAltDone = false;
static void readAltCallback(ADCDriver *)
{
    readAltDone = true;
}
ADCConversionGroup ADC::m_group_config2 = {
    .circular = false,
    .num_channels = 2,
    .end_cb = readAltCallback,
    .error_cb = nullptr,
    .cfgr = ADC_CFGR_EXTEN_RISING | ADC_CFGR_EXTSEL_SRC(13),  /* TIM6_TRGO */
    .cfgr2 = 0,//ADC_CFGR2_ROVSE | ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSS_0, // Oversampling 2x
#if defined(TARGET_PLATFORM_H7)
    .ccr = 0,
    .pcsel = 0,
    .ltr1 = 0, .htr1 = 4095,
    .ltr2 = 0, .htr2 = 4095,
    .ltr3 = 0, .htr3 = 4095,
#else
    .tr1 = ADC_TR(0, 4095),
    .tr2 = ADC_TR(0, 4095),
    .tr3 = ADC_TR(0, 4095),
    .awd2cr = 0,
    .awd3cr = 0,
#endif
    .smpr = {
        ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_2P5) | ADC_SMPR1_SMP_AN2(ADC_SMPR_SMP_2P5), 0
    },
    .sqr = {
        ADC_SQR1_SQ1_N(ADC_CHANNEL_IN2) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN1),
        0, 0, 0
    },
};

adcsample_t *ADC::m_current_buffer = nullptr;
size_t ADC::m_current_buffer_size = 0;
ADC::Operation ADC::m_operation = nullptr;

void ADC::begin()
{
#if defined(TARGET_PLATFORM_H7)
    palSetPadMode(GPIOF, 3, PAL_MODE_INPUT_ANALOG);
#else
    palSetPadMode(GPIOA, 0, PAL_MODE_INPUT_ANALOG); // Algorithm in
    palSetPadMode(GPIOC, 0, PAL_MODE_INPUT_ANALOG); // Potentiometer 1
    palSetPadMode(GPIOC, 1, PAL_MODE_INPUT_ANALOG); // Potentiometer 2
#endif

    adcStart(m_driver, &m_config);
    adcStart(m_driver2, &m_config2);
}

void ADC::start(adcsample_t *buffer, size_t count, Operation operation)
{
    m_current_buffer = buffer;
    m_current_buffer_size = count;
    m_operation = operation;

    adcStartConversion(m_driver, &m_group_config, buffer, count);
    SClock::start();
}

void ADC::stop()
{
    SClock::stop();
    adcStopConversion(m_driver);

    m_current_buffer = nullptr;
    m_current_buffer_size = 0;
    m_operation = nullptr;
}

adcsample_t ADC::readAlt(unsigned int id)
{
    if (id > 1)
        return 0;
    static adcsample_t result[16] = {};
    readAltDone = false;
    adcStartConversion(m_driver2, &m_group_config2, result, 8);
    while (!readAltDone);
        //__WFI();
    adcStopConversion(m_driver2);
    return result[id];
}

void ADC::setRate(SClock::Rate rate)
{
#if defined(TARGET_PLATFORM_H7)
    std::array<std::array<uint32_t, 2>, 6> m_rate_presets = {{
         // Rate   PLL N  PLL P
        {/* 8k  */ 80,    20},
        {/* 16k */ 80,    10},
        {/* 20k */ 80,    8},
        {/* 32k */ 80,    5},
        {/* 48k */ 96,    4},
        {/* 96k */ 288,   10}
    }};

    auto& preset = m_rate_presets[static_cast<unsigned int>(rate)];
    auto pllbits = (preset[0] << RCC_PLL2DIVR_N2_Pos) |
                   (preset[1] << RCC_PLL2DIVR_P2_Pos);

    adcStop(m_driver);

    // Adjust PLL2
    RCC->CR &= ~(RCC_CR_PLL2ON);
    while ((RCC->CR & RCC_CR_PLL2RDY) == RCC_CR_PLL2RDY);
    auto pll2divr = RCC->PLL2DIVR &
                    ~(RCC_PLL2DIVR_N2_Msk | RCC_PLL2DIVR_P2_Msk);
    pll2divr |= pllbits;
    RCC->PLL2DIVR = pll2divr;
    RCC->CR |= RCC_CR_PLL2ON;
    while ((RCC->CR & RCC_CR_PLL2RDY) != RCC_CR_PLL2RDY);

    m_group_config.smpr[0] = rate != SClock::Rate::R96K ? ADC_SMPR1_SMP_AN5(ADC_SMPR_SMP_12P5)
                                                        : ADC_SMPR1_SMP_AN5(ADC_SMPR_SMP_2P5);

    adcStart(m_driver, &m_config);
#elif defined(TARGET_PLATFORM_L4)
    std::array<std::array<uint32_t, 3>, 6> m_rate_presets = {{
        // PLLSAI2 sources MSI of 4MHz, divided by PLLM of /1 = 4MHz.
        // 4MHz is then multiplied by PLLSAI2N (x8 to x86), with result
        // between 64 and 344 MHz.
        //
        // SAI2N MUST BE AT LEAST 16 TO MAKE 64MHz MINIMUM.
        //
        // That is then divided by PLLSAI2R:
        //     R of 0 = /2; 1 = /4, 2 = /6, 3 = /8.
        // PLLSAI2 then feeds into the ADC, which has a prescaler of /10.
        // Finally, the ADC's SMP value produces the desired sample rate.
        //
        // 4MHz * N / R / 10 / SMP = sample rate.
        //
        // With oversampling, must create faster clock
        // (x2 oversampling requires x2 sample rate clock).
        //
        //  Rate   PLLSAI2N  R  SMPR
        {/* 8k  */ 16,       1, ADC_SMPR_SMP_12P5}, // R3=32k (min), R1=64k
        {/* 16k */ 16,       0, ADC_SMPR_SMP_12P5},
        {/* 20k */ 20,       0, ADC_SMPR_SMP_12P5},
        {/* 32k */ 32,       0, ADC_SMPR_SMP_12P5},
        {/* 48k */ 48,       0, ADC_SMPR_SMP_12P5},
        {/* 96k */ 73,       0, ADC_SMPR_SMP_6P5}   // Technically 96.05263kS/s
    }};

    auto& preset = m_rate_presets[static_cast<int>(rate)];
    auto pllnr = (preset[0] << RCC_PLLSAI2CFGR_PLLSAI2N_Pos) |
                 (preset[1] << RCC_PLLSAI2CFGR_PLLSAI2R_Pos);
    auto smpr = preset[2];

    // Adjust PLLSAI2
    RCC->CR &= ~(RCC_CR_PLLSAI2ON);
    while ((RCC->CR & RCC_CR_PLLSAI2RDY) == RCC_CR_PLLSAI2RDY);
    RCC->PLLSAI2CFGR = (RCC->PLLSAI2CFGR & ~(RCC_PLLSAI2CFGR_PLLSAI2N_Msk | RCC_PLLSAI2CFGR_PLLSAI2R_Msk)) | pllnr;
    RCC->CR |= RCC_CR_PLLSAI2ON;
    while ((RCC->CR & RCC_CR_PLLSAI2RDY) != RCC_CR_PLLSAI2RDY);

    m_group_config.smpr[0] = ADC_SMPR1_SMP_AN5(smpr);

    // 8x oversample
    m_group_config.cfgr2 = ADC_CFGR2_ROVSE | (2 << ADC_CFGR2_OVSR_Pos) | (3 << ADC_CFGR2_OVSS_Pos);
    m_group_config2.cfgr2 = ADC_CFGR2_ROVSE | (2 << ADC_CFGR2_OVSR_Pos) | (3 << ADC_CFGR2_OVSS_Pos);
#endif
}

void ADC::setOperation(ADC::Operation operation)
{
    m_operation = operation;
}

void ADC::conversionCallback(ADCDriver *driver)
{
    if (m_operation != nullptr) {
        auto half_size = m_current_buffer_size / 2;
        if (adcIsBufferComplete(driver))
            m_operation(m_current_buffer + half_size, half_size);
        else
            m_operation(m_current_buffer, half_size);
    }
}

