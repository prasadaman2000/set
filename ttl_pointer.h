#ifndef TTL_POINTER_H
#define TTL_POINTER_H

#include <iostream>
#include <memory>

#include "absl/time/clock.h"
#include "absl/time/time.h"

template <typename T>
class ttl_pointer {
 public:
  ~ttl_pointer() {};
  T& operator*();
  T* get();
  bool has_value();
  void refresh();

  static ttl_pointer Create(T val, int64_t timeout_ms);

 private:
  ttl_pointer(T val, int64_t timeout_ms);
  std::unique_ptr<T> ptr_;
  absl::Time expiry_;
  int64_t timeout_ms_;
};

template <typename T>
ttl_pointer<T> ttl_pointer<T>::Create(T val, int64_t timeout_ms) {
  return ttl_pointer(val, timeout_ms);
}

template <typename T>
ttl_pointer<T>::ttl_pointer(T val, int64_t timeout_ms)
    : timeout_ms_(timeout_ms) {
  ptr_ = std::make_unique<T>(val);
  expiry_ = absl::Now() + absl::Milliseconds(timeout_ms);
}

template <typename T>
T& ttl_pointer<T>::operator*() {
  return *ptr_;
}

template <typename T>
T* ttl_pointer<T>::get() {
  if (ptr_ != nullptr && absl::Now() < expiry_) {
    return ptr_.get();
  }
  return nullptr;
}

template <typename T>
bool ttl_pointer<T>::has_value() {
  // absl::Duration time_to_expiry = expiry_ - absl::Now();
  // std::cout << time_to_expiry << " left till expiry\n";
  return ptr_ != nullptr && absl::Now() < expiry_;
}

template <typename T>
void ttl_pointer<T>::refresh() {
  expiry_ = absl::Now() + absl::Milliseconds(timeout_ms_);
}

#endif
