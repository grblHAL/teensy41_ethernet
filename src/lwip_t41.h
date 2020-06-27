#ifndef __LWIP_T41__
#define __LWIP_T41__

#if defined(ARDUINO_TEENSY41)

#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"

#ifdef __cplusplus
extern "C" {
#endif
   
typedef void (*rx_frame_fn)(void*);
typedef void (*tx_timestamp_fn)(uint32_t);

void enet_getmac(uint8_t *mac);
void enet_init(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw);
void enet_set_rx_callback(rx_frame_fn rx_cb);
void enet_set_tx_timestamp_callback(tx_timestamp_fn tx_cb);
struct pbuf* enet_rx_next();
void enet_input(struct pbuf* p_frame);
void enet_proc_input(void);
void enet_poll();
void enet_txTimestampNextPacket();
uint32_t read_1588_timer();

#ifdef __cplusplus
}
#endif

#endif

#endif
