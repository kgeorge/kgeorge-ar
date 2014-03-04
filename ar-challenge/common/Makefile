INCLUDES = $(shell /usr/local/bin/pkg-config --cflags opencv)

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

BUILD-DIR=build

GL-EXTRA-INCLUDES = $(shell /usr/local/bin/pkg-config --cflags glew) $(shell /usr/local/bin/pkg-config --cflags glfw3)
GL-EXTRA-LIBS = $(shell /usr/local/bin/pkg-config --libs glew) $(shell /usr/local/bin/pkg-config --libs glfw3) -F /System/Library/Frameworks  -framework IOKit -framework Cocoa -framework QuartzCore



CXX=clang++
CXXFLAGS=-g -Wall -Wno-deprecated -std=c++11 -D WITH_OPENGL  $(INCLUDES)  $(GL-EXTRA-INCLUDES)
LDFLAGS=$(OPENCV-LIBS)  $(GL-EXTRA-LIBS)

OBJS= $(addprefix $(BUILD-DIR)/, kgBackground.o kgShaderSupport.o kgUtil.o)
PROG=$(BUILD-DIR)/libKgCommon.a

#.PHONY: run

#run: ar
#	./ar

all: $(PROG)

$(OBJS): | $(BUILD-DIR)


$(PROG): $(OBJS)
	ar rcs $(PROG) $(OBJS)

$(BUILD-DIR)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@  $<

$(BUILD-DIR):
	mkdir -p $(BUILD-DIR)


clean:
	rm $(OBJS); rm $(PROG)
