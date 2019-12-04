TARGET := test

#COMPILE_CROSS:= arm-hisiv300-linux-
CC := $(COMPILE_CROSS)gcc
GG := $(COMPILE_CROSS)g++

 
INC := -I ./include/

SRC_C		     := $(wildcard ./src/*.c)
SRC_OBJ	  	     := $(patsubst %.c,%.o,$(SRC_C))

CFLAGS += -g -DWITH_DOM -DWITH_OPENSSL

LDLIBS := ./lib/libssl.a
LDLIBS += ./lib/libcrypto.a
LDLIBS += -lpthread -lm -lz -lrt -ldl
 
$(TARGET): $(SRC_OBJ)
	$(CC) -o $(TARGET) $(SRC_OBJ) $(LDLIBS)
	
$(SRC_OBJ):%.o:%.c
	echo $(SRC_OBJ)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC)	
 
clean:
	$(RM) src/*.o 
	$(RM) $(TARGET)
