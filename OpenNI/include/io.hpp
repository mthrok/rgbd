#ifndef __OPENNI_INCLUDE_IO_HPP__
#define __OPENNI_INCLUDE_IO_HPP__

#include "types.hpp"

#include <cstdint>
#include <cstdlib>

//!
//! Given one value return its jet color mapping in RGB.
//! @param val   Input value
//! @param R     Output red value
//! @param G     Output green value
//! @param B     Output blue value
//! @param v_min Lower bound of input value range
//! @param v_max Upper bound of input value range
//! @note Values smaller than v_min are mapped to black, and
//!   values bigger than v_max is mapped to white.
//! 
void jet(const uint16_t val, uint8_t& R, uint8_t& G, uint8_t& B,
	 const uint16_t v_min = DEFAULT_DEPTH_MIN,
	 const uint16_t v_max = DEFAULT_DEPTH_MAX);

//!
//! Convert depth frame (16 bit 1 ch) to RGB frame (8bit 3ch) using jet
//! colormap.
//! @param format Inidicates the alignment of output buffer.
//!   1: ARGB == SDL_PIXELFORMAT_BGRA8888
//! @param v_min Minimum value to trancate. Unit: [mm]
//! @param v_max Maximum value to trancate. Unit: [mm]
//!
void convert16BitFrameToJet(const uint16_t* pSrc, uint8_t* pDst,
			    const uint width, const uint height,
			    const uint format=1,
			    const uint16_t v_min = DEFAULT_DEPTH_MIN,
			    const uint16_t v_max = DEFAULT_DEPTH_MAX);


//!
//! Copy one frame data from source to destination buffer (both preallocated).
//! @param pSrc    Pointer to the head of source buffer
//! @param pDst    Pointer to the head of destination buffer
//! @param width   Width of frame in pixel
//! @param height  Height of frame in pixel
//! @param BPP     Byte Par Pixel. (Size of one pixel in byte.)
//! @param offset  Destination buffer offset in byte. This size from the head
//!   of destination buffer is skipped.
//! @param padding Destination buffer padding. At the end of each pixel, this
//!   size of buffer is skipped.
//!
void copyFrame(const void* pSrc, void* pDst,
	       const uint width, const uint height,
	       const uint BPP, const int offset=0, const int padding=0);

class Frames {
  uint m_width, m_height, m_BPP, m_nFrames, m_currentFrame; 
  void *m_pBuffer;

public:
  Frames();
  ~Frames();

  void deallocate();
  void allocate(uint width=0, uint height=0, uint BPP=4, uint nFrames=0);

  uint getWidth();
  uint getHeight();
  uint getNumFrames();
  uint getFrameIndex();
  void setFrameIndex(int iFrame);
  void incrementFrameIndex();

  void* getFrame(int iFrame=-1);
  void copyFrameTo(void* pDst, int iFrame=-1, int offset=0, int padding=0);
};

class RGBDFrames {
  Frames m_depthFrames, m_colorFrames;
public:
  RGBDFrames();
  ~RGBDFrames();
  
  void deallocate();
  void allocate(uint depthW, uint depthH, uint colorW, uint colorH, uint nFrames);

  uint getNumFrames();
  void incrementFrameIndex();
  uint getFrameIndex();
  void setFrameIndex(int iFrame);
  
  uint8_t* getColorFrame(int iFrame=-1);
  uint16_t* getDepthFrame(int iFrame=-1);
  
  void copyDepthFrameTo(uint16_t* pDst, int iFrame=-1, uint offset=0, uint padding=0);
  void copyColorFrameTo(uint8_t* pDst, int iFrame=-1, uint offset=0, uint padding=0);
  void convert16BitFrameToJet(uint8_t* pDst, int iFrame,
			      const uint16_t v_min, const uint16_t v_max,
			      const uint color_format=1);
};


#endif
