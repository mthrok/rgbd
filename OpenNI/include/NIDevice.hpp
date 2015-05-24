#ifndef __OPENNI_INCLUDE_NIDEVICE_HPP__
#define __OPENNI_INCLUDE_NIDEVICE_HPP__

#include "types.hpp"
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
  
  void getDepthResolution(uint& w, uint& h) const;
  void getColorResolution(uint& w, uint& h) const;

  void setImageRegistration(const bool enable=true);
  void setDepthColorSync(const bool enable=true);

  void startColorStream();
  void startDepthStream();
  
  void readDepthStream();
  void readColorStream();

  void copyDepthFrame(uint16_t* const pBuffer, int offset=0, int padding=0);
  void copyColorFrame(uint8_t* const pBuffer, int offset=0, int padding=0);
  void convertDepthFrameToJet(uint8_t* pDst, const uint mode=1,
			      const uint16_t v_min = DEFAULT_DEPTH_MIN,
			      const uint16_t v_max = DEFAULT_DEPTH_MAX);

  void releaseDepthFrame();
  void releaseColorFrame();
};

#endif
