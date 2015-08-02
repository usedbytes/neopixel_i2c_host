
TARGET := a.out
SRC := main.c neopixel_i2c_host.c pcm.c
OBJS = $(patsubst %.c,%.o,$(SRC))

CFLAGS = -Wall -g
LDFLAGS = -lasound

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o $(TARGET)

.PHONY: clean all
