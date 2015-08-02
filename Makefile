
TARGET := a.out
SRC := main.c neopixel_i2c_host.c
OBJS = $(patsubst %.c,%.o,$(SRC))

CFLAGS = -Wall -g

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o $(TARGET)

.PHONY: clean all
