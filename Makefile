CC = gcc
CFLAGS = -O2 -Wall -std=c99
LDFLAGS = -lraylib -lSDL2 -lSDL2_mixer -lGL -lm -lpthread -ldl -lrt -lX11
SRC = raylib-water-plane.c
OUT = raylib-water-plane

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -f $(OUT)