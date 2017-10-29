OUT_DIR = _build/
SRC_DIR = ./

S_SRCS += $(SRC_DIR)socket_epoll_server.c
C_SRCS += $(SRC_DIR)socket_epoll_client.c

S_OBJS := $(S_SRCS:$(SRC_DIR)%.c=$(OUT_DIR)%.o)
C_OBJS := $(C_SRCS:$(SRC_DIR)%.c=$(OUT_DIR)%.o)

S_OUT   = $(OUT_DIR)server
C_OUT   = $(OUT_DIR)client

CFLAGS += -std=c99 -D__GLIBC_USE 
DEBUG_FLAG += -g

.PHONY : all server client clean

all: $(OUT_DIR) $(S_OUT) $(C_OUT)

$(S_OUT): $(S_OBJS)
	$(CC) $(CFLAGS) $(S_OBJS) -o $(S_OUT)

$(C_OUT): $(C_OBJS)
	$(CC) $(CFLAGS) $(C_OBJS) -o $(C_OUT)

$(S_OBJS): $(OUT_DIR)%.o : $(SRC_DIR)%.c
	$(CC) $(CFLAGS) $(DEBUG_FLAG) -c $< -o $@

$(C_OBJS): $(OUT_DIR)%.o : $(SRC_DIR)%.c
	$(CC) $(CFLAGS) $(DEBUG_FLAG) -c $< -o $@

$(OUT_DIR):
	mkdir -p $@

clean:
	rm -rf $(OUT_DIR)
