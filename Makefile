CC      = cc
CFLAGS  = -Wall -Wextra -O2
LDLIBS  = -lgpgme
TARGET  = cpass

SRC     = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
