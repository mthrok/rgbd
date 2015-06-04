#ifndef DEPTH_REASONING_OPENNI_ONISTREAMER_HPP
#define DEPTH_REASONING_OPENNI_ONISTREAMER_HPP

#include "OpenNI2/OpenNI.h"
#include "common/common_defs.hpp"

#include <mutex>
#include <atomic>
#include <thread>

#define WAIT_STREAM_TIMEOUT 500 // [ms]

class ONIStreamer {
  openni::Device        device_;
  openni::VideoStream   streams_[3];
  openni::VideoFrameRef frames_[3];

  std::atomic<bool> streaming_;
  std::mutex  frameMutex_[3];
  std::thread frameUpdater_[3];

  void updateFrameRef(const openni::SensorType type);
public:
  static void initONI();
  static void quitONI();

  ONIStreamer();   // TODO check initializer list
  ~ONIStreamer();

  void openDevice(const char* uri=openni::ANY_DEVICE);
  void closeDevice();

  void listAvailableSensors();

  void createStream(const openni::SensorType type,
		    const int mode=0, bool mirroring=false);
  void createDepthStream(const int mode=0, bool mirroring=false);

  void startStreaming();
  void stopStreaming();

  void getFrame(const openni::SensorType type, void* pBuff, uint64_t* time);
};


#endif // #ifndef DEPTH_REASONING_OPENNI_STREAMER_HPP
