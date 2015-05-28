#ifndef __OPENNI_INCLUDE_NIDEVICE_HPP__
#define __OPENNI_INCLUDE_NIDEVICE_HPP__

#include <vector>
#include <thread>

#include "types.hpp"
#include "OpenNI2/OpenNI.h"

const char * getPixelFormatString(const openni::PixelFormat& val);

void printSupportedVideoModes(const openni::SensorInfo* info);
class NIDevice {
  openni::Device device;
  openni::VideoStream streams[3];
  openni::VideoFrameRef frames[3];
  std::mutex frame_mutex[3];
public:
  static void initONI();
  static void quitONI();

  NIDevice();
  ~NIDevice();

  void openDevice(const char* uri=openni::ANY_DEVICE);
  void listAllSensorModes();

  void createStream(const openni::SensorType type, const int mode, bool mirroring=false);
  void createDepthStream(const int mode, bool mirroring=false);
  void createColorStream(const int mode, bool mirroring=false);
  void createIRStream(const int mode, bool mirroring=false);

  uint getResolutionX(openni::SensorType type) const;
  uint getResolutionY(openni::SensorType type) const;
  uint getNumChannels(openni::SensorType type) const;
  uint getDepthResolutionX() const;
  uint getDepthResolutionY() const;
  uint getColorResolutionX() const;
  uint getColorResolutionY() const;
  uint getIRResolutionX() const;
  uint getIRResolutionY() const;
  uint getIRNumChannels() const;

  int getMinValue(openni::SensorType type) const;
  int getMaxValue(openni::SensorType type) const;
  int getDepthMinValue() const;
  int getDepthMaxValue() const;
  int getColorMinValue() const;
  int getColorMaxValue() const;
  int getIRMinValue() const;
  int getIRMaxValue() const;
    
  void setImageRegistration(const bool enable=true);
  void setDepthColorSync(const bool enable=true);

  void startStream(openni::SensorType type);
  void startAllStreams();

  void copyFrame(openni::SensorType type,
		 void* pDstBuffer, int offset, int padding) const;
  void copyDepthFrame(void* pDstBuffer, int offset=0, int padding=0) const;
  void copyColorFrame(void* pDstBuffer, int offset=0, int padding=0) const;
  void copyIRFrame(void* pDstBuffer, int offset=0, int padding=0) const;

  void stopStream(openni::SensorType type);
  void stopAllStreams();
  
  void getFrame(openni::SensorType type);
  void getAllFrames();

  void releaseFrame(openni::SensorType type);
  void releaseAllFrames();

  // The following conversion function
  void convert16BitFrameTo8Bit(openni::SensorType type,
			       uint8_t* pDst,
			       const uint color_format,
			       const uint16_t v_min,
			       const uint16_t v_max);
  // @note the unit of v_min and v_max is millimeter, not intensity value.
  void convertDepthFrameToJet(uint8_t* pDstBuffer,
			       const uint color_format=1,
			       const uint16_t v_min = DEFAULT_DEPTH_MIN,
			       const uint16_t v_max = DEFAULT_DEPTH_MAX);
  void convertIRFrameToJet(uint8_t* pDstBuffer,
			    const uint color_format=1,
			    const uint16_t v_min = DEFAULT_IR_MIN,
			    const uint16_t v_max = DEFAULT_IR_MAX);
};

#endif
