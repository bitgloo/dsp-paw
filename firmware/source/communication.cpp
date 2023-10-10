#include "communication.hpp"

#include "ch.h"
#include "hal.h"

#include "periph/adc.hpp"
#include "periph/dac.hpp"
#include "periph/usbserial.hpp"
#include "elfload.hpp"
#include "error.hpp"
#include "conversion.hpp"
#include "runstatus.hpp"
#include "samples.hpp"

#include <algorithm>
#include <tuple>

__attribute__((section(".stacks")))
std::array<char, 4096> CommunicationManager::m_thread_stack = {};

void CommunicationManager::begin()
{
    chThdCreateStatic(m_thread_stack.data(),
                      m_thread_stack.size(),
                      NORMALPRIO,
                      threadComm,
                      nullptr);
}

static void writeADCBuffer(unsigned char *);
static void setBufferSize(unsigned char *);
static void updateGenerator(unsigned char *);
static void loadAlgorithm(unsigned char *);
static void readStatus(unsigned char *);
static void measureConversion(unsigned char *);
static void startConversion(unsigned char *);
static void stopConversion(unsigned char *);
static void startGenerator(unsigned char *);
static void readADCBuffer(unsigned char *);
static void readDACBuffer(unsigned char *);
static void unloadAlgorithm(unsigned char *);
static void readIdentifier(unsigned char *);
static void readExecTime(unsigned char *);
static void sampleRate(unsigned char *);
static void readConversionResults(unsigned char *);
static void readConversionInput(unsigned char *);
static void readMessage(unsigned char *);
static void stopGenerator(unsigned char *);

static const std::array<std::pair<char, void (*)(unsigned char *)>, 19> commandTable {{
    {'A', writeADCBuffer},
    {'B', setBufferSize},
    {'D', updateGenerator},
    {'E', loadAlgorithm},
    {'I', readStatus},
    {'M', measureConversion},
    {'R', startConversion},
    {'S', stopConversion},
    {'W', startGenerator},
    {'a', readADCBuffer},
    {'d', readDACBuffer},
    {'e', unloadAlgorithm},
    {'i', readIdentifier},
    {'m', readExecTime},
    {'r', sampleRate},
    {'s', readConversionResults},
    {'t', readConversionInput},
    {'u', readMessage},
    {'w', stopGenerator}
}};

void CommunicationManager::threadComm(void *)
{
	while (1) {
        if (USBSerial::isActive()) {
            // Attempt to receive a command packet
            if (unsigned char cmd[3]; USBSerial::read(&cmd[0], 1) > 0) {
                // Packet received, first byte represents the desired command/action
                auto func = std::find_if(commandTable.cbegin(), commandTable.cend(),
                                         [&cmd](const auto& f) { return f.first == cmd[0]; });
                if (func != commandTable.cend())
                    func->second(cmd);
            }
        }

		chThdSleepMicroseconds(100);
    }
}

void writeADCBuffer(unsigned char *)
{
    USBSerial::read(Samples::In.bytedata(), Samples::In.bytesize());
}

void setBufferSize(unsigned char *cmd)
{
    if (EM.assert(run_status == RunStatus::Idle, Error::NotIdle) &&
        EM.assert(USBSerial::read(&cmd[1], 2) == 2, Error::BadParamSize))
    {
        // count is multiplied by two since this command receives size of buffer
        // for each algorithm application.
        unsigned int count = (cmd[1] | (cmd[2] << 8)) * 2;
        if (EM.assert(count <= MAX_SAMPLE_BUFFER_SIZE, Error::BadParam)) {
            Samples::In.setSize(count);
            Samples::Out.setSize(count);
        }
    }
}

void updateGenerator(unsigned char *cmd)
{
    if (EM.assert(USBSerial::read(&cmd[1], 2) == 2, Error::BadParamSize)) {
        unsigned int count = cmd[1] | (cmd[2] << 8);
        if (EM.assert(count <= MAX_SAMPLE_BUFFER_SIZE, Error::BadParam)) {
            if (!DAC::isSigGenRunning()) {
                Samples::Generator.setSize(count);
                USBSerial::read(
                    reinterpret_cast<uint8_t *>(Samples::Generator.data()),
                    Samples::Generator.bytesize());
            } else {
                const int more = DAC::sigGenWantsMore();
                if (more == -1) {
                    USBSerial::write(reinterpret_cast<const uint8_t *>("\0"), 1);
                } else {
                    USBSerial::write(reinterpret_cast<const uint8_t *>("\1"), 1);

                    // Receive streamed samples in half-buffer chunks.
                    USBSerial::read(reinterpret_cast<uint8_t *>(
                        more == 0 ? Samples::Generator.data() : Samples::Generator.middata()),
                        Samples::Generator.bytesize() / 2);
                }
            }
        }
    }
}

void loadAlgorithm(unsigned char *cmd)
{
    if (EM.assert(run_status == RunStatus::Idle, Error::NotIdle) &&
        EM.assert(USBSerial::read(&cmd[1], 2) == 2, Error::BadParamSize))
    {
        // Only load the binary if it can fit in the memory reserved for it.
        unsigned int size = cmd[1] | (cmd[2] << 8);
        if (EM.assert(size < MAX_ELF_FILE_SIZE, Error::BadUserCodeSize)) {
            USBSerial::read(ELFManager::fileBuffer(), size);
            auto success = ELFManager::loadFromInternalBuffer();
            EM.assert(success, Error::BadUserCodeLoad);
        }
    }
}

void readStatus(unsigned char *)
{
    unsigned char buf[2] = {
        static_cast<unsigned char>(run_status),
        static_cast<unsigned char>(EM.pop())
    };

    USBSerial::write(buf, sizeof(buf));
}

void measureConversion(unsigned char *)
{
    if (EM.assert(run_status == RunStatus::Running, Error::NotRunning))
        ConversionManager::startMeasurement();
}

void startConversion(unsigned char *)
{
    if (EM.assert(run_status == RunStatus::Idle, Error::NotIdle)) {
        run_status = RunStatus::Running;
        ConversionManager::start();
    }
}

void stopConversion(unsigned char *)
{
    if (EM.assert(run_status == RunStatus::Running, Error::NotRunning)) {
        ConversionManager::stop();
        run_status = RunStatus::Idle;
    }
}

void startGenerator(unsigned char *)
{
    DAC::start(1, Samples::Generator.data(), Samples::Generator.size());
}

void readADCBuffer(unsigned char *)
{
    USBSerial::write(Samples::In.bytedata(), Samples::In.bytesize());
}

void readDACBuffer(unsigned char *)
{

    USBSerial::write(Samples::Out.bytedata(), Samples::Out.bytesize());
}

void unloadAlgorithm(unsigned char *)
{
    ELFManager::unload();
}

void readIdentifier(unsigned char *)
{
#if defined(TARGET_PLATFORM_H7)
    USBSerial::write(reinterpret_cast<const uint8_t *>("stmdsph"), 7);
#else
    USBSerial::write(reinterpret_cast<const uint8_t *>("stmdspl"), 7);
#endif
}

void readExecTime(unsigned char *)
{
    // Stores the measured execution time.
    extern time_measurement_t conversion_time_measurement;
    USBSerial::write(reinterpret_cast<uint8_t *>(&conversion_time_measurement.last),
                     sizeof(rtcnt_t));
}

void sampleRate(unsigned char *cmd)
{
    if (EM.assert(USBSerial::read(&cmd[1], 1) == 1, Error::BadParamSize)) {
        if (cmd[1] == 0xFF) {
            auto r = static_cast<unsigned char>(SClock::getRate());
            USBSerial::write(&r, 1);
        } else {
            auto r = static_cast<SClock::Rate>(cmd[1]);
            SClock::setRate(r);
            ADC::setRate(r);
        }
    }
}

void readConversionResults(unsigned char *)
{
    if (auto samps = Samples::Out.modified(); samps != nullptr) {
        unsigned char buf[2] = {
            static_cast<unsigned char>(Samples::Out.size() / 2 & 0xFF),
            static_cast<unsigned char>(((Samples::Out.size() / 2) >> 8) & 0xFF)
        };
        USBSerial::write(buf, 2);
        unsigned int total = Samples::Out.bytesize() / 2;
        unsigned int offset = 0;
        unsigned char unused;
        while (total > 512) {
            USBSerial::write(reinterpret_cast<uint8_t *>(samps) + offset, 512);
            while (USBSerial::read(&unused, 1) == 0);
            offset += 512;
            total -= 512;
        }
        USBSerial::write(reinterpret_cast<uint8_t *>(samps) + offset, total);
        while (USBSerial::read(&unused, 1) == 0);
    } else {
        USBSerial::write(reinterpret_cast<const uint8_t *>("\0\0"), 2);
    }
}

void readConversionInput(unsigned char *)
{
    if (auto samps = Samples::In.modified(); samps != nullptr) {
        unsigned char buf[2] = {
            static_cast<unsigned char>(Samples::In.size() / 2 & 0xFF),
            static_cast<unsigned char>(((Samples::In.size() / 2) >> 8) & 0xFF)
        };
        USBSerial::write(buf, 2);
        unsigned int total = Samples::In.bytesize() / 2;
        unsigned int offset = 0;
        unsigned char unused;
        while (total > 512) {
            USBSerial::write(reinterpret_cast<uint8_t *>(samps) + offset, 512);
            while (USBSerial::read(&unused, 1) == 0);
            offset += 512;
            total -= 512;
        }
        USBSerial::write(reinterpret_cast<uint8_t *>(samps) + offset, total);
        while (USBSerial::read(&unused, 1) == 0);
    } else {
        USBSerial::write(reinterpret_cast<const uint8_t *>("\0\0"), 2);
    }
}

void readMessage(unsigned char *)
{
    //USBSerial::write(reinterpret_cast<uint8_t *>(userMessageBuffer), userMessageSize);
}

void stopGenerator(unsigned char *)
{
    DAC::stop(1);
}

