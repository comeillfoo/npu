CC=gcc
LD=gcc

INC_DIR=include
SOURCE_DIR=src
BUILD_DIR=build

CFLAGS=-std=gnu11 -Wall -pedantic -I$(INC_DIR)

LIBS=-lm

EXEC=npu

all: $(BUILD_DIR)/main.o $(BUILD_DIR)/neural_net.o
	$(LD) $(LDFLAGS) -o $(EXEC) $^ $(LIBS)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(EXEC)

.PHONY: clean
