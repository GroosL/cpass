CC      ?= cc
CFLAGS  ?= -Wall -Wextra -O2
DBGFLAGS = -ggdb -Wall -Weffc++ -Wextra -Wsign-conversion -Og
TARGET  = cpass
SRC     = main.c

PKG_CONFIG ?= pkg-config
GPGME_CFLAGS := $(shell $(PKG_CONFIG) --cflags gpgme 2>/dev/null)
GPGME_LIBS := $(shell $(PKG_CONFIG) --libs gpgme 2>/dev/null)

CPPFLAGS += $(GPGME_CFLAGS)
LDLIBS += $(if $(GPGME_LIBS),$(GPGME_LIBS),-lgpgme)

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

debug: $(SRC)
	$(CC) $(CPPFLAGS) $(DBGFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean debug
