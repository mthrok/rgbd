#include "NIDevice.hpp"
#include <stdexcept>
#include <string>

using namespace openni;

void NIDevice::initOpenNI() {
  Status rc = OpenNI::initialize();
  if (rc != STATUS_OK)
    throw std::runtime_error("Failed to initialize OpenNI.");  
}

void NIDevice::quitOpenNI() {
  OpenNI::shutdown();
}

NIDevice::NIDevice()
  : device()
  , depth()
  , color()
{}

NIDevice::~NIDevice() {

}

void NIDevice::connect(const char* uri) {
  Status rc = device.open(uri);
  std::string message = "Failed to open device.";
  message += uri;
  if (rc != STATUS_OK)
    throw std::runtime_error(message.c_str());
}
