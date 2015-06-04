#ifndef DEPTH_REASONING_COMMON_DEFS_HPP
#define DEPTH_REASONING_COMMON_DEFS_HPP

#include <chrono>
#include <string>
#include <stdexcept>

typedef unsigned int uint;

//!
//! Get the current timestamp in second up to microsecond order.
std::chrono::microseconds getCurrentTimestamp();


//!
//! Runtime Error class for facilitating the dynamic error message generation.
class RuntimeError : public std::exception {
  std::string message_;

  void concat(std::string message);
  void concat(const char* message);
  //!
  //! Concatenate numerical value to error message
  template <typename T> void concat(T arg) {
    message_ += std::to_string(arg);
  };
  //! Concatenate variable number of inputs to error message.
  template <typename T, typename... Args> void concat(T arg, Args... args) {
    concat(arg); concat(args...);
  }
public:
  //!
  //! Construct error message from arbitrary number of strings and numbers.
  template <typename... Args> RuntimeError(Args... args) {
    concat(args...);
  };

  virtual const char* what() const throw();
};


//!
//!
class VideoFrame {
  uint8_t* pFrame_;
  uint width_, height_, BPP_;
  std::chrono::microseconds timestamp_;
public:
  VideoFrame(uint width=0, uint height=0, uint BPP=0);
  ~VideoFrame();

  void setSize(uint width, uint height, uint BPP);
  void allocate();
  void free();

  void copyDataFrom(void* pSrcBuff, int width=-1, int height=-1, int BPP=-1);
  void setTimestamp(std::chrono::microseconds timestamp);
};

#endif // #ifndef DEPTH_REASONING_COMMON_DEFS_HPP
