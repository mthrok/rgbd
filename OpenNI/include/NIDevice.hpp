#ifndef __OPENNI_INCLUDE_NIDEVICE_HPP__
#define __OPENNI_INCLUDE_NIDEVICE_HPP__

#include <mutex>
#include <atomic>
#include <string>
#include <functional>

#include "types.hpp"
#include "OpenNI2/OpenNI.h"

const char* getSensorTypeString(const openni::SensorType type);
const char* getPixelFormatString(const openni::PixelFormat val);

void printSupportedVideoModes(const openni::SensorInfo* info);

class Listener : public openni::VideoStream::NewFrameListener {
  std::function<void()> m_callbackFcn;
public:
  Listener();
  ~Listener();
  void onNewFrame (openni::VideoStream &stream) override;
  void setCallbackFcn (std::function<void()> fcn);
};

class Streamer {
  openni::VideoStream   m_stream;
  openni::VideoFrameRef m_frame;
  std::mutex            m_frameMutex;
  Listener              m_listener;
  bool                  m_bStreaming;
  std::atomic<std::chrono::microseconds> m_time;
public:
  Streamer();
  ~Streamer();

  void create(openni::Device &device, const openni::SensorType type,
	      const int mode=0, const bool mirroring=false);
  void start();
  void stop();
  bool isStreamValid() const;
  bool isStreaming() const;
  bool isFrameValid() const;

  uint getWidth() const;
  uint getHeight() const;
  uint getNumChannels() const;
  uint getMinValue() const;
  uint getMaxValue() const;
  void copyTo(void* pDst, const uint offset=0, const uint padding=0);
};

class NIDevice {
  openni::Device m_device;
  Streamer       m_streamers[3];
public:
  static void initONI();
  static void quitONI();

  NIDevice();
  ~NIDevice();

  void openDevice(const char* uri=openni::ANY_DEVICE);
  void listAllSensorModes();

  void createStream(const openni::SensorType type, const int mode=0, bool mirroring=false);
  void createDepthStream(const int mode, bool mirroring=false);
  void createColorStream(const int mode, bool mirroring=false);
  void createIRStream(const int mode, bool mirroring=false);

  uint getWidth(openni::SensorType type) const;
  uint getHeight(openni::SensorType type) const;
  uint getNumChannels(openni::SensorType type) const;
  int getMinValue(openni::SensorType type) const;
  int getMaxValue(openni::SensorType type) const;

  uint getDepthWidth() const;
  uint getDepthHeight() const;
  int getDepthMinValue() const;
  int getDepthMaxValue() const;

  uint getColorWidth() const;
  uint getColorHeight() const;
  int getColorMinValue() const;
  int getColorMaxValue() const;

  uint getIRWidth() const;
  uint getIRHeight() const;
  uint getIRNumChannels() const;
  int getIRMinValue() const;
  int getIRMaxValue() const;

  void setImageRegistration(const bool enable=true);
  void setDepthColorSync(const bool enable=true);

  void startStream(openni::SensorType type);
  void startStreams();

  void waitStreamsToGetReady() const;

  void stopStream(openni::SensorType type);
  void stopStreams();

  void copyFrame(openni::SensorType type,
		 void* pDstBuffer, int offset, int padding);
  void copyDepthFrame(void* pDstBuffer, int offset=0, int padding=0);
  void copyColorFrame(void* pDstBuffer, int offset=0, int padding=0);
  void copyIRFrame(void* pDstBuffer, int offset=0, int padding=0);
};
#endif // __OPENNI_INCLUDE_NIDEVICE_HPP__
