###############################################################################
# Makefile – RPC‑Scratch
###############################################################################
CC      := gcc
CFLAGS  := -I.      # -I. so sub‑dirs find is_prime.h
RM      := rm -f

# ── shared implementation (used by local math_program & server) --------------
COMMON_SRCS  := is_prime.c
COMMON_OBJS  := $(COMMON_SRCS:.c=.o)

# ── stand‑alone math_program (no RPC) -----------------------------------------
LOCAL_SRCS   := math_program.c
LOCAL_OBJS   := $(LOCAL_SRCS:.c=.o)
LOCAL_BIN    := math_program

# ── distributed RPC client ----------------------------------------------------
RPC_CLIENT_MAIN_SRCS := client/math_program_rpc.c
RPC_CLIENT_MAIN_OBJS := $(RPC_CLIENT_MAIN_SRCS:.c=.o)

RPC_CLIENT_STUB_SRCS := client/rpc_client.c
RPC_CLIENT_STUB_OBJS := $(RPC_CLIENT_STUB_SRCS:.c=.o)

RPC_CLIENT_BIN       := client/math_program_rpc

# ── RPC server ----------------------------------------------------------------
SERVER_SRCS := server/rpc_server.c
SERVER_OBJS := $(SERVER_SRCS:.c=.o)
SERVER_BIN  := server/rpc_server

# ── targets -------------------------------------------------------------------
.PHONY: all
all : $(LOCAL_BIN) $(RPC_CLIENT_BIN) $(SERVER_BIN)

# generic pattern rule (works in sub‑directories)
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

# link rules
$(LOCAL_BIN) : $(LOCAL_OBJS) $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(RPC_CLIENT_BIN) : $(RPC_CLIENT_MAIN_OBJS) $(RPC_CLIENT_STUB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SERVER_BIN) : $(SERVER_OBJS) $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# clean
.PHONY: clean
clean :
	$(RM) $(LOCAL_BIN) $(RPC_CLIENT_BIN) $(SERVER_BIN) \
	      $(COMMON_OBJS) $(LOCAL_OBJS) \
	      $(RPC_CLIENT_MAIN_OBJS) $(RPC_CLIENT_STUB_OBJS) \
	      $(SERVER_OBJS)

###############################################################################
# Usage
# -----
# $ make                         # builds:
#                                 #   math_program           (local test)
#                                 #   client/math_program_rpc (distributed client)
#                                 #   server/rpc_server      (server)
#
# Terminal 1:
#   $ ./server/rpc_server
#
# Terminal 2:
#   $ ./client/math_program_rpc
#
# Optional local check (no networking):
#   $ ./math_program
###############################################################################
