#ifndef __NIDEVICE_HPP__
#define __NIDEVICE_HPP__

#include "OpenNI2/OpenNI.h"

const char * getPixelFormatString(const openni::PixelFormat& val);

void printSupportedVideoModes(const openni::SensorInfo* info);

class NIDevice {  
  openni::Device device;
  openni::VideoStream depthStream;
  openni::VideoStream colorStream;
  openni::VideoFrameRef depthFrame;
  openni::VideoFrameRef colorFrame;

public:
  static void initONI();
  static void quitONI();
  
  NIDevice();
  ~NIDevice();

  void initDevice(int depthMode=0, int colorMode=0);
  
  void openDevice(const char* uri=openni::ANY_DEVICE);
  void listAllSensorModes();
  
  void createDepthStream();
  void createColorStream();

  void setDepthMode(const int i);
  void setColorMode(const int i);
  
  void getDepthResolution(int& w, int& h) const;
  void getColorResolution(int& w, int& h) const;

  void setImageRegistration(const bool enable=true);
  void setDepthColorSync(const bool enable=true);

  void startColorStream();
  void startDepthStream();
  
  void readDepthStream();
  void readColorStream();

  void copyDepthData(uint16_t* const pBuffer, int offset=1, int skip=1);
  void copyColorData(uint8_t* const pBuffer, int offset=1, int skip=1);

  void releaseDepthFrame();
  void releaseColorFrame();
};

#endif
