#include "common/common_defs.hpp"

using namespace std::chrono;

microseconds getCurrentTimestamp() {
  return duration_cast<microseconds>(system_clock::now().time_since_epoch());
}

void RuntimeError::concat(std::string message) {
  message_ += message;
}

void RuntimeError::concat(const char* message) {
  message_ += message;
}

const char* RuntimeError::what() const throw () {
  return message_.c_str();
}

VideoFrame::VideoFrame(uint width, uint height, uint BPP)
  : pFrame_(NULL)
  , width_(width)
  , height_(height)
  , BPP_(BPP)
{};

void VideoFrame::setSize(uint width, uint height, uint BPP){
  if (pFrame_)
    throw RuntimeError(__func__,
		       ": Can't alter frame size while buffer is allocated.");
  width_ = width; height_ = height; BPP_ = BPP;
}

void VideoFrame::allocate() {
  if (pFrame_)
    throw RuntimeError(__func__, ": Buffer already allocated.");
  pFrame_ = new uint8_t[static_cast<uint64_t>(width_) * height_ * BPP_];
};

void VideoFrame::free() {
  delete[] pFrame_;
  pFrame_ = NULL;
}

void VideoFrame::copyDataFrom(void* pSrcBuff, int width, int height, int BPP) {
  if (!pSrcBuff)
    throw RuntimeError(__func__, ": Invalid source buffer.");
  if (!pFrame_)
    throw RuntimeError(__func__, ": Destination buffer not allocated.");

  if (width < 0)
    width = width_;
  if (height < 0)
    height = height_;
  if (BPP < 0)
    BPP = BPP_;

  uint64_t srcSize = static_cast<uint64_t>(width) * height * BPP;
  uint64_t dstSize = static_cast<uint64_t>(width_) * height_ * BPP_;
  if (dstSize < srcSize)
    throw RuntimeError(__func__, ": Not enough buffer.");

  uint8_t* pSrc = static_cast<uint8_t*>(pSrcBuff);
  uint8_t* pDst = pFrame_;
  for (uint64_t i=0; i<dstSize; ++i) {
    *pDst = *pSrc; ++pDst; ++pSrc;
  }
};

void VideoFrame::setTimestamp(std::chrono::microseconds timestamp) {
  timestamp_ = timestamp;
};
