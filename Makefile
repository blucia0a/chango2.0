GXX=g++
GCC=gcc

CFLAGS+= $(INCLUDES) -g -fPIC -O3 -pthread 
CXXFLAGS+= $(INCLUDES) -g -fPIC -O3  -pthread
LDFLAGS+= -L/usr/lib/x86_64-linux-gnu -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_videoio  -pthread -lpulse

TARGET= chango
SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,%.o,$(SRCS))

all: $(TARGET) 

%.o: %.cpp
	$(GXX) $(CXXFLAGS) -c $^ -o $@

%.o: %.c
	$(GCC) $(CFLAGS) -c $^ -o $@

$(TARGET): $(OBJS)
	$(GXX) -o $@ $^	$(LDFLAGS) 

clean:
	-rm *.o
	-rm chango
