#ifndef __OPENNI_INCLUDE_NIDEVICE_HPP__
#define __OPENNI_INCLUDE_NIDEVICE_HPP__

#include <string>
#include <stdexcept>
#include <mutex>
#include <functional>


#include "types.hpp"
#include "OpenNI2/OpenNI.h"

const char * getPixelFormatString(const openni::PixelFormat& val);

void printSupportedVideoModes(const openni::SensorInfo* info);

class RuntimeError : public std::exception {
  std::string m_message;
public:
  RuntimeError(std::string fn, std::string msg);
  virtual const char* what() const throw();
};

class NIDevice {
  class Streamer {
    class Listener : public openni::VideoStream::NewFrameListener {
      std::function<void()> m_callbackFcn;
    public:
      Listener();
      ~Listener();
      void onNewFrame (openni::VideoStream &stream) override;
      void setCallbackFcn (std::function<void()> fcn);
    };

    openni::VideoStream   m_stream;
    openni::VideoFrameRef m_frame;
    std::mutex            m_frameMutex;
    Listener              m_listener;
  public:
    Streamer();
    ~Streamer();

    void create(openni::Device &device, const openni::SensorType type,
		const int mode=0, const bool mirroring=false);
    bool isValid() const;
    void start();
    void stop();

    void copyTo(void* pDst, const uint offset=0, const uint padding=0);
    uint getWidth() const;
    uint getHeight() const;
    uint getNumChannels() const;
    uint getMinValue() const;
    uint getMaxValue() const;
  };

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

  void stopStream(openni::SensorType type);
  void stopAllStreams();

  void copyFrame(openni::SensorType type,
		 void* pDstBuffer, int offset, int padding);
  void copyDepthFrame(void* pDstBuffer, int offset=0, int padding=0);
  void copyColorFrame(void* pDstBuffer, int offset=0, int padding=0);
  void copyIRFrame(void* pDstBuffer, int offset=0, int padding=0);
};
#endif // __OPENNI_INCLUDE_NIDEVICE_HPP__
