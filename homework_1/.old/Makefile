OUT_DIR = _build/
SRC_DIR = ./

S_SRCS += $(SRC_DIR)server.c
C_SRCS += $(SRC_DIR)client.c

S_OBJS := $(S_SRCS:$(SRC_DIR)%.c=$(OUT_DIR)%.o)
C_OBJS := $(C_SRCS:$(SRC_DIR)%.c=$(OUT_DIR)%.o)

S_OUT   = $(OUT_DIR)server
C_OUT   = $(OUT_DIR)client

CFLAGS += `pkg-config gtk+-2.0 --cflags --libs -std=c99`
DEBUG_FLAG += -g

.PHONY : all clean

all: $(S_OBJS) $(C_OBJS)
	$(CC) $(CFLAGS) $(S_OBJS) -o $(S_OUT)
	$(CC) $(CFLAGS) $(C_OBJS) -o $(C_OUT)

$(S_OBJS): $(OUT_DIR)%.o : $(SRC_DIR)%.c $(OUT_DIR)
	$(CC) $(CFLAGS) $(DEBUG_FLAG) -c $< -o $@

$(C_OBJS): $(OUT_DIR)%.o : $(SRC_DIR)%.c $(OUT_DIR)
	$(CC) $(CFLAGS) $(DEBUG_FLAG) -c $< -o $@

$(OUT_DIR):
	mkdir -p $@

clean:
	rm -rf $(OUT_DIR)
