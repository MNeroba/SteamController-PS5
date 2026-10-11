#pragma once
#ifndef WEBUI_PORT
#define WEBUI_PORT 8090
#endif
typedef void (*webui_notify_fn)(const char *message);
/* Return 0 only after private auth, pairing, bind/listen and worker startup
 * succeed. Notifications carry a local short-lived code, never the token. */
int webui_start(webui_notify_fn notify);
/* Stops admission, cancels client sockets, joins all workers, erases secrets.
 * Start/stop are owned by the main lifecycle thread. */
int webui_stop(void);
