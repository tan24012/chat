CC := gcc
CFLAGS := -Wall -Wextra -g
CPPFLAGS := -I./MTSocket -I./Client -I./Server
LDLIBS := -pthread

IP ?= 127.0.0.1
PORT ?= 8080

BUILD_DIR := build
TARGET_CLIENT := $(BUILD_DIR)/client
TARGET_SERVER := $(BUILD_DIR)/server

SRCS_CLIENT := \
	MTSocket/MThread.c \
	MTSocket/TCPSocket.c \
	MTSocket/MultipleTCPSocketsListener.c \
	MTSocket/UDPSocket.c \
	Client/Peer2Peer.c \
	Client/Client.c \
	Client/main.c

SRCS_SERVER := \
	MTSocket/MThread.c \
	MTSocket/TCPSocket.c \
	MTSocket/MultipleTCPSocketsListener.c \
	Server/LoginAndSignUp.c \
	Server/Dispatcher.c \
	Server/User.c \
	Server/Server.c \
	Server/main.c

OBJS_CLIENT := $(SRCS_CLIENT:%.c=$(BUILD_DIR)/%.o)
OBJS_SERVER := $(SRCS_SERVER:%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean run-client run-server

all: $(TARGET_CLIENT) $(TARGET_SERVER)

$(TARGET_CLIENT): $(OBJS_CLIENT)
	$(CC) $^ -o $@ $(LDLIBS)

$(TARGET_SERVER): $(OBJS_SERVER)
	$(CC) $^ -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run-client: $(TARGET_CLIENT)
	./$(TARGET_CLIENT)

run-server: $(TARGET_SERVER)
	./$(TARGET_SERVER) 

clean:
	rm -rf $(BUILD_DIR)