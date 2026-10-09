#pragma once
#define WEBUI_PORT 8090
/* Starts the remap portal: http://<PS5_IP>:8090 */
void webui_start(void);
int webui_stop(void); /* joins the listener after its bounded accept wait */
