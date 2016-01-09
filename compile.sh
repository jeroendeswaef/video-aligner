#g++ -g `pkg-config --cflags gtk+-3.0 gtkmm-3.0` main.cpp `pkg-config --libs opencv gtk+-3.0 gtkmm-3.0` -std=c++11
#g++ -g `pkg-config --cflags gtk+-3.0 gtkmm-3.0` main.cpp `pkg-config --libs gtk+-3.0 gtkmm-3.0` -std=c++11
g++ -g `pkg-config --cflags gtk+-3.0 gtkmm-3.0`  -I/usr/include/ffmpeg save_to_png.c main.cpp `pkg-config --libs gtk+-3.0 gtkmm-3.0` -lavutil -lavformat -lavcodec -lz -lavutil -lm -lswscale -std=c++11
