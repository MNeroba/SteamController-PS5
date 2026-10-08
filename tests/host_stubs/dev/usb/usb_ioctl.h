#pragma once

/* sc2_gyro_service is not invoked in host tests; values only satisfy parsing. */
#define USB_DO_REQUEST 0xfeed
#define USB_FS_OPEN     0xf001
#define USB_FS_START    0xf002
#define USB_FS_COMPLETE 0xf003
#define USB_FS_STOP     0xf004
#define USB_FS_CLOSE    0xf005
#define USB_GET_FULL_DESC 0xf006
#define USB_FS_FLAG_SINGLE_SHORT_OK 0x01
#define USB_FS_FLAG_MULTI_SHORT_OK  0x02
#define USETW(field, value) ((field) = (uint16_t)(value))
