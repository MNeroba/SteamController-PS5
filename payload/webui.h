#pragma once
#ifndef WEBUI_PORT
#define WEBUI_PORT 8090
#endif
typedef void (*webui_notify_fn)(const char *message);
/* Starts the default loopback listener and first-run Media registration worker. */
int webui_start(webui_notify_fn notify);
/* Stops admission, cancels client sockets, joins all workers, erases secrets.
 * Start/stop are owned by the main lifecycle thread. */
int webui_stop(void);
