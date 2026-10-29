#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"
#include "net/packetbuf.h"
#include "dev/leds.h"
#include <stdio.h>
#include <string.h>

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UDP_PORT 3000
#define SEND_INTERVAL (5 * CLOCK_SECOND)
#define MAX_NEIGHBORS 3
#define NEIGHBOR_TIMEOUT (30 * CLOCK_SECOND) /* Thoi gian timeout neu mat tin hieu */

/* Cau truc du lieu bang neighbor */
struct neighbor_entry {
  uint16_t node_id;
  int16_t rssi;
  clock_time_t last_seen;
  uint8_t is_valid;
};

static struct neighbor_entry neighbors[MAX_NEIGHBORS];
static struct uip_udp_conn *client_conn;
static struct uip_udp_conn *server_conn;

/*---------------------------------------------------------------------------*/
PROCESS(assignment4_process, "Assignment 4: Top 3 RSSI Neighbors");
AUTOSTART_PROCESSES(&assignment4_process);
/*---------------------------------------------------------------------------*/

/* Lay byte cuoi cung cua dia chi IPv6 lam Node ID */
static uint16_t get_node_id_from_ip(const uip_ipaddr_t *ip) {
  return (uint16_t)ip->u8[15];
}

/* 1. Ham sap xep: Day nut co RSSI tot nhat len Rank 1 xep o [Empty] xuong cuoi */
static void sort_neighbors(void) {
  int i, j;
  for(i = 0; i < MAX_NEIGHBORS - 1; i++) {
    for(j = i + 1; j < MAX_NEIGHBORS; j++) {
      /* Neu o i la [Empty] ma o j hop le: Doi cho de day [Empty] xuong */
      if(!neighbors[i].is_valid && neighbors[j].is_valid) {
        struct neighbor_entry temp = neighbors[i];
        neighbors[i] = neighbors[j];
        neighbors[j] = temp;
      }
      /* Neu ca 2 o deu hop le: So sanh dua nut co RSSI tot hon len tren */
      else if(neighbors[i].is_valid && neighbors[j].is_valid) {
        if(neighbors[i].rssi < neighbors[j].rssi) {
          struct neighbor_entry temp = neighbors[i];
          neighbors[i] = neighbors[j];
          neighbors[j] = temp;
        }
      }
    }
  }
}

/* 2. Ham cap nhat bang 3 neighbor co RSSI tot nhat */
static void update_neighbor(uint16_t id, int16_t rssi) {
  int i, j;
  int existing_idx = -1;
  int worst_idx = 0;
  int16_t min_rssi = 32767;

  /* Kiem tra xem nut da co san trong bang chua */
  for(i = 0; i < MAX_NEIGHBORS; i++) {
    if(neighbors[i].is_valid && neighbors[i].node_id == id) {
      existing_idx = i;
      break;
    }
  }

  if(existing_idx != -1) {
    /* Da ton tai: Cap nhat RSSI moi nhat va lam moi thoi gian */
    neighbors[existing_idx].rssi = rssi;
    neighbors[existing_idx].last_seen = clock_time();
  } else {
    /* Neu chua co, tim vi tri trong */
    int empty_idx = -1;
    for(i = 0; i < MAX_NEIGHBORS; i++) {
      if(!neighbors[i].is_valid) {
        empty_idx = i;
        break;
      }
    }

    if(empty_idx != -1) {
      /* Con cho trong: Them truc tiep */
      neighbors[empty_idx].node_id = id;
      neighbors[empty_idx].rssi = rssi;
      neighbors[empty_idx].last_seen = clock_time();
      neighbors[empty_idx].is_valid = 1;
    } else {
      /* 3. Bang da day (du 3 neighbor): Tim neighbor co RSSI yeu nhat (nho nhat) */
      min_rssi = neighbors[0].rssi;
      worst_idx = 0;
      for(i = 1; i < MAX_NEIGHBORS; i++) {
        if(neighbors[i].rssi < min_rssi) {
          min_rssi = neighbors[i].rssi;
          worst_idx = i;
        }
      }

      /* Neu RSSI moi tot hon (lon hon) RSSI yeu nhat (nho nhat) thi thay the */
      if(rssi > min_rssi) {
        neighbors[worst_idx].node_id = id;
        neighbors[worst_idx].rssi = rssi;
        neighbors[worst_idx].last_seen = clock_time();
        neighbors[worst_idx].is_valid = 1;
      }
    }
  }

  /* Sap xep lai bang sau khi cap nhat */
  sort_neighbors();
}

/* 3. Ham kiem tra timeout, sap xep lai va in bang neighbor */
static void print_neighbor_table(void) {
  int i;
  clock_time_t now = clock_time();

  /* Xoa cac nut da het han timeout */
  for(i = 0; i < MAX_NEIGHBORS; i++) {
    if(neighbors[i].is_valid && (now - neighbors[i].last_seen > NEIGHBOR_TIMEOUT)) {
      neighbors[i].is_valid = 0;
    }
  }

  /* Luon sap xep lai sau khi xoa timeout de day [Empty] xuong cuoi */
  sort_neighbors();

  /* In bang ket qua */
  printf("\n=== [TOP 3 NEIGHBORS (BEST RSSI)] ===\n");
  for(i = 0; i < MAX_NEIGHBORS; i++) {
    if(neighbors[i].is_valid) {
      printf(" Rank %d | Node ID: %u | RSSI: %d dBm\n", 
             i + 1, neighbors[i].node_id, neighbors[i].rssi);
    } else {
      printf(" Rank %d | [Empty]\n", i + 1);
    }
  }
  printf("=====================================\n\n");
}

/* Callback nhan du lieu UDP */
static void tcpip_handler(void) {
  if(uip_newdata()) {
    leds_on(LEDS_GREEN);
    int16_t rssi = (int16_t)packetbuf_attr(PACKETBUF_ATTR_RSSI);
    uint16_t sender_id = get_node_id_from_ip(&UIP_IP_BUF->srcipaddr);

    update_neighbor(sender_id, rssi);
    leds_off(LEDS_GREEN);
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(assignment4_process, ev, data)
{
  static struct etimer send_timer;
  static struct etimer report_timer;
  uip_ipaddr_t addr;

  PROCESS_BEGIN();

  /* Khoi tao bang neighbor */
  memset(neighbors, 0, sizeof(neighbors));

  /* Ket noi nhan Broadcast UDP */
  server_conn = udp_new(NULL, UIP_HTONS(0), NULL);
  udp_bind(server_conn, UIP_HTONS(UDP_PORT));

  /* Ket noi gui den tat ca cac nut (link-local all-nodes multicast FF02::1) */
  uip_create_linklocal_allnodes_mcast(&addr);
  client_conn = udp_new(&addr, UIP_HTONS(UDP_PORT), NULL);

  etimer_set(&send_timer, SEND_INTERVAL);
  etimer_set(&report_timer, 10 * CLOCK_SECOND);

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == tcpip_event) {
      tcpip_handler();
    }

    /* Dinh ky phat thong diep */
    if(ev == PROCESS_EVENT_TIMER && data == &send_timer) {
      char msg[] = "BEACON";
      uip_udp_packet_send(client_conn, msg, sizeof(msg));
      etimer_reset(&send_timer);
    }

    /* Dinh ky in bang top 3 neighbor */
    if(ev == PROCESS_EVENT_TIMER && data == &report_timer) {
      print_neighbor_table();
      etimer_reset(&report_timer);
    }
  }

  PROCESS_END();
}
