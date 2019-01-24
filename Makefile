CC=clang
CFLAGS=-Wall -Iinclude -Iinclude/glad -Wno-pragma-pack
LDFLAGS=-L. -lSDL2 -llibfbxc
BINARY=rasterizer
BINARY_GPU=gpu

ifeq ($(OS),Windows_NT)
	BINARY:=$(BINARY).exe
	BINARY_GPU:=$(BINARY_GPU).exe
endif

all: rasterizer gpu

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

rasterizer: aiv_math.o main.o glad.o
	$(CC) -o $(BINARY) $(LDFLAGS) $^

# gpu: gpu.o
# 	$(CC) -o $(BINARY_GPU) $(LDFLAGS) $^

# plugin.dll: plugin.c
# 	$(CC) -o $@ -shared $^