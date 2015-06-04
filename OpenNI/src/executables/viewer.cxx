// c++ src/executables/viewer.cxx src/common/common_defs.cxx  src/OpenNI/ONIStreamer.cxx -I./include/ --std=c++11 -lOpenNI2

#include "common/common_defs.hpp"
#include "OpenNI/ONIStreamer.hpp"

void main_exec() {
  ONIStreamer onis;
  onis.openDevice();
  onis.listAvailableSensors();
  onis.createDepthStream();
  onis.startStreaming();

  onis.stopStreaming();
  onis.closeDevice();
};

int main() {
  ONIStreamer::initONI();
  try {
    main_exec();
  } catch (RuntimeError &e) {
    printf("%s\n", e.what());
  }
  ONIStreamer::quitONI();
  return 0;
};
