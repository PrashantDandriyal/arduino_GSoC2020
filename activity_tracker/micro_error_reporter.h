#ifndef TENSORFLOW_LITE_MICRO_MICRO_ERROR_REPORTER_H_
#define TENSORFLOW_LITE_MICRO_MICRO_ERROR_REPORTER_H_

#include "tensorflow/lite/core/api/error_reporter.h"
#include "tensorflow/lite/micro/compatibility.h"
#include "tensorflow/lite/micro/debug_log.h"
#include "tensorflow/lite/micro/debug_log_numbers.h"

namespace tflite {

class MicroErrorReporter : public ErrorReporter {
 public:
  ~MicroErrorReporter() {}
  int Report(const char* format, va_list args) override;

 private:
  TF_LITE_REMOVE_VIRTUAL_DELETE
};

}  // namespace tflite

#endif  // TENSORFLOW_LITE_MICRO_MICRO_ERROR_REPORTER_H_
