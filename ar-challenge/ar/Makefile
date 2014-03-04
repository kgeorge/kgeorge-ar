OPENCV-INCLUDES = $(shell /usr/local/bin/pkg-config --cflags opencv)

OPENCV-LIBS-DIR = $(shell /usr/local/bin/pkg-config --libs opencv | dirname `awk '{ print $$1 }'`)

OPENCV-LIBS=\
    $(OPENCV-LIBS-DIR)/libopencv_calib3d.dylib \
    $(OPENCV-LIBS-DIR)/libopencv_contrib.dylib \
    $(OPENCV-LIBS-DIR)/libopencv_core.dylib   \
    $(OPENCV-LIBS-DIR)/libopencv_features2d.dylib \
    $(OPENCV-LIBS-DIR)/libopencv_flann.dylib \
    $(OPENCV-LIBS-DIR)/libopencv_gpu.dylib \
    $(OPENCV-LIBS-DIR)/libopencv_highgui.dylib \
    $(OPENCV-LIBS-DIR)/libopencv_imgproc.dylib  \
    $(OPENCV-LIBS-DIR)/libopencv_legacy.dylib \
    $(OPENCV-LIBS-DIR)/libopencv_ml.dylib \
    $(OPENCV-LIBS-DIR)/libopencv_nonfree.dylib \
    $(OPENCV-LIBS-DIR)/libopencv_objdetect.dylib \
    $(OPENCV-LIBS-DIR)/libopencv_ocl.dylib \
    $(OPENCV-LIBS-DIR)/libopencv_photo.dylib \
    $(OPENCV-LIBS-DIR)/libopencv_stitching.dylib \
    $(OPENCV-LIBS-DIR)/libopencv_superres.dylib \
    $(OPENCV-LIBS-DIR)/libopencv_ts.a \
    $(OPENCV-LIBS-DIR)/libopencv_video.dylib \
    $(OPENCV-LIBS-DIR)/libopencv_videostab.dylib \
    -F /System/Library/Frameworks   -framework OpenGL -framework AGL


GL-EXTRA-INCLUDES = $(shell /usr/local/bin/pkg-config --cflags glew) $(shell /usr/local/bin/pkg-config --cflags glfw3)
GL-EXTRA-LIBS = $(shell /usr/local/bin/pkg-config --libs glew) $(shell /usr/local/bin/pkg-config --libs glfw3) -F /System/Library/Frameworks  -framework IOKit -framework Cocoa -framework QuartzCore

LOCAL-INCLUDES= ./../common
#LOCAL-LIBS= -L./../common/build/ libKgCommon.a
#LOCAL-LIBS= -L/Users/kgeorge/Documents/projects/kgeorge-ar/ar-challenge/common/build libKgCommon.a
LOCAL-LIBS= -L./../common/build -lKgCommon

CXX=clang++
CXXFLAGS=-g -Wall -Wno-deprecated -std=c++11 -D WITH_OPENGL  $(OPENCV-INCLUDES)  $(GL-EXTRA-INCLUDES) $(LOCAl-INCLUDES)
LDFLAGS= $(OPENCV-LIBS)  $(GL-EXTRA-LIBS) $(LOCAL-LIBS)

BUILD-DIR=build
OBJS=$(addprefix $(BUILD-DIR)/, main.o ogl.o findPattern.o)
PROG=$(BUILD-DIR)/augrel

#.PHONY: run

#run: augrel
#	./build/augrel

all: $(PROG)

#order-only-prerequisites
$(OBJS): | $(BUILD-DIR)

$(PROG): $(OBJS)
	$(CXX) -v -o $(PROG) $(OBJS) $(LDFLAGS)

$(BUILD-DIR)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(BUILD-DIR):
	mkdir -p $(BUILD-DIR)
clean:
	rm $(OBJS); rm $(PROG)
