#pragma once

/* sc2_gyro_service is not invoked in host tests; values only satisfy parsing. */
#define USB_DO_REQUEST 0xfeed
#define USETW(field, value) ((field) = (uint16_t)(value))
