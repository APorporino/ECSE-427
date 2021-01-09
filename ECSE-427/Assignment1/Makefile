# .DEFAULT_GOAL=all

CC=gcc
CFLAGS=-fsanitize=signed-integer-overflow -fsanitize=undefined -g -std=gnu99 -O2 -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow

CLIENT=frontend
SERVER=backend

CLEINT_DEBUG=frontend_debug
SERVER_DEBUG=backend_debug

rpc: a1_lib.c frontend.c backend.c rpc.c
	$(CC) -o $(CLIENT) $(CFLAGS) frontend.c a1_lib.c rpc.c
	$(CC) -o $(SERVER) $(CFLAGS) backend.c a1_lib.c rpc.c

debug: a1_lib.c frontend.c backend.c rpc.c
	$(CC) -g -o $(CLEINT_DEBUG) $(CFLAGS) frontend.c a1_lib.c rpc.c
	$(CC) -g -o $(SERVER_DEBUG) $(CFLAGS) backend.c a1_lib.c rpc.c

clean:
	rm -rf $(CLIENT) $(SERVER)
	rm -rf $(SERVER_DEBUG) $(CLEINT_DEBUG)
	rm -rf frontend_debug.dSYM frontend.dSYM backend_debug.dSYM backend.dSYM 

