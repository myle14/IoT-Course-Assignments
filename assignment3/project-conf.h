#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/* 1. CHON MAC DRIVER (Chon 1 trong 2): */
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     csma_driver
/* #define NETSTACK_CONF_MAC     nullmac_driver */

/* 2. CHON RDC DRIVER (Chon 1 trong 3): */
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     contikimac_driver
/* #define NETSTACK_CONF_RDC     cxmac_driver */
/* #define NETSTACK_CONF_RDC     nullrdc_driver */

/* Tan so kiem tra kenh (Channel Check Rate) cua RDC (Hz): 2, 4, 8, 16 */
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 8

/* 4. CHON FRAMER PHU HOP */
#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154
// Neu dung NullMAC + NullRDC co the dung:
// #define NETSTACK_CONF_FRAMER framer_nullmac

/* Bat bo dem nang luong Energest de do Duty Cycle */
#define ENERGEST_CONF_ON 1

#endif /* PROJECT_CONF_H_ */
