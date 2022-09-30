#引入头信息
STCC=$(CC) -Wall -fgnu89-inline -g -c -O0 -o
STLK=$(CC) -Wl,--copy-dt-needed-entries -o
OBJ_DIR=./obj
PROG_NAME=asura

#LIB_BASE+= ./libgmssl/libssl.so.1.1 ./libgmssl/libcrypto.so.1.1 -lev -lpthread
#LIB_BASE+= -L./libgmssl -lssl -lcrypto -lev -lpthread
LIB_BASE+= /usr/local/lib/libev.a -lpthread -lm

SOURCE_FILE_LIST += common.c
SOURCE_FILE_LIST += debug_msg.c
SOURCE_FILE_LIST += sock.c
SOURCE_FILE_LIST += report.c
SOURCE_FILE_LIST += work_client.c
SOURCE_FILE_LIST += client_context.c
SOURCE_FILE_LIST += config.c
SOURCE_FILE_LIST += asura.c

OBJ_FILE_LIST:=$(addprefix $(OBJ_DIR)/, $(patsubst %.c, %.o, $(SOURCE_FILE_LIST)))

#OBJ_BASE:=$(notdir $(OBJ_FILE_LIST))

.PHONY=all clean

$(PROG_NAME):$(OBJ_FILE_LIST)
	$(STLK) ./$@ $(OBJ_FILE_LIST) $(LDFLAGS) $(LIB_BASE)

clean:
	rm -rf obj/*
	rm -f $(PROG_NAME)

$(OBJ_DIR)/%.o: %.c
	@if [ ! -d $(OBJ_DIR)/$(dir $<) ]; then mkdir -p $(OBJ_DIR)/$(dir $<) ; fi;
	$(STCC) $@ $(INCL_DIR) $<
