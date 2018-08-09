GXX=g++
GCC=gcc

CFLAGS+= $(INCLUDES) -g -fPIC -O3  
CXXFLAGS+= $(INCLUDES) -g -fPIC -O3  
LDFLAGS+= -L/usr/lib/x86_64-linux-gnu -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_videoio -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_stitching

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
