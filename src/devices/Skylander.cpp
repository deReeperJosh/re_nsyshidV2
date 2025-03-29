#include "Skylander.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <thread>
#include <wut.h>

SkylanderPortal g_skyportal;

SkylanderUSBDevice::SkylanderUSBDevice() : Device(0x1430, 0x0150, 1, 2, 0, 0x40, 0x40) {
}

SkylanderUSBDevice::~SkylanderUSBDevice() = default;

bool SkylanderUSBDevice::GetDescriptor(uint8_t descType,
                                       uint8_t descIndex,
                                       uint16_t lang,
                                       uint8_t *buffer,
                                       uint32_t bufferLength) {
    uint8_t configurationDescriptor[0x29];

    uint8_t *currentWritePtr;

    // configuration descriptor
    currentWritePtr                    = configurationDescriptor + 0;
    *(uint8_t *) (currentWritePtr + 0) = 9;    // bLength
    *(uint8_t *) (currentWritePtr + 1) = 2;    // bDescriptorType
    *(uint8_t *) (currentWritePtr + 2) = 0x00; // wTotalLength
    *(uint8_t *) (currentWritePtr + 3) = 0x29; // wTotalLength
    *(uint8_t *) (currentWritePtr + 4) = 1;    // bNumInterfaces
    *(uint8_t *) (currentWritePtr + 5) = 1;    // bConfigurationValue
    *(uint8_t *) (currentWritePtr + 6) = 0;    // iConfiguration
    *(uint8_t *) (currentWritePtr + 7) = 0x80; // bmAttributes
    *(uint8_t *) (currentWritePtr + 8) = 0xFA; // MaxPower
    currentWritePtr                    = currentWritePtr + 9;
    // interface descriptor
    *(uint8_t *) (currentWritePtr + 0) = 9;    // bLength
    *(uint8_t *) (currentWritePtr + 1) = 0x04; // bDescriptorType
    *(uint8_t *) (currentWritePtr + 2) = 0;    // bInterfaceNumber
    *(uint8_t *) (currentWritePtr + 3) = 0;    // bAlternateSetting
    *(uint8_t *) (currentWritePtr + 4) = 2;    // bNumEndpoints
    *(uint8_t *) (currentWritePtr + 5) = 3;    // bInterfaceClass
    *(uint8_t *) (currentWritePtr + 6) = 0;    // bInterfaceSubClass
    *(uint8_t *) (currentWritePtr + 7) = 0;    // bInterfaceProtocol
    *(uint8_t *) (currentWritePtr + 8) = 0;    // iInterface
    currentWritePtr                    = currentWritePtr + 9;
    // HID descriptor
    *(uint8_t *) (currentWritePtr + 0) = 9;    // bLength
    *(uint8_t *) (currentWritePtr + 1) = 0x21; // bDescriptorType
    *(uint8_t *) (currentWritePtr + 2) = 0x01; // bcdHID
    *(uint8_t *) (currentWritePtr + 3) = 0x11; // bcdHID
    *(uint8_t *) (currentWritePtr + 4) = 0x00; // bCountryCode
    *(uint8_t *) (currentWritePtr + 5) = 0x01; // bNumDescriptors
    *(uint8_t *) (currentWritePtr + 6) = 0x22; // bDescriptorType
    *(uint8_t *) (currentWritePtr + 7) = 0x00; // wDescriptorLength
    *(uint8_t *) (currentWritePtr + 8) = 0x1D; // wDescriptorLength
    currentWritePtr                    = currentWritePtr + 9;
    // endpoint descriptor 1
    *(uint8_t *) (currentWritePtr + 0) = 7;    // bLength
    *(uint8_t *) (currentWritePtr + 1) = 0x05; // bDescriptorType
    *(uint8_t *) (currentWritePtr + 2) = 0x81; // bEndpointAddress
    *(uint8_t *) (currentWritePtr + 3) = 0x03; // bmAttributes
    *(uint8_t *) (currentWritePtr + 4) = 0x00; // wMaxPacketSize
    *(uint8_t *) (currentWritePtr + 5) = 0x40; // wMaxPacketSize
    *(uint8_t *) (currentWritePtr + 6) = 0x01; // bInterval
    currentWritePtr                    = currentWritePtr + 7;
    // endpoint descriptor 2
    *(uint8_t *) (currentWritePtr + 0) = 7;    // bLength
    *(uint8_t *) (currentWritePtr + 1) = 0x05; // bDescriptorType
    *(uint8_t *) (currentWritePtr + 2) = 0x02; // bEndpointAddress
    *(uint8_t *) (currentWritePtr + 3) = 0x03; // bmAttributes
    *(uint8_t *) (currentWritePtr + 4) = 0x00; // wMaxPacketSize
    *(uint8_t *) (currentWritePtr + 5) = 0x40; // wMaxPacketSize
    *(uint8_t *) (currentWritePtr + 6) = 0x01; // bInterval
    currentWritePtr                    = currentWritePtr + 7;

    memcpy(buffer, configurationDescriptor,
           std::min<uint32_t>(bufferLength, sizeof(configurationDescriptor)));

    return true;
}

bool SkylanderUSBDevice::SetDescriptor(uint8_t descType,
                                       uint8_t descIndex,
                                       uint16_t lang,
                                       uint8_t *buffer,
                                       uint32_t bufferLength) {
    return true;
}

bool SkylanderUSBDevice::GetReport(uint8_t *buffer,
                                   uint32_t bufferLength) {
    return true;
}

bool SkylanderUSBDevice::SetReport(uint8_t *buffer,
                                   uint32_t bufferLength) {
    g_skyportal.ControlTransfer(buffer, bufferLength);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return true;
}

bool SkylanderUSBDevice::GetIdle(uint8_t ifIndex,
                                 uint8_t reportId,
                                 uint8_t *duration) {
    return true;
}

bool SkylanderUSBDevice::SetIdle(uint8_t ifIndex,
                                 uint8_t reportId,
                                 uint8_t duration) {
    return true;
}

bool SkylanderUSBDevice::GetProtocol(uint8_t ifIndex,
                                     uint8_t *protocol) {
    return true;
}

bool SkylanderUSBDevice::SetProtocol(uint8_t ifIndex,
                                     uint8_t protocol) {
    return true;
}

bool SkylanderUSBDevice::Read(uint8_t *buffer,
                              uint32_t bufferLength) {
    return true;
}

bool SkylanderUSBDevice::Write(uint8_t *buffer,
                               uint32_t bufferLength) {
    memcpy(buffer, g_skyportal.GetStatus().data(), bufferLength);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return true;
}

void SkylanderPortal::ControlTransfer(uint8_t *buf, uint32_t length) {
    std::array<uint8_t, 64> interruptResponse = {};
    switch (buf[0]) {
        case 'A': {
            interruptResponse = {buf[0], buf[1], 0xFF, 0x77};
            g_skyportal.Activate();
            break;
        }
        case 'C': {
            g_skyportal.SetLeds(0x01, buf[1], buf[2], buf[3]);
            break;
        }
        case 'J': {
            g_skyportal.SetLeds(buf[1], buf[2], buf[3], buf[4]);
            interruptResponse = {buf[0]};
            break;
        }
        case 'L': {
            uint8_t side = buf[1];
            if (side == 0x02) {
                side = 0x04;
            }
            g_skyportal.SetLeds(side, buf[2], buf[3], buf[4]);
            break;
        }
        case 'M': {
            interruptResponse = {buf[0], buf[1], 0x00, 0x19};
            break;
        }
        case 'Q': {
            const uint8_t skyNum = buf[1] & 0xF;
            const uint8_t block  = buf[2];
            g_skyportal.QueryBlock(skyNum, block, interruptResponse.data());
            break;
        }
        case 'R': {
            interruptResponse = {buf[0], 0x02, 0x1b};
            break;
        }
        case 'S':
        case 'V': {
            // No response needed
            break;
        }
        case 'W': {
            const uint8_t skyNum = buf[1] & 0xF;
            const uint8_t block  = buf[2];
            g_skyportal.WriteBlock(skyNum, block, &buf[3], interruptResponse.data());
            break;
        }
        default:
            break;
    }
    if (interruptResponse[0] != 0) {
        std::lock_guard lock(m_queryMutex);
        m_queries.push(interruptResponse);
    }
}

std::array<uint8_t, 64> SkylanderPortal::GetStatus() {
    std::lock_guard lock(m_queryMutex);
    std::array<uint8_t, 64> interruptResponse = {};

    if (!m_queries.empty()) {
        interruptResponse = m_queries.front();
        m_queries.pop();
        // This needs to happen after ~22 milliseconds
    } else {
        uint32_t status = 0;
        uint8_t active  = 0x00;
        if (m_activated) {
            active = 0x01;
        }

        for (int i = 16 - 1; i >= 0; i--) {
            auto &s = m_skylanders[i];

            if (!s.queuedStatus.empty()) {
                s.status = s.queuedStatus.front();
                s.queuedStatus.pop();
            }
            status <<= 2;
            status |= s.status;
        }
        interruptResponse = {0x53, 0x00, 0x00, 0x00, 0x00, m_interruptCounter++,
                             active, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00};
        memcpy(&interruptResponse[1], &status, sizeof(status));
    }
    return interruptResponse;
}

void SkylanderPortal::Activate() {
    std::lock_guard lock(m_skyMutex);
    if (m_activated) {
        // If the portal was already active no change is needed
        return;
    }

    // If not we need to advertise change to all the figures present on the portal
    for (auto &s : m_skylanders) {
        if (s.status & 1) {
            s.queuedStatus.push(3);
            s.queuedStatus.push(1);
        }
    }

    m_activated = true;
}

void SkylanderPortal::Deactivate() {
    std::lock_guard lock(m_skyMutex);

    for (auto &s : m_skylanders) {
        // check if at the end of the updates there would be a figure on the portal
        if (!s.queuedStatus.empty()) {
            s.status       = s.queuedStatus.back();
            s.queuedStatus = std::queue<uint8_t>();
        }

        s.status &= 1;
    }

    m_activated = false;
}

void SkylanderPortal::SetLeds(uint8_t side, uint8_t r, uint8_t g, uint8_t b) {
    std::lock_guard lock(m_skyMutex);
    if (side == 0x00) {
        m_colorRight.red   = r;
        m_colorRight.green = g;
        m_colorRight.blue  = b;
    } else if (side == 0x01) {
        m_colorRight.red   = r;
        m_colorRight.green = g;
        m_colorRight.blue  = b;

        m_colorLeft.red   = r;
        m_colorLeft.green = g;
        m_colorLeft.blue  = b;
    } else if (side == 0x02) {
        m_colorLeft.red   = r;
        m_colorLeft.green = g;
        m_colorLeft.blue  = b;
    } else if (side == 0x03) {
        m_colorTrap.red   = r;
        m_colorTrap.green = g;
        m_colorTrap.blue  = b;
    }
}

void SkylanderPortal::QueryBlock(uint8_t skyNum, uint8_t block, uint8_t *replyBuf) {
    std::lock_guard lock(m_skyMutex);

    const auto &skylander = m_skylanders[skyNum];

    replyBuf[0] = 'Q';
    replyBuf[2] = block;
    if (skylander.status & 1) {
        replyBuf[1] = (0x10 | skyNum);
        memcpy(replyBuf + 3, skylander.data.data() + (16 * block), 16);
    } else {
        replyBuf[1] = skyNum;
    }
}

void SkylanderPortal::WriteBlock(uint8_t skyNum, uint8_t block,
                                 const uint8_t *toWriteBuf, uint8_t *replyBuf) {
    std::lock_guard lock(m_skyMutex);

    auto &skylander = m_skylanders[skyNum];

    replyBuf[0] = 'W';
    replyBuf[2] = block;

    if (skylander.status & 1) {
        replyBuf[1] = (0x10 | skyNum);
        memcpy(skylander.data.data() + (block * 16), toWriteBuf, 16);
        skylander.Save();
    } else {
        replyBuf[1] = skyNum;
    }
}

void SkylanderPortal::Skylander::Save() {
    // if (!skyFile)
    // 	return;

    // skyFile->SetPosition(0);
    // skyFile->writeData(data.data(), data.size());
}
