CC = /home/chester/wujf/code/test/Firefly-RK3308_Linux_SDK_git_20190924/prebuilts/gcc/linux-x86/aarch64/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
#CC = aarch64-linux-gnu-gcc

CFLAGS = -I. -O0 -g -ggdb -lpthread -lm

CFLAGS += -I./../src/common -I./../src/net -I./../src/prt -I./../src/tools

LDFLAGS = -I. -L. -L./../src/prt -lpthread -lesc2bmp

#DEPS = ./../src/esc2bmp.h

SRCS = ./../src/main.c

include ./../src/common/common.mk
include ./../src/net/net.mk
include ./../src/prt/prt.mk
include ./../src/tools/tools.mk


OBJS = $(patsubst %.c, %.o, $(SRCS))


TARGET = d9main

#all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o:%c 
	$(CC) ${CFLAGS} -c $< -o $@

clean:
	rm -rf $(OBJS) $(TARGET)