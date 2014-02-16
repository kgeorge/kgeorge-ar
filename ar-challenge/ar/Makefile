INCLUDES = $(shell /usr/local/bin/pkg-config --cflags opencv)
INCLUDES-EIGEN=$(shell /usr/local/bin/pkg-config --cflags opencv)
OPENCV-LIBS-DIR = $(shell /usr/local/bin/pkg-config --libs opencv | dirname `awk '{ print $$1 }'`)
EIGEN-LIBS = $(shell /usr/local/bin/pkg-config --libs eigen3)
EIGEN-INCLUDES=$(shell /usr/local/bin/pkg-config --cflags eigen3)
LIBS=\
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
    -F /System/Library/Frameworks -framework OpenGL -framework AGL




    

CXX=clang++
CXXFLAGS=-g -Wall -Wno-deprecated -std=c++11 -D WITH_OPENGL -D USE_GLUT  $(INCLUDES) $(EIGEN-INCLUDES)
LDFLAGS=$(LIBS) $(EIGEN-LIBS)
#LDFLAGS=

OBJS=ar.o ogl.o
PROG=ar

#.PHONY: run

#run: ar
#	./ar

all: $(PROG)


$(PROG): $(OBJS)
	#@echo hihihi $(OPENCV-LIBS-1)
	$(CXX) -v -o $(PROG) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $<

#ar: ar.cpp

clean:
	rm $(OBJS); rm ar
