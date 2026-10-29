#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "dev/leds.h"
#include "node-id.h"
#include "sys/energest.h"
#include <stdio.h>
#include <string.h>

/* CAU HINH TAI (Traffic rate):
 * Tai cao: 1 giay | Tai trung binh: 5 giay | Tai thap: 10
 */
#ifndef SEND_INTERVAL_SEC
#define SEND_INTERVAL_SEC 1  /* Tai cao */
#endif

#define MAX_NEIGHBORS 10

/* Cau truc goi tin */
struct packet_msg {
  uint16_t sender_id;
  uint16_t seq_no;
};

/* Bang thong ke not lan can */
struct neighbor_stat {
  uint16_t node_id;
  uint16_t tx_packets; /* So goi not do da gui (lay theo seq_no) */
  uint16_t rx_packets; /* So goi not nay nhan duoc tu not do */
  uint16_t last_seq; /* Luu so thu tu goi tin nhan gan nhat */
};

static struct neighbor_stat neighbors[MAX_NEIGHBORS];
static int num_neighbors = 0;
static uint16_t my_tx_count = 0;
static struct broadcast_conn broadcast;

/* Bien luu tru nang luong de do RDC theo chu ky */
static unsigned long last_cpu, last_lpm, last_tx, last_rx;

/* Ham in thong ke PRR va Radio Duty Cycle (RDC) */
static void print_performance_stats(void) {
  int j;
  unsigned long cpu, lpm, tx, rx, total_time;
  uint32_t rdc_radio_total;
  uint16_t rdc_int, rdc_frac;

  /* 1. Lay thong so Duty Cycle qua Energest */
  energest_flush();
  cpu = energest_type_time(ENERGEST_TYPE_CPU) - last_cpu;
  lpm = energest_type_time(ENERGEST_TYPE_LPM) - last_lpm;
  tx  = energest_type_time(ENERGEST_TYPE_TRANSMIT) - last_tx;
  rx  = energest_type_time(ENERGEST_TYPE_LISTEN) - last_rx;
  total_time = cpu + lpm;

  if(total_time > 0) {
    /* Ty le bat Radio = (Thoi gian truyen TX + Thoi gian nghe RX) / Tong thoi gian */
    rdc_radio_total = (uint32_t)(((tx + rx) * 10000ULL) / total_time);
    rdc_int = rdc_radio_total / 100;
    rdc_frac = rdc_radio_total % 100;
  } else {
    rdc_int = 0;
    rdc_frac = 0;
  }

  printf("\n=================== STATS REPORT (Node %d) ===================\n", node_id);
  printf("[RADIO DUTY CYCLE] TX: %lu ticks, RX: %lu ticks, Total: %lu ticks\n", tx, rx, total_time);
  printf("[RADIO DUTY CYCLE] Radio Duty Cycle (RDC) = %3u.%02u %%\n", rdc_int, rdc_frac);
  
  /* 2. In bang PRR */
  printf("--------------------- BANG THONG KE PRR ---------------------\n");
  printf("| Node_ID | TX_Packets | RX_Packets |       PRR (%%)         |\n");
  printf("-------------------------------------------------------------\n");
  for(j = 0; j < num_neighbors; j++) {
    uint16_t prr_int = 0;
    uint16_t prr_frac = 0;
    if(neighbors[j].tx_packets > 0) {
      /* Nhan 10000 de lay 2 chu so sau dau phay ma khong can dung so thuc float */
      uint32_t prr_total = ((uint32_t)neighbors[j].rx_packets * 10000) / neighbors[j].tx_packets;
      prr_int = prr_total / 100;
      prr_frac = prr_total % 100;
    }
    printf("|   %03d   |    %04d    |    %04d    |       %3u.%02u%%        |\n",
           neighbors[j].node_id, neighbors[j].tx_packets, neighbors[j].rx_packets, prr_int, prr_frac);
  }
  printf("=============================================================\n\n");
}

/* Callback xu ly khi nhan goi Broadcast */
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from) {
  struct packet_msg msg;
  int index = -1;
  int j;

  if(packetbuf_datalen() >= sizeof(struct packet_msg)) {
    memcpy(&msg, packetbuf_dataptr(), sizeof(msg));
    leds_toggle(LEDS_GREEN);

    /* Cap nhat danh sach nut lan can */
    for(j = 0; j < num_neighbors; j++) {
      if(neighbors[j].node_id == msg.sender_id) {
        index = j;
        break;
      }
    }

    if(index == -1) {
      if(num_neighbors < MAX_NEIGHBORS) {
        index = num_neighbors;
        neighbors[index].node_id = msg.sender_id;
        neighbors[index].rx_packets = 0;
		neighbors[index].last_seq = 0;
        num_neighbors++;
      } else {
        return;
      }
    }

    /* Bo qua goi trung lap do co che strobe phat lap lai cua CX-MAC */
    if(msg.seq_no == neighbors[index].last_seq) {
      return;
    }

    neighbors[index].last_seq = msg.seq_no;
    neighbors[index].tx_packets = msg.seq_no;
    neighbors[index].rx_packets++;
  }
}

static const struct broadcast_callbacks broadcast_call = {broadcast_recv};

/* Khoi tao tien trinh */
PROCESS(rdc_mac_eval_process, "RDC and MAC Evaluation Process");
AUTOSTART_PROCESSES(&rdc_mac_eval_process);

PROCESS_THREAD(rdc_mac_eval_process, ev, data) {
  static struct etimer send_timer;
  static struct etimer report_timer;
  static struct packet_msg out_msg;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  /* Khoi tao module do luong nang luong */
  energest_init();
  energest_flush();
  last_cpu = energest_type_time(ENERGEST_TYPE_CPU);
  last_lpm = energest_type_time(ENERGEST_TYPE_LPM);
  last_tx  = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  last_rx  = energest_type_time(ENERGEST_TYPE_LISTEN);

  broadcast_open(&broadcast, 129, &broadcast_call);

  /* Thiet lap timer bao cao dinh ky moi 30 giay */
  etimer_set(&report_timer, CLOCK_SECOND * 30);

  while(1) {
    /* Dat timer gui goi theo SEND_INTERVAL_SEC va cong them thoi gian ngau nhien (jitter) de chong nghen dong pha */
    etimer_set(&send_timer, CLOCK_SECOND * SEND_INTERVAL_SEC + (random_rand() % CLOCK_SECOND));

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer) || etimer_expired(&report_timer));

    if(etimer_expired(&send_timer)) {
      my_tx_count++;
      out_msg.sender_id = node_id;
      out_msg.seq_no = my_tx_count;

      packetbuf_copyfrom(&out_msg, sizeof(out_msg));
      broadcast_send(&broadcast);
      leds_toggle(LEDS_RED);
    }

    if(etimer_expired(&report_timer)) {
      print_performance_stats();
      etimer_reset(&report_timer);
    }
  }

  PROCESS_END();
}
