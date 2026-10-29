CONTIKI_PROJECT = assignment3-node
all: $(CONTIKI_PROJECT)

# Bat thu vien mang Rime de su dung cac ham broadcast
CONTIKI_WITH_RIME = 1

# Khai bao file cau hinh project-conf.h
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

# Duong dan tro ve thu muc goc contiki 
CONTIKI = ../..
include $(CONTIKI)/Makefile.include
