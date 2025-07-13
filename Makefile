CC=gcc -g
TARGET=StreamingService
SRC=main.c streaming_service.c

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET)

.PHONY: clean
clean:
	rm -f $(TARGET)
