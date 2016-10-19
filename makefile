CFLAGS = -Wall
TARGETS = server client

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
