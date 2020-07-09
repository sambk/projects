# define SRC_DIR
SRC_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))src
export SRC_DIR
ifeq ($(SRC_DIR),)
$(error "Please define SRC_DIR environment variable")
endif

# Target
A_TARGET    = libcade.a
SO_TARGET   = libcade.so
CC          = gcc
AR          = ar
RL          = ranlib
BUILD       = ./build
INC         = ./build/include
LIB         = ./build/lib
COPY_FILES  = $(INC)/cade.h $(INC)/cade_declaration.h $(INC)/cade_dealer.h $(INC)/cade_player.h $(INC)/cade_watcher.h $(LIB)/libcade.a

CFLAGS      =   -I$(SRC_DIR) -O0 -g -Wall -std=gnu99 -fPIC

SRC         =   cade_utilities.c    \
                cade_socket.c       \
                cade_msg.c          \
                cade_table.c        \
                cade_tlv.c          \
                cade_algorithm.c    \
                cade_member.c       \
                cade_common.c       \
                cade.c       		\
                cade_dealer.c       \
                cade_player.c       \
                cade_watcher.c
            
#LDFLAGS = -lpthread
OBJS    =   $(patsubst %,$(BUILD)/%, $(subst .c,.o,$(SRC)))

.PHONY: all
all: $(BUILD) $(BUILD)/$(A_TARGET) $(BUILD)/$(SO_TARGET) $(COPY_FILES)

$(BUILD):
	@mkdir -p $(BUILD)
	@mkdir -p $(INC)
	@mkdir -p $(LIB)

# Linking
$(BUILD)/$(A_TARGET) : $(OBJS)
	$(AR) rcs $(LDFLAGS) $@ $^
	$(RL) $(BUILD)/$(A_TARGET)

$(BUILD)/$(SO_TARGET) : $(OBJS)
	$(CC) ${LDFLAGS} -shared -o $@ $^    
    
# Compiling
$(BUILD)/cade_utilities.o :
	$(CC) -c $(SRC_DIR)/cade_utilities.c -o $(BUILD)/cade_utilities.o $(CFLAGS) 
$(BUILD)/cade_socket.o :
	$(CC) -c $(SRC_DIR)/cade_socket.c -o $(BUILD)/cade_socket.o $(CFLAGS)
$(BUILD)/cade_msg.o :
	$(CC) -c $(SRC_DIR)/cade_msg.c -o $(BUILD)/cade_msg.o $(CFLAGS)
$(BUILD)/cade_table.o :
	$(CC) -c $(SRC_DIR)/cade_table.c -o $(BUILD)/cade_table.o $(CFLAGS)
$(BUILD)/cade_tlv.o :
	$(CC) -c $(SRC_DIR)/cade_tlv.c -o $(BUILD)/cade_tlv.o $(CFLAGS)
$(BUILD)/cade_algorithm.o :
	$(CC) -c $(SRC_DIR)/cade_algorithm.c -o $(BUILD)/cade_algorithm.o $(CFLAGS)
$(BUILD)/cade_member.o :
	$(CC) -c $(SRC_DIR)/cade_member.c -o $(BUILD)/cade_member.o $(CFLAGS)   
$(BUILD)/cade_common.o :
	$(CC) -c $(SRC_DIR)/cade_common.c -o $(BUILD)/cade_common.o $(CFLAGS)
$(BUILD)/cade.o :
	$(CC) -c $(SRC_DIR)/cade.c -o $(BUILD)/cade.o $(CFLAGS)	
$(BUILD)/cade_dealer.o :
	$(CC) -c $(SRC_DIR)/cade_dealer.c -o $(BUILD)/cade_dealer.o $(CFLAGS)
$(BUILD)/cade_player.o :
	$(CC) -c $(SRC_DIR)/cade_player.c -o $(BUILD)/cade_player.o $(CFLAGS)
$(BUILD)/cade_watcher.o :
	$(CC) -c $(SRC_DIR)/cade_watcher.c -o $(BUILD)/cade_watcher.o $(CFLAGS)
	
# Copying files
$(COPY_FILES):
$(INC)/cade.h: $(SRC_DIR)/cade.h
$(INC)/cade_declaration.h: $(SRC_DIR)/cade_declaration.h
$(INC)/cade_dealer.h: $(SRC_DIR)/cade_dealer.h
$(INC)/cade_player.h: $(SRC_DIR)/cade_player.h
$(INC)/cade_watcher.h: $(SRC_DIR)/cade_watcher.h
$(LIB)/libcade.a: $(BUILD)/libcade.a
$(INC)/%:
	cp -f $< $@
$(LIB)/%:
	cp -f $< $@	

# Cleaning
.PHONY : clean
clean :
	rm -f $(BUILD)/$(A_TARGET) $(BUILD)/$(SO_TARGET) $(OBJS) $(INC)/*.h $(LIB)/$(A_TARGET)
