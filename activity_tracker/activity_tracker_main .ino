/*
 activity_tracker: Demo project for GSoC 2020 (Arduino)
 - The file has not been tested on an actual Arduino Platform but suits the purpose of demonstrating the methodology. 
 - There may be logical issues which can be corrected at the time of implementation on actual hardware, and hence, can be neglected right now.

 Author: Prashant Dandriyal
 Date: 30 March, 2020
 
 References: example provided in the book: TinyML by Pete Warden
*/

#include <Arduino_LSM9DS1.h>	// For IMU
#include <TensorFlowLite.h>

#include "model.h" // Importing the model trained using TF Lite
#include "tensorflow/lite/experimental/micro/kernels/micro_ops.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/experimental/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#include "model.h"	// Model exported from iPython notebook

const float accelerationThreshold = 2.5; // threshold of significant in G's
const int numSamples = 50; // The format of data fed to the model is: (numFeature, 50 samples), where numFeatures in our case is 6: ax, ay, az, gx, gy, gz

// Globals, used for compatibility with Arduino-style sketches.
namespace {
  //Error Reporter
tflite::ErrorReporter* error_reporter = nullptr;
// Model handler
const tflite::Model* tf_model = nullptr;
// Interpreter handler
tflite::MicroInterpreter* interpreter = nullptr;
// Tensor handler
TfLiteTensor* model_input = nullptr;
TfLiteTensor* model_output = nullptr;

int input_length = numSamples;	//To keep a count of samples taken
int num_classes = 4; 		// The model has been trained on 4 classes: [walk, sit, jog, stand (but in order)

// Create an area of memory to use for input, output, and intermediate arrays.
// The size of this will depend on the model we're using
constexpr int kTensorArenaSize = 60 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

// Whether we should clear the buffer next time we fetch data
bool should_clear_buffer = false;
}  


void setup() 
{
  // Set up logging
  Serial.begin(9600);
  static tflite::MicroErrorReporter micro_error_reporter;  
  error_reporter = &micro_error_reporter;
  
  // Intialise the IMU
  if (!IMU.begin()) 
  {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // Map the model into a usable data structure.
  tf_model = tflite::GetModel(model);

  if (tf_model->version() != TFLITE_SCHEMA_VERSION) 
  {
    error_reporter->Report(
        "Model provided is schema version %d not equal "
        "to supported version %d.",
        tf_model->version(), TFLITE_SCHEMA_VERSION);
    return;	// Exit
  }

  // Add all operations needed for forward propagation in this model
  // Note: It varies from model to model. Generally, all operations
  // are imported but this is not recommended as it overloads the memory
  static tflite::ops::micro::AllOpsResolver op_resolver;
 

  // Build an interpreter to run the model with
  static tflite::MicroInterpreter static_interpreter(
      tf_model, op_resolver, tensor_arena, kTensorArenaSize,
      error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors
  interpreter->AllocateTensors();

  // Obtain pointer to the model's input tensor
  model_input = interpreter->input(0);
  model_output = interpreter->output(0);
  ///input_length = model_input->bytes / sizeof(float);

  TfLiteStatus setup_status = SetupAccelerometer(error_reporter);
  if (setup_status != kTfLiteOk) 
  {
    error_reporter->Report("Set up failed\n");
  }
}

void loop() 
{
  float ax, ay, az, gx, gy, gz;

  // Reset Count 
  samplesRead = 0;

  // Attempt to read new data from the accelerometer
  while(input_length < numSamples)
  {
      if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      // read the acceleration and gyroscope data
      IMU.readAcceleration(ax, ay, az);
      IMU.readGyroscope(gx, gy, gz);
	
       // Prepare data by normalizing/standardising
	// Use the same method as done while training
	model_input->data.f[samplesRead *6 +0] = (ax+7.5)/15;
	model_input->data.f[samplesRead *6 +1] = (ay+7.5)/15;
	model_input->data.f[samplesRead *6 +2] = (az+7.5)/15;
	model_input->data.f[samplesRead *6 +3] = (gx+4)/8;
	model_input->data.f[samplesRead *6 +4] = (gy+4)/8;
	model_input->data.f[samplesRead *6 +5] = (gz+4)/8;
	
	// Update count
	samplesRead++;
  }

  // Data Collected. Perform Inference
  TfLiteStatus invoke_status = interpreter->Invoke();

  if(invoke_status != kTfLiteOk)
  {
	Serial.println("Inference Failed");
	error_reporter->Report("Invoke failed on index: %d\n", begin_index);
	return;
  }
  else	// Print results
  {
	for(int i=0; i<num_classes; ++i)
	{
	    // Print probablities of all classes
	    Serial.println(model_output->data.f[i]); 
	} 
  }
}

