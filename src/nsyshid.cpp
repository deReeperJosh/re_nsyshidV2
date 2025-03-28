#include <cstdio>
#include <nsyshid/hid.h>
#include <string>
#include <wups.h>

#include "utils/logger.h"

DECL_FUNCTION(int32_t, HIDSetup)
{
  DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDSetup Called");
  return 0;
}

DECL_FUNCTION(int32_t, HIDTeardown)
{
  DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDTeardown Called");
  return 0;
}

DECL_FUNCTION(int32_t, HIDAddClient, HIDClient* client, HIDAttachCallback attachCallback)
{
  DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDAddClient Called");
  return 0;
}

DECL_FUNCTION(int32_t, HIDDelClient, HIDClient* client)
{
  DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDDelClient Called");
  return 0;
}

DECL_FUNCTION(int32_t, HIDGetDescriptor, uint32_t handle, uint8_t descriptorType,
              uint8_t descriptorIndex, uint16_t languageId, uint8_t* buffer, uint32_t bufferLength,
              HIDCallback callback, void* userContext)
{
  DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDGetDescriptor Called");
  return 0;
}

DECL_FUNCTION(int32_t, HIDSetDescriptor, uint32_t handle, uint8_t descriptorType,
              uint8_t descriptorIndex, uint16_t languageId, uint8_t* buffer, uint32_t bufferLength,
              HIDCallback callback, void* userContext)
{
  DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDSetDescriptor Called");
  return 0;
}

DECL_FUNCTION(int32_t, HIDGetReport, uint32_t handle, uint8_t reportType, uint8_t reportId,
              uint8_t* buffer, uint32_t bufferLength, HIDCallback callback, void* userContext)
{
  DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDGetReport Called");
  return 0;
}

DECL_FUNCTION(int32_t, HIDSetReport, uint32_t handle, uint8_t reportType, uint8_t reportId,
              uint8_t* buffer, uint32_t bufferLength, HIDCallback callback, void* userContext)
{
  DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDSetReport Called");
  return 0;
}

DECL_FUNCTION(int32_t, HIDGetIdle, uint32_t handle, uint8_t interfaceIndex, uint8_t reportId,
              uint8_t* duration, HIDCallback callback, void* userContext)
{
  DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDGetIdle Called");
  return 0;
}

DECL_FUNCTION(int32_t, HIDSetIdle, uint32_t handle, uint8_t interfaceIndex, uint8_t reportId,
              uint8_t duration, HIDCallback callback, void* userContext)
{
  DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDSetIdle Called");
  return 0;
}

DECL_FUNCTION(int32_t, HIDGetProtocol, uint32_t handle, uint8_t interfaceIndex, uint8_t* protocol,
              HIDCallback callback, void* userContext)
{
  DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDGetProtocol Called");
  return 0;
}

DECL_FUNCTION(int32_t, HIDSetProtocol, uint32_t handle, uint8_t interfaceIndex, uint8_t protocol,
              HIDCallback callback, void* userContext)
{
  DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDSetProtocol Called");
  return 0;
}

DECL_FUNCTION(int32_t, HIDRead, uint32_t handle, uint8_t* buffer, uint32_t bufferLength,
              HIDCallback callback, void* userContext)
{
  DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDRead Called");
  return 0;
}

DECL_FUNCTION(int32_t, HIDWrite, int32_t handle, uint8_t* buffer, uint32_t bufferLength,
              HIDCallback callback, void* userContext)
{
  DEBUG_FUNCTION_LINE_INFO("nsyshid::HIDWrite Called");
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
