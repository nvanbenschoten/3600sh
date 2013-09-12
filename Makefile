TARGET = 3600sh

BIN_DIR = bin/
VPATH = src 

$(TARGET): $(TARGET).c
	gcc -std=c99 -O0 -g -lm -Wall -pedantic -Werror -Wextra -o $(BIN_DIR)$@ $<

all: $(BIN_DIR)$(TARGET)

test: all
	$(BIN_DIR)test

clean:
	rm -f $(BIN_DIR)$(TARGET)

