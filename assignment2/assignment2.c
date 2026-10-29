#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "dev/leds.h"
#include "node-id.h"
#include <stdio.h>
#include <string.h>

#define MAX_NEIGHBORS 10

/* Cau truc goi du lieu Broadcast gui qua song vo tuyen */
struct broadcast_message {
  uint16_t sender_id;
  uint16_t tx_seq;
};

/* Bang theo doi thong ke cac nut lan can*/
struct neighbor_stat {
  uint16_t node_id;
  uint16_t tx_packets; // So goi nut lan can da gui (lay tu tx_seq cua nut do)
  uint16_t rx_packets; // So goi nut hien tai nhan duoc tu nut do
};

static struct neighbor_stat neighbors[MAX_NEIGHBORS];
static int num_neighbors = 0;
static uint16_t my_tx_count = 0; // Bo dem so goi nut hien tai da gui

static struct broadcast_conn broadcast;

/* Ham cap nhat thong ke va in bang PRR */
static void update_and_print_stats(uint16_t sender_id, uint16_t tx_seq)
{
  int index = -1;
  int j;

  // Tìm node_id trong bang
  for(j = 0; j < num_neighbors; j++) {
    if(neighbors[j].node_id == sender_id) {
      index = j;
      break;
    }
  }

  // Neu la node moi thi them vao bang
  if(index == -1) {
    if(num_neighbors < MAX_NEIGHBORS) {
      index = num_neighbors;
      neighbors[index].node_id = sender_id;
      neighbors[index].rx_packets = 0;
      num_neighbors++;
    } else {
      printf("[WARNING] Danh sach nut lan can da day!\n");
      return;
    }
  }

  // Cap nhat so goi gui va nhan
  neighbors[index].tx_packets = tx_seq;
  neighbors[index].rx_packets++;

  // In bang thong ke chi tiet
  printf("\n================ BANG THONG KE PRR (Node %d) ================\n", node_id);
  printf("| Node_ID | TX_Packets (Goi gui) | RX_Packets (Goi nhan) | PRR (%%)  |\n");
  printf("-------------------------------------------------------------\n");
  for(j = 0; j < num_neighbors; j++) {
    uint16_t prr_int = 0;
    uint16_t prr_frac = 0;

    if(neighbors[j].tx_packets > 0) {
      // Tinh phan tram nhan 100 thanh so nguyen de tranh dung float
      uint32_t prr_total = ((uint32_t)neighbors[j].rx_packets * 10000) / neighbors[j].tx_packets;
      prr_int = prr_total / 100;         // Phan nguyen
      prr_frac = prr_total % 100;        // Hai chu so thap phan
    }

    printf("|   %03d   |         %04d         |         %04d          |  %3u.%02u%%  |\n",
           neighbors[j].node_id,
           neighbors[j].tx_packets,
           neighbors[j].rx_packets,
           prr_int,
           prr_frac);
  }
  printf("=============================================================\n\n");
}

/* Callback khi nhan goi tin Broadcast */
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  struct broadcast_message msg;

  if(packetbuf_datalen() >= sizeof(struct broadcast_message)) {
    memcpy(&msg, packetbuf_dataptr(), sizeof(msg));
    leds_toggle(LEDS_GREEN);
    printf("Nhan Broadcast tu Node %d (seq: %d)\n", msg.sender_id, msg.tx_seq);
    update_and_print_stats(msg.sender_id, msg.tx_seq);
  }
}

static const struct broadcast_callbacks broadcast_call = {broadcast_recv};

/* Tien trinh Contiki chinh*/
PROCESS(assignment2_broadcast_process, "Assignment 2 Broadcast Process");
AUTOSTART_PROCESSES(&assignment2_broadcast_process);

PROCESS_THREAD(assignment2_broadcast_process, ev, data)
{
  static struct etimer et;
  static struct broadcast_message msg_to_send;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  while(1) {
    // Phat goi tin ngau nhien chu ky 3 den 6 giay
    etimer_set(&et, CLOCK_SECOND * 3 + random_rand() % (CLOCK_SECOND * 3));
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    my_tx_count++;
    msg_to_send.sender_id = node_id;
    msg_to_send.tx_seq = my_tx_count;

    packetbuf_copyfrom(&msg_to_send, sizeof(msg_to_send));
    broadcast_send(&broadcast);
    leds_toggle(LEDS_RED);
    printf("[Broadcast Sent] Node %d da gui goi tin thu %d\n", node_id, my_tx_count);
  }

  PROCESS_END();
}
