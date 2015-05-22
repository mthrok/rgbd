#ifndef __NIDEVICE_HPP__
#define __NIDEVICE_HPP__

#include "OpenNI2/OpenNI.h"

class NIDevice {
public:
  openni::Device device;
  openni::VideoStream depth;
  openni::VideoStream color;
  //public:
  static void initOpenNI();
  static void quitOpenNI();
  
  NIDevice();
  void connect(const char* uri=openni::ANY_DEVICE);
  ~NIDevice();
};

#endif
