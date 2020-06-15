
#PLATFORM = 2410
PLATFORM = 2416

ifeq ($(PLATFORM),2410)
CC=/usr/local/arm/2.95.3/bin/arm-linux-g++
else
CC=arm-linux-g++
endif

CFLAGS=-Wall -lrt

LIBPATH=../
LDFLAGS=-I$(LIBPATH) -L$(LIBPATH) -lfunc

TARGET=a.out
SRC=main_power_off.cpp

all: $(TARGET)
target: $(TARGET) 

$(TARGET): $(SRC) 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
#	cp -rf $(TARGET) /home/share/$(TARGET)
clean:
	rm -f $(TARGET) 
