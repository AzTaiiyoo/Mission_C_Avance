# Robot (MRPiZ C) - Makefile pour création de deux exécutables

# Emplacements
BINDIR = ../bin
DOCDIR = ../doc
LIB_MRPIZ = $(realpath  ../../lib_mrpiz-aarch64-v0.5/)

# Définitions des binaires
CLIENT_EXE = $(BINDIR)/client
SERVER_EXE = $(BINDIR)/server

# Définitions des outils
CC = gcc

# Flags communs
CCFLAGS  = -DINTOX -DINTOX_ADDRESS=127.0.0.1 -DINTOX_PORT=12345
CCFLAGS += -I"$(LIB_MRPIZ)/include/mrpiz/"
CCFLAGS += -MMD -MP
CCFLAGS += -D_BSD_SOURCE -D_XOPEN_SOURCE_EXTENDED -D_XOPEN_SOURCE -D_DEFAULT_SOURCE -D_GNU_SOURCE
CCFLAGS += -std=gnu11 -Wall -pedantic
CCFLAGS += -fanalyzer -Wformat=2 -Wformat-overflow=2 -Wformat-truncation=2 -Winit-self -Wstringop-overflow=4 -ftrapv -fstack-protector-strong -fPIE -Wdate-time
CCFLAGS += -Og -g -DDEBUG

# Flags pour le linker
LDFLAGS  = -L"$(LIB_MRPIZ)/lib/" -lintoxmrpiz -lintox
LDFLAGS += -lm
LDFLAGS += -fsanitize=undefined -fPIE

# Sources du serveur
SERVER_SRC = serveur.c robot_app/robot.c robot_app/robot_state.c robot_app/pilot.c robot_app/copilot.c
SERVER_OBJ = $(SERVER_SRC:.c=.o)
SERVER_DEP = $(SERVER_SRC:.c=.d)

# Sources du client
CLIENT_SRC = client.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
CLIENT_DEP = $(CLIENT_SRC:.c=.d)

# Règles
.PHONY: all clean server client

all: server client

server: $(SERVER_EXE)

client: $(CLIENT_EXE)

$(SERVER_EXE): $(SERVER_OBJ)
	$(CC) $(CCFLAGS) $(SERVER_OBJ) $(LDFLAGS) -o $(SERVER_EXE)

$(CLIENT_EXE): $(CLIENT_OBJ)
	$(CC) $(CCFLAGS) $(CLIENT_OBJ) $(LDFLAGS) -o $(CLIENT_EXE)

.c.o:
	$(CC) -c $(CCFLAGS) $< -o $@

-include $(SERVER_DEP) $(CLIENT_DEP)

clean:
	@rm -f $(SERVER_EXE) $(CLIENT_EXE) $(BINDIR)/core*
	@rm -rf $(DOCDIR)
	@rm -f $(SERVER_DEP) $(SERVER_OBJ) $(CLIENT_DEP) $(CLIENT_OBJ)