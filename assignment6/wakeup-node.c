#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "dev/leds.h"
#include "node-id.h"
#include <stdio.h>
#include <string.h>

/* Khai bao cau truc goi tin Wake-Up Beacon */
struct wakeup_message {
  uint16_t sender_id;
  uint16_t seq_no;
};

static struct broadcast_conn wakeup_broadcast;
static uint16_t tx_count = 0;

/* Ham callback xu ly khi nhan duoc goi tin Wake-Up tu kenh vo tuyen */
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  struct wakeup_message msg;
  
  if(packetbuf_datalen() >= sizeof(struct wakeup_message)) {
    memcpy(&msg, packetbuf_dataptr(), sizeof(msg));
    
    /* Bat den LED xanh de bao hieu da nhan tin hieu Wake-Up */
    leds_toggle(LEDS_GREEN);
    
    /* In thong tin nhat ky nhan goi tin Wake-Up */
    printf("[WAKE-UP RX] Node %u nhan tin hieu tu Node %u (seq: %u)\n", 
           node_id, msg.sender_id, msg.seq_no);
  }
}

static const struct broadcast_callbacks broadcast_call = {broadcast_recv};

/* Khai bao tien trinh chinh cua giao thuc Wake-Up */
PROCESS(wakeup_protocol_process, "Wake-Up Protocol Process");
AUTOSTART_PROCESSES(&wakeup_protocol_process);

PROCESS_THREAD(wakeup_protocol_process, ev, data)
{
  static struct etimer periodic_timer;
  static struct wakeup_message msg_to_send;

  PROCESS_EXITHANDLER(broadcast_close(&wakeup_broadcast);)

  PROCESS_BEGIN();

  /* Mo ket noi Rime Broadcast tren kenh 130 cho giao thuc Wake-Up */
  broadcast_open(&wakeup_broadcast, 130, &broadcast_call);

  /* Khoi tao timer dinh ky phat tin hieu Wake-Up (2 giây) */
  etimer_set(&periodic_timer, CLOCK_SECOND * 2 + (random_rand() % (CLOCK_SECOND)));

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    /* Tang so thu tu goi tin phat di */
    tx_count++;
    msg_to_send.sender_id = node_id;
    msg_to_send.seq_no = tx_count;

    /* Sao chep du lieu vao bo dem va phat broadcast ra khong gian */
    packetbuf_copyfrom(&msg_to_send, sizeof(msg_to_send));
    broadcast_send(&wakeup_broadcast);

    /* Bat den LED do de bao hieu dang phat goi tin Wake-Up (Beacon) */
    leds_toggle(LEDS_RED);
    printf("[WAKE-UP TX] Node %u dang phat goi tin Wake-Up thu %u\n", node_id, tx_count);

    /* Dat lai timer cho chu ky tiep theo */
    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}
