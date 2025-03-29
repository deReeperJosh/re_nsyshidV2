#include <cstdio>
#include <list>
#include <nsyshid/hid.h>
#include <string>
#include <wups.h>

#include <coreinit/thread.h>
#include <coreinit/time.h>

#include <array>
#include <map>
#include <mutex>
#include <queue>
#include <thread>

#include "devices/Device.h"
#include "devices/Skylander.h"
#include "utils/logger.h"

std::thread hotplugThread;
bool hotplugThreadStop = false;

std::list<HIDClient *> clients;
std::map<HIDClient *, HIDAttachCallback> clientCallbacks;
std::mutex clientMutex;

std::list<std::shared_ptr<Device>> devices;
std::list<HIDDevice *> addedDevices;
std::list<HIDDevice *> removedDevices;
std::map<HIDDevice *, HIDClient *> devicePairings;
std::mutex deviceMutex;

std::recursive_mutex hidMutex;

uint32_t _lastGeneratedHidHandle = 1;
const int HID_MAX_NUM_DEVICES    = 128;
std::array<HIDDevice *, HID_MAX_NUM_DEVICES> HIDPool;
std::queue<size_t> HIDPoolIndexQueue;

Device::Device(uint16_t vendorId,
               uint16_t productId,
               uint8_t interfaceIndex,
               uint8_t interfaceSubClass,
               uint8_t protocol,
               uint16_t m_maxPacketSizeRX,
               uint16_t m_maxPacketSizeTX) {
    m_vendorId          = vendorId;
    m_productId         = productId;
    m_interfaceIndex    = interfaceIndex;
    m_interfaceSubClass = interfaceSubClass;
    m_protocol          = protocol;
    m_maxPacketSizeRX   = m_maxPacketSizeRX;
    m_maxPacketSizeTX   = m_maxPacketSizeTX;
}

void Device::AssignHID(HIDDevice *hid) {
    if (hid != nullptr) {
        hid->vid                = this->m_vendorId;
        hid->pid                = this->m_productId;
        hid->interfaceIndex     = this->m_interfaceIndex;
        hid->subClass           = this->m_interfaceSubClass;
        hid->protocol           = this->m_protocol;
        hid->physicalDeviceInst = 0;
        hid->maxPacketSizeRx    = this->m_maxPacketSizeRX;
        hid->maxPacketSizeTx    = this->m_maxPacketSizeTX;
    }
    this->m_hid = hid;
}

void InitHIDPoolIndexQueue() {
    static bool HIDPoolIndexQueueInitialized = false;
    std::lock_guard<std::recursive_mutex> lock(hidMutex);
    if (HIDPoolIndexQueueInitialized) {
        return;
    }
    HIDPoolIndexQueueInitialized = true;
    for (size_t i = 0; i < HID_MAX_NUM_DEVICES; i++) {
        HIDPoolIndexQueue.push(i);
    }
}

HIDDevice *GetFreeHID() {
    std::lock_guard<std::recursive_mutex> lock(hidMutex);
    InitHIDPoolIndexQueue();
    if (HIDPoolIndexQueue.empty()) {
        return nullptr;
    }
    size_t index = HIDPoolIndexQueue.front();
    HIDPoolIndexQueue.pop();
    return HIDPool[index];
}

std::shared_ptr<Device> GetDeviceByHandle(uint32_t handle) {
    std::shared_ptr<Device> device;
    {
        for (const auto &d : devices) {
            if (d->m_hid->handle == handle) {
                device = d;
                break;
            }
        }
    }
    if (device != nullptr) {
        return device;
    }
    return nullptr;
}

void DeviceHotplugThread() {
    do {
        clientMutex.lock();
        deviceMutex.lock();
        if (!clients.empty()) {
            for (auto client = clients.begin(); client != clients.end(); client++) {
                for (auto device = addedDevices.begin(); device != addedDevices.end(); device++) {
                    const auto callback = clientCallbacks.find(*client);
                    int32_t result      = callback->second(*client, *device, HID_DEVICE_ATTACH);
                    if (result == 1) {
                        devicePairings.insert({*device, *client});
                    }
                }
            }
        }
        for (auto device = removedDevices.begin(); device != removedDevices.end(); device++) {
            const auto client = devicePairings.find(*device);
            if (client != devicePairings.end()) {
                client->second->attachCallback(client->second, *device, HID_DEVICE_DETACH);
            }
        }
        addedDevices.clear();
        removedDevices.clear();
        deviceMutex.unlock();
        clientMutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    } while (!hotplugThreadStop);
}

DECL_FUNCTION(int32_t, HIDSetup) {
    DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDSetup Called");

    // auto skylanderDevice = std::make_shared<SkylanderUSBDevice>();
    // HIDDevice *devicePtr = GetFreeHID();
    // devicePtr->handle    = 1;
    // skylanderDevice->AssignHID(devicePtr);
    // deviceMutex.lock();
    // devices.push_back(skylanderDevice);

    // addedDevices.push_front(devicePtr);
    // deviceMutex.unlock();

    hotplugThreadStop = false;
    hotplugThread     = std::thread(DeviceHotplugThread);
    hotplugThread.detach();

    return 0;
}

DECL_FUNCTION(int32_t, HIDTeardown) {
    DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDTeardown Called");
    hotplugThreadStop = true;
    hotplugThread.join();
    return 0;
}

DECL_FUNCTION(int32_t, HIDAddClient, HIDClient *client, HIDAttachCallback attachCallback) {
    DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDAddClient Called");
    clientMutex.lock();
    client->attachCallback = attachCallback;
    clients.push_front(client);
    clientMutex.unlock();
    return 0;
}

DECL_FUNCTION(int32_t, HIDDelClient, HIDClient *client) {
    DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDDelClient Called");
    clientMutex.lock();
    clients.remove(client);
    clientMutex.unlock();
    return 0;
}

void DoHIDCallback(HIDCallback callback, uint32_t handle, void *userContext, uint32_t errorCode, uint32_t responseCode, uint8_t *buffer) {
    callback(handle, errorCode, buffer, responseCode, userContext);
}

void GetDescriptorAsync(std::shared_ptr<Device> device, uint8_t descType, uint8_t descIndex, uint16_t lang,
                        uint8_t *buffer, uint32_t bufferLength, HIDCallback callback, void *userContext) {
    if (device->GetDescriptor(descType, descIndex, lang, buffer, bufferLength)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, bufferLength, buffer);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, bufferLength, buffer);
    }
}

DECL_FUNCTION(int32_t, HIDGetDescriptor, uint32_t handle, uint8_t descriptorType,
              uint8_t descriptorIndex, uint16_t languageId, uint8_t *buffer, uint32_t bufferLength,
              HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDGetDescriptor Called");

    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device) {
        return -111;
    }

    if (callback != nullptr) {
        std::thread t(GetDescriptorAsync, device, descriptorType, descriptorIndex, languageId, buffer, bufferLength, callback, userContext);
        t.detach();
    } else {
        if (device->GetDescriptor(descriptorType, descriptorIndex, languageId, buffer, bufferLength)) {
            return bufferLength;
        }
    }
    return 0;
}

void SetDescriptorAsync(std::shared_ptr<Device> device, uint8_t descType, uint8_t descIndex, uint16_t lang,
                        uint8_t *buffer, uint32_t bufferLength, HIDCallback callback, void *userContext) {
    if (device->SetDescriptor(descType, descIndex, lang, buffer, bufferLength)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, bufferLength, buffer);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, bufferLength, buffer);
    }
}

DECL_FUNCTION(int32_t, HIDSetDescriptor, uint32_t handle, uint8_t descriptorType,
              uint8_t descriptorIndex, uint16_t languageId, uint8_t *buffer, uint32_t bufferLength,
              HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDSetDescriptor Called");

    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device) {
        return -111;
    }

    if (callback != nullptr) {
        std::thread t(SetDescriptorAsync, device, descriptorType, descriptorIndex, languageId, buffer, bufferLength, callback, userContext);
        t.detach();
    } else {
        if (device->SetDescriptor(descriptorType, descriptorIndex, languageId, buffer, bufferLength)) {
            return bufferLength;
        }
    }
    return 0;
}

void GetReportAsync(std::shared_ptr<Device> device, uint8_t reportType, uint8_t reportId,
                    uint8_t *buffer, uint32_t bufferLength, HIDCallback callback, void *userContext) {
    if (device->GetReport(buffer, bufferLength)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, bufferLength, buffer);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, bufferLength, buffer);
    }
}

DECL_FUNCTION(int32_t, HIDGetReport, uint32_t handle, uint8_t reportType, uint8_t reportId,
              uint8_t *buffer, uint32_t bufferLength, HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDGetReport Called");

    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device) {
        return -111;
    }

    if (callback != nullptr) {
        std::thread t(GetReportAsync, device, reportType, reportId, buffer, bufferLength, callback, userContext);
        t.detach();
    } else {
        if (device->GetReport(buffer, bufferLength)) {
            return bufferLength;
        }
    }
    return 0;
}

void SetReportAsync(std::shared_ptr<Device> device, uint8_t reportType, uint8_t reportId,
                    uint8_t *buffer, uint32_t bufferLength, HIDCallback callback, void *userContext) {
    if (device->SetReport(buffer, bufferLength)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, bufferLength, buffer);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, bufferLength, buffer);
    }
}

DECL_FUNCTION(int32_t, HIDSetReport, uint32_t handle, uint8_t reportType, uint8_t reportId,
              uint8_t *buffer, uint32_t bufferLength, HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDSetReport Called");
    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device) {
        return -111;
    }

    if (callback != nullptr) {
        std::thread t(SetReportAsync, device, reportType, reportId, buffer, bufferLength, callback, userContext);
        t.detach();
    } else {
        if (device->SetReport(buffer, bufferLength)) {
            return bufferLength;
        }
    }
    return 0;
}

void GetIdleAsync(std::shared_ptr<Device> device, uint8_t interfaceIndex, uint8_t reportId,
                  uint8_t *duration, HIDCallback callback, void *userContext) {
    if (device->GetIdle(interfaceIndex, reportId, duration)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, 1, duration);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, 1, duration);
    }
}

DECL_FUNCTION(int32_t, HIDGetIdle, uint32_t handle, uint8_t interfaceIndex, uint8_t reportId,
              uint8_t *duration, HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDGetIdle Called");
    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device) {
        return -111;
    }

    if (callback != nullptr) {
        std::thread t(GetIdleAsync, device, interfaceIndex, reportId, duration, callback, userContext);
        t.detach();
    } else {
        if (device->GetIdle(interfaceIndex, reportId, duration)) {
            return 1;
        }
    }
    return 0;
}

void SetIdleAsync(std::shared_ptr<Device> device, uint8_t interfaceIndex, uint8_t reportId,
                  uint8_t duration, HIDCallback callback, void *userContext) {
    if (device->SetIdle(interfaceIndex, reportId, duration)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, 0, nullptr);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, 0, nullptr);
    }
}

DECL_FUNCTION(int32_t, HIDSetIdle, uint32_t handle, uint8_t interfaceIndex, uint8_t reportId,
              uint8_t duration, HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDSetIdle Called");
    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device) {
        return -111;
    }

    if (callback != nullptr) {
        std::thread t(SetIdleAsync, device, interfaceIndex, reportId, duration, callback, userContext);
        t.detach();
    } else {
        if (device->SetIdle(interfaceIndex, reportId, duration)) {
            return 0;
        }
    }
    return 0;
}

void GetProtocolAsync(std::shared_ptr<Device> device, uint8_t interfaceIndex, uint8_t *protocol,
                      HIDCallback callback, void *userContext) {
    if (device->GetProtocol(interfaceIndex, protocol)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, 1, protocol);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, 1, protocol);
    }
}

DECL_FUNCTION(int32_t, HIDGetProtocol, uint32_t handle, uint8_t interfaceIndex, uint8_t *protocol,
              HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDGetProtocol Called");
    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device) {
        return -111;
    }

    if (callback != nullptr) {
        std::thread t(GetProtocolAsync, device, interfaceIndex, protocol, callback, userContext);
        t.detach();
    } else {
        if (device->GetProtocol(interfaceIndex, protocol)) {
            return 1;
        }
    }
    return 0;
}

void SetProtocolAsync(std::shared_ptr<Device> device, uint8_t interfaceIndex, uint8_t protocol,
                      HIDCallback callback, void *userContext) {
    if (device->SetProtocol(interfaceIndex, protocol)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, 0, nullptr);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, 0, nullptr);
    }
}

DECL_FUNCTION(int32_t, HIDSetProtocol, uint32_t handle, uint8_t interfaceIndex, uint8_t protocol,
              HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDSetProtocol Called");
    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device) {
        return -111;
    }

    if (callback != nullptr) {
        std::thread t(SetProtocolAsync, device, interfaceIndex, protocol, callback, userContext);
        t.detach();
    } else {
        if (device->SetProtocol(interfaceIndex, protocol)) {
            return 0;
        }
    }
    return 0;
}

void ReadAsync(std::shared_ptr<Device> device, uint8_t *buffer, uint32_t bufferLength,
               HIDCallback callback, void *userContext) {
    if (device->Read(buffer, bufferLength)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, bufferLength, buffer);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, bufferLength, buffer);
    }
}

DECL_FUNCTION(int32_t, HIDRead, uint32_t handle, uint8_t *buffer, uint32_t bufferLength,
              HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDRead Called");
    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device) {
        return -111;
    }

    if (callback != nullptr) {
        std::thread t(ReadAsync, device, buffer, bufferLength, callback, userContext);
        t.detach();
    } else {
        if (device->Read(buffer, bufferLength)) {
            return bufferLength;
        }
    }
    return 0;
}

void WriteAsync(std::shared_ptr<Device> device, uint8_t *buffer, uint32_t bufferLength,
                HIDCallback callback, void *userContext) {
    if (device->Write(buffer, bufferLength)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, bufferLength, buffer);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, bufferLength, buffer);
    }
}

DECL_FUNCTION(int32_t, HIDWrite, int32_t handle, uint8_t *buffer, uint32_t bufferLength,
              HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDWrite Called");
    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device) {
        return -111;
    }

    if (callback != nullptr) {
        std::thread t(WriteAsync, device, buffer, bufferLength, callback, userContext);
        t.detach();
    } else {
        if (device->Write(buffer, bufferLength)) {
            return bufferLength;
        }
    }
    return 0;
}

WUPS_MUST_REPLACE(HIDSetup, WUPS_LOADER_LIBRARY_NSYSHID, HIDSetup);
WUPS_MUST_REPLACE(HIDTeardown, WUPS_LOADER_LIBRARY_NSYSHID, HIDTeardown);
WUPS_MUST_REPLACE(HIDAddClient, WUPS_LOADER_LIBRARY_NSYSHID, HIDAddClient);
WUPS_MUST_REPLACE(HIDDelClient, WUPS_LOADER_LIBRARY_NSYSHID, HIDDelClient);
WUPS_MUST_REPLACE(HIDGetDescriptor, WUPS_LOADER_LIBRARY_NSYSHID, HIDGetDescriptor);
WUPS_MUST_REPLACE(HIDSetDescriptor, WUPS_LOADER_LIBRARY_NSYSHID, HIDSetDescriptor);
WUPS_MUST_REPLACE(HIDGetReport, WUPS_LOADER_LIBRARY_NSYSHID, HIDGetReport);
WUPS_MUST_REPLACE(HIDSetReport, WUPS_LOADER_LIBRARY_NSYSHID, HIDSetReport);
WUPS_MUST_REPLACE(HIDGetIdle, WUPS_LOADER_LIBRARY_NSYSHID, HIDGetIdle);
WUPS_MUST_REPLACE(HIDSetIdle, WUPS_LOADER_LIBRARY_NSYSHID, HIDSetIdle);
WUPS_MUST_REPLACE(HIDGetProtocol, WUPS_LOADER_LIBRARY_NSYSHID, HIDGetProtocol);
WUPS_MUST_REPLACE(HIDSetProtocol, WUPS_LOADER_LIBRARY_NSYSHID, HIDSetProtocol);
WUPS_MUST_REPLACE(HIDRead, WUPS_LOADER_LIBRARY_NSYSHID, HIDRead);
WUPS_MUST_REPLACE(HIDWrite, WUPS_LOADER_LIBRARY_NSYSHID, HIDWrite);
