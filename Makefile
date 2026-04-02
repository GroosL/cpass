CC      = cc
CFLAGS  = -Wall -Wextra -O2
DBGFLAGS = -ggdb -Wall -Weffc++ -Wextra -Wsign-conversion -Og
LDLIBS  = -lgpgme
TARGET  = cpass

SRC     = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

debug: $(SRC)
	$(CC) $(DBGFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
