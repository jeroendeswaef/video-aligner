#include <vector>
#include <iostream>
#include <thread>         // std::thread
#include <mutex>          // std::mutex
#include <algorithm>      // std::min
//#include <opencv2/opencv.hpp>
#include <gtkmm/builder.h>
#include <gtkmm/main.h>


#include <gtkmm/window.h>
#include <glibmm/dispatcher.h>
#include <gtkmm/image.h>
#include <gtkmm/scale.h>

#include "Queue.h"
#include "DrawParams.h"

#include "save_to_png.h"

Gtk::Image *drawingImage;
volatile bool threadRun;
std::mutex myMutex;
std::thread myThread;

Glib::Dispatcher dispatcher;
//cv::Mat frame, outImage;

Gtk::Scale* scaleLeft = 0;

Queue<DrawParams> toProcessQ;
DrawParams currentDrawParams;

struct Surface 
{
    uint width;
    uint height;
    bool isValid;
};

void run() {

    /*cv::Mat originalFrame;
    cv::VideoCapture cap ( "test.mp4" ); // open the default camera
    threadRun = true;
    if( ! cap.isOpened () )  // check if we succeeded
        return;
    double max_frame ( cap.get ( CV_CAP_PROP_FRAME_COUNT ) );
    std::cout << "max frame" << max_frame << std::endl;
    while(1) {
        auto item = toProcessQ.pop();
        uint pos = item.getPos();
        std::cout << "popped" << pos << item.getWidth() << std::endl;
        auto requested_frame = (max_frame * pos / 100);
        cap.set (CV_CAP_PROP_POS_FRAMES, requested_frame);
        bool success = cap.read(originalFrame); 
        
        myMutex.lock();
        uint areaWidth = item.getWidth();
        uint areaHeight = item.getHeight();
        cv::Size s = originalFrame.size();
        uint imgWidth = s.width;
        uint imgHeight = s.height;
        Surface width_bound;
        width_bound.width = std::min(imgWidth, areaWidth);
        width_bound.height = width_bound.width * imgHeight / imgWidth;
        width_bound.isValid = (width_bound.height <= areaHeight);
        std::cout << "width bound:" << width_bound.width << "x" << width_bound.height << ", valid? " << width_bound.isValid << std::endl;
        Surface height_bound;
        height_bound.height = std::min(imgHeight, areaHeight);
        height_bound.width = height_bound.height * imgWidth / imgHeight;
        height_bound.isValid = (height_bound.width <= areaWidth);
        std::cout << "height bound:" << height_bound.width << "x" << height_bound.height << ", valid? " << height_bound.isValid << std::endl;
        Surface* p_surface_to_use;
        if (width_bound.isValid && height_bound.isValid) {
            p_surface_to_use = (width_bound.width * width_bound.height > height_bound.width * height_bound.height ? &width_bound : &height_bound);
        } else if (width_bound.isValid) {
            p_surface_to_use = &width_bound;
        } else {
            p_surface_to_use = &height_bound;
        }
        cv::Size size(p_surface_to_use->width, p_surface_to_use->height);
        resize(originalFrame,frame,size);//resize image
        myMutex.unlock();
        
        dispatcher.emit();
    }*/
}

void updateLeft(DrawParams& newDrawParams) 
{
    if (newDrawParams != currentDrawParams) 
    {
        toProcessQ.push(newDrawParams);
        currentDrawParams = newDrawParams;
    }
}

void updateSliderLeft() 
{
    std::cout << "update left: " << scaleLeft->get_value() << std::endl;
    DrawParams newDrawParams(currentDrawParams);
    newDrawParams.setPos(scaleLeft->get_value());
    updateLeft(newDrawParams);
}

bool updateSize(GdkEventConfigure*) 
{
    //std::cout << "update size:" << p_config->width << std::endl;

    std::cout << "update size:" << drawingImage->get_allocated_width() << std::endl;
    DrawParams newDrawParams(currentDrawParams);
    newDrawParams.setDimensions(drawingImage->get_allocated_width(), drawingImage->get_allocated_height());
    updateLeft(newDrawParams);
    return false;
}

int main(int argc, char** argv) {
    Gtk::Main kit(argc, argv);

    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file("ui.glade");

    Gtk::Window *window1 = 0;
    builder->get_widget("window1", window1);
    builder->get_widget("image1", drawingImage);

    
    builder->get_widget("scale1", scaleLeft);

    scaleLeft->set_range(1, 100);
    scaleLeft->set_value(40);
    currentDrawParams.setPos(40);

    uint8_t *p_image_data = NULL;
    int width, height, linesize;
    video_aligner_get_frame(&p_image_data, &width, &height, &linesize);
    //printf("wxh: %dx%d", width, height);
    drawingImage->set(Gdk::Pixbuf::create_from_data(p_image_data, Gdk::COLORSPACE_RGB, false, 8, width, height, linesize));
    /*scaleLeft->signal_value_changed().connect(sigc::ptr_fun(&updateSliderLeft));

    // putting the signal on drawingImage doesn't fire the signal
    window1->signal_configure_event().connect(sigc::ptr_fun(&updateSize), false);

    dispatcher.connect([&]() {
        myMutex.lock();
        cv::cvtColor(frame, outImage, CV_BGR2RGB);
        drawingImage->set(Gdk::Pixbuf::create_from_data(outImage.data, Gdk::COLORSPACE_RGB, false, 8, outImage.cols, outImage.rows, outImage.step));
        drawingImage->queue_draw();
        myMutex.unlock();
    });

    myThread = std::thread(&run);*/


    Gtk::Main::run(*window1);

    free(p_image_data);
    return 0;
}