/*
 * This file is part of libfreespace.
 *
 * Copyright (c) 2009 Hillcrest Laboratories, Inc.
 *
 * libfreespace is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef FREESPACE_WIN32_H_
#define FREESPACE_WIN32_H_

#include "freespace/freespace.h"
#include "freespace/freespace_codecs.h"

// Define our debug printf statements
#ifdef DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#define DEBUG_WPRINTF(...) wprintf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...) /* printf(__VA_ARGS__) */
#define DEBUG_WPRINTF(...) /* wprintf(__VA_ARGS__) */
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <setupapi.h>
#include <hidsdi.h>
#include <dbt.h>

/**
 * The reference used by the discovery API.
 */
typedef WCHAR* FreespaceDeviceRef;
#define FREESPACE_MAXIMUM_SEND_MESSAGE_COUNT 10

struct LibfreespaceData {
    // Device management
    struct FreespaceDeviceStruct* devices_[FREESPACE_MAXIMUM_DEVICE_COUNT];

    int deviceCount_ ;
    void* hotplugCookie_;
    freespace_hotplugCallback hotplugCallback_;
    freespace_pollfdAddedCallback fdAddedCallback_;
    freespace_pollfdRemovedCallback fdRemovedCallback_;

    int nextDeviceId_;

    // Discovery data
    HWND    window_;
    HDEVNOTIFY windowEvent_;
    WNDCLASSEX *wndclass_;

    // Whenever a device is attached or removed to the system,
    // the needToRescanDevicesFlag_ is set and the discoveryEvent
    // is set to wake up a WFMO call.
    BOOL needToRescanDevicesFlag_;
    HANDLE discoveryEvent_;

    int discoveryTheadStatus_;
};

extern struct LibfreespaceData* freespace_instance_;

enum freespace_discoveryStatus {
    FREESPACE_DISCOVERY_STATUS_UNKNOWN,
    FREESPACE_DISCOVERY_STATUS_EXISTING,
    FREESPACE_DISCOVERY_STATUS_ADDED,
    FREESPACE_DISCOVERY_STATUS_REMOVED
};

#define FREESPACE_HANDLE_COUNT_MAX 2
// Defined in ms.
#define RECEIVE_TIMEOUT 250

struct FreespaceDeviceInterfaceInfo {
    // The device vendor ID.
    uint16_t        idVendor_;
    // The device product ID.
    uint16_t        idProduct_;
    // The device usage.
    USAGE           usage_;
    // The device usage page.
    USAGE           usagePage_;

    // The input (read) report byte length.
    USHORT          inputReportByteLength_;
    // The output (send) report byte length.
    USHORT          outputReportByteLength_;
};

/**
 * Representation for a single Windows HID device.
 */
struct FreespaceSubStruct {
    // The device path used by CreateFile.
    WCHAR*          devicePath;   // malloc
    /// The handle returned by CreateFile.
    HANDLE          handle_;
    // The overlapped IO used for read operations.
    OVERLAPPED      readOverlapped_;
    // The current read status: true to start a new read, false to continue to
    // use the old overlapped IO operation.
    BOOL            readStatus_;
    // The read data buffer.
    unsigned char   readBuffer[FREESPACE_MAX_INPUT_MESSAGE_SIZE];
    // The size of the data currently in the read data buffer.
    unsigned long   readBufferSize;

    struct FreespaceDeviceInterfaceInfo info_;
};

struct FreespaceSendStruct {
    // The target device interface.
    struct FreespaceSubStruct* interface_;
    // The overlapped IO used for the send operation.
    OVERLAPPED      overlapped_;
    // The callback used to handle each sent message.
    freespace_sendCallback callback_;
    // The cookie used for the send message
    void*                  cookie_;
    // The timeout for this event.
    unsigned int timeoutMs_;

    // The report to be sent.
    unsigned char report_[FREESPACE_MAX_OUTPUT_MESSAGE_SIZE];
    // The number of bytes sent.
    unsigned long numBytes_;
    // The return code
    int             rc_;
    // The value from GetLastError() for any errors encountered.
    DWORD           error_;
};

/**
 * FreespaceDevice internal data.
 * May contain multiple Windows HID devices.
 */
struct FreespaceDeviceStruct {
    // The Freespace device handle used by the Freespace API.
    FreespaceDeviceId           id_;

    // The user-meaningful name for the device.
    const char*					name_; // Uses constant value.

    // The discovery status for the device which is used to detect when
    // devices are removed from the system.
    enum freespace_discoveryStatus   status_;

    // Whether freespace_openDevice has been called or not.
    BOOL						isOpened_;

    // The number of handles supported by this device.
    int                         handleCount_;
    // The structure containing the detailed description for each handle.
    struct FreespaceSubStruct   handle_[FREESPACE_HANDLE_COUNT_MAX];

    // The callback used for each received message.
    freespace_receiveCallback   receiveCallback_;
    // The cookie passed to the receive callback.
    void*                       receiveCookie_;

    // Send events outstanding
    struct FreespaceSendStruct  send_[FREESPACE_MAXIMUM_SEND_MESSAGE_COUNT];
};


#ifdef __cplusplus
}
#endif

#endif /* FREESPACE_WIN32_H_ */