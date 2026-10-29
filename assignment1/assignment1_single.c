#include "contiki.h"
#include "dev/leds.h"
#include "sys/etimer.h"
#include <stdio.h>

PROCESS(single_thread_led_process, "Single Thread LED Process");
AUTOSTART_PROCESSES(&single_thread_led_process);

PROCESS_THREAD(single_thread_led_process, ev, data)
{
  static struct etimer et_red;
  static struct etimer et_green;

  PROCESS_BEGIN();

  printf("=== Bat dau Assignment 1 - 1 Thread ===\n");
  
  // Khoi tao timer: RED 3s, GREEN 5s
  etimer_set(&et_red, CLOCK_SECOND * 3);
  etimer_set(&et_green, CLOCK_SECOND * 5);

  while(1) {
    PROCESS_WAIT_EVENT();

    // Kiem tra neu et_red het han
    if(ev == PROCESS_EVENT_TIMER && data == &et_red) {
      leds_toggle(LEDS_RED);
      printf("[1-Thread] Toggle LED RED (Chu ky 3s)\n");
      etimer_reset(&et_red);
    }

    // Kiem tra neu et_green het han
    if(ev == PROCESS_EVENT_TIMER && data == &et_green) {
      leds_toggle(LEDS_GREEN);
      printf("[1-Thread] Toggle LED GREEN (Chu ky 5s)\n");
      etimer_reset(&et_green);
    }
  }

  PROCESS_END();
}
