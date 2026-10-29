#include "contiki.h"
#include "dev/leds.h"
#include "sys/etimer.h"
#include <stdio.h>

// Bien toan cuc i dung chung giua 2 threads
static int i = 0;

PROCESS(led_red_process, "Thread 1 - LED RED");
PROCESS(led_green_process, "Thread 2 - LED GREEN");
AUTOSTART_PROCESSES(&led_red_process, &led_green_process);

/* ------------------- THREAD 1: CHU KY 3S ------------------- */
PROCESS_THREAD(led_red_process, ev, data)
{
  static struct etimer et1;

  PROCESS_BEGIN();
  etimer_set(&et1, CLOCK_SECOND * 3);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER && data == &et1);

    leds_toggle(LEDS_RED);
    printf("[Thread 1 - 3s] Toggle RED | Gia tri i hien tai: %d\n", i);
    i += 2;
    printf("[Thread 1 - 3s] -> Sau khi tang i (+2): %d\n", i);

    etimer_reset(&et1);
  }

  PROCESS_END();
}

/* ------------------- THREAD 2: CHU KY 5S ------------------- */
PROCESS_THREAD(led_green_process, ev, data)
{
  static struct etimer et2;

  PROCESS_BEGIN();
  etimer_set(&et2, CLOCK_SECOND * 5);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER && data == &et2);

    leds_toggle(LEDS_GREEN);
    printf("[Thread 2 - 5s] Toggle GREEN | Gia tri i hien tai: %d\n", i);
    i += 5;
    printf("[Thread 2 - 5s] -> Sau khi tang i (+5): %d\n", i);

    etimer_reset(&et2);
  }

  PROCESS_END();
}
