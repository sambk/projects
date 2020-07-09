# define SRC_DIR
SRC_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
INC_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))../../build/include
LIB_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))../../build/lib
export SRC_DIR
ifeq ($(SRC_DIR),)
$(error "Please define SRC_DIR environment variable")
endif

# Target
TARGET  = test_cade_dealer
CC      = gcc
BUILD   = ./build

CFLAGS  = 	-I$(SRC_DIR) -I$(INC_DIR) -O0 -g -Wall -std=gnu99

SRC         =   test_cade_dealer.c

LDFLAGS = -lcade -L$(LIB_DIR) -lrt -lpthread -lm
OBJS 	=	$(patsubst %,$(BUILD)/%, $(subst .c,.o,$(SRC)))

.PHONY : all
all : $(BUILD) $(BUILD)/$(TARGET)

$(BUILD):
	@mkdir -p $(BUILD)

$(BUILD)/$(TARGET) : $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD)/test_cade_dealer.o :
	$(CC) -c test_cade_dealer.c -o $(BUILD)/test_cade_dealer.o $(CFLAGS)

.PHONY : clean
clean :
	rm $(BUILD)/$(TARGET) $(OBJS)
