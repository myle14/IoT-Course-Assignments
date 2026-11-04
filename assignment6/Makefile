CONTIKI_PROJECT = wakeup-node
all: $(CONTIKI_PROJECT)

# Kich hoat ngan xep mang Rime
CONTIKI_WITH_RIME = 1

# Khai bao file cau hinh project-conf.h
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
