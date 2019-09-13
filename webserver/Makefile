CC=gcc
CPPFLAGS= -I./include
CFLAGS=-Wall 


#src path
BIN_PATH=./bin
SRC_PATH=./src
COMMON_PATH=./common


# server app
server=$(BIN_PATH)/shttpd 	

target=$(test_server) $(server) 


ALL:$(target)


# server
$(server):	$(SRC_PATH)/server.o \
	$(COMMON_PATH)/event.o \
	$(COMMON_PATH)/event.o \
	$(COMMON_PATH)/utils.o \
	$(COMMON_PATH)/wrap.o
	$(CC) $^ -o $@
	
#clean指令
clean:
	rm -rf $(target)  $(SRC_PATH)/*.o $(COMMON_PATH)/*.o

distclean:
	rm -rf $(target)  $(SRC_PATH)/*.o $(COMMON_PATH)/*.o

#将clean目标 改成一个虚拟符号
.PHONY: clean ALL distclean
