CC=gcc
CFLAGS=-Wall -g

TARGET=main
SOURCES=main.c networking.c handler.c
OBJECTS=$(SOURCES:.c=.o)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS)
