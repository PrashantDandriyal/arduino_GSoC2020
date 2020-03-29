# Cross check that all the code snippets are from your proto code
#Submit PR

## Abstract

_Short description of your project. Max 10 sentences. This **SHOULD NOT** be a
copy of the project idea text._

## Technical Details

_Long description of the project. **Must** include all technical details of the
projects, like libraries involved.Here you can show relevant pieces of code that you want to change. You can
link to literature you used during the research._

The project aims at developing examples for the official Arduino library: __Arduino_TensorFlowLite__. The library has implicitly been associated with the _Arduino Nano 33 BLE Sense_ board with most efforts from Pete Warden, Daniel Situnayake, Don Coleman, Sandeep Mistry, Dominic Pajak. All the developers have done an excellent job by providing three finely detailed examples like __hello_world, magic_wand, Gesture to emoji, person_detection and micro_speech__. The board is one of the most prominent work-horses for _TinyML_ that operate under the 1W# range. These examples have successfully demonstrated the utility and capabilities of the Nano. They use the ported [version](https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/micro/examples/micro_speech) of TensorFlowLite and make use of on-board sensors like the digital microphone (MP34DT05), Gesture Sensor for color, ambiance illumination (APDS9960) and IMU (LSM9DS1). The examples provide a clear interface method between the Machine Learning software stack and the hardware sensors. This is exactly what a newcomer seeks; a developer finds it welcoming. I plan to extend this legacy by following the same plan but with newer examples. I will be developing five examples for the Nano (classic) and the Nano BLE Sense board. The examples cover most of the sensors to widen the possible uses of the boards, with a machine learning support in four of them. Almost all the examples are based on the previous (four) examples developed by TensorFlow. [Pete Warden](https://petewarden.com/) and Daniel Situnayake have explained the _TensorFlow Lite_ part in their [book](https://books.google.com/books/about/TinyML.html?id=sB3mxQEACAAJ&source=kp_book_description) on TinyML. They have termed these as experimental and mainly all the tests have been done on the _SparkFun_ Board. In one of the instances, they say: 
>Because the default numbers were calibrated for the SparkFun Edge, the Arduino implementation needs its own set of numbers.

The development setup is summarised as:

-|-
Target Platform(s) | Arduino NANO, Arduino NANO 33 BLE Sense
IDE | Arduino Web IDE, Arduino Desktop IDE
Libraries | TensorFlow-Arduino Interface (Arduino_TensorFlowLite, experimental headers, gesture_predictor, etc), Sensor interface libraries (ArduinoLSM9DS1, ArduinoSound, ArduinoAPDS9960, ArduinoLPS22HB, ArduinoHTS221), Pre-installed Board Libraries (Arduino.h), Miscellaneous

The examples have various common functionalities (like the `micro_error_reporter` for logging debug information) mostly because all of them use the same __Arduino_TensorFlowLite__ library. Hence they share certain dependencies with the pre-existing examples. A common import process seen in all the examples is: 

```
#include <TensorFlowLite.h>
#include <tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h>
#include <tensorflow/lite/experimental/micro/micro_error_reporter.h>
#include <tensorflow/lite/experimental/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>
```

The workflow I will be using is broken into two parts: **1) Model Training** (using TensorFlow Lite API) and **2) Inference** (on Arduino Platform). They are detailed as:
 1) **Model Training:** architecture, training, data. The model is converted to a header file which is then imported into the _main_ module. It is handled by the 

 2) **Inference:** The steps involved are broadly contained in the `Main()` module. Each step is implemented through some method provided by the common libraries mentioned above. The steps are:
  * Model Handler: The role of this object is to create a handle that allows us to use the model which is basically binary data stored in a byte array which is itself packed in a header file after the training process. It also provides the model description like the schema version. 

```
if (tflModel->version() != TFLITE_SCHEMA_VERSION) 
{
    Serial.println("Model schema mismatch!");
    while (1); //Stop indefinitely
}
``` 

  * Resolver: The `AllOpsResolver` class provides the operations required for inference on microcontrollers and achieves this by providing resolver objects/instances. Usually, all the available operations are pulled in but this is not recommended as this can cause wastage of memory.
```
// This pulls in all the operation implementations we need.
  static tflite::ops::micro::AllOpsResolver resolver;
```

  * Interpreter: The interpreter takes in the model and resolver and some other related parameters and allocates memory for the model's input and outpur tensors (called _tensor arena._). After allocation, it is used to access these tensors using pointers. 
```
// Create an interpreter
  interpreter = new tflite::MicroInterpreter(model, opsResolver, tensorArena, tensorArenaSize, &errorReporter);

// Allocate memory for the model's input and output tensors
interpreter->AllocateTensors();

// Get pointers for the model's input and output tensors
inputTensor = interpreter->input(0);
outputTensor = interpreter->output(0);
```
The above three routines/modules are placed in the `setup()` sub-routine. The following run indefinitely in the `loop()` sub-routine, in compliance with Arduino's workflow.
  
  Once the data collected is ready for feeding to the model, i.e., after the required number of samples have been successfully collected and if needed, pre-processed (like normalisation, upsampling/downsampling, etc), the input data is forward propagated and actual inference is made using the follwing modules:
  * Invoke: Although a member of the Interpreter object, it performs inference on the input data.

```
TfLiteStatus invokeStatus = tflInterpreter->Invoke();
if (invokeStatus != kTfLiteOk) 
    Serial.println("Invoke failed!");
```

To avoid hassle of dependency mismanagement, I have kept the new examples with the old ones. These common files are imported _as-it-is_ in the examples, unless stated. The examples are elaborated as follows:

1) **Activity Tracker**: Smart Wristband-based detection

Keywords: _Accelerometer, gyroscope, BLE, TensorFlow_
The example is based on the fact that the NANO board is fit for the smart wristband applications, owing to its size, power and computation requirements. Another favourable feature is the _BLE_. It has almost everything needed to develop one such application. The application tracks 3 activities: _walking, running and resting_. These 3 classes are determined at the output of the model inference. This is used to track the duration of each activity. So it becomes clear if the person has been jogging since the last hour. This data is shared to the user's smartphone connected to the device via _BLE_. This further creates possibilities on the smartphone side. The model is trained on data collected through on-board IMU (Inertial Measurement Unit): accelerometer and gyroscope. This saves us from the need of calibration before performing inference. The process is: `Gather Training Data via IMU`->`Classify data among the 3 classes`->`Train Model using TensorFlow`->`Use the TF_Lite exported as byte array`->`Perform inference on real-time (processed) data`

2) **Hand Gesture Detection**: Sound-based-diarization, federated-learning, 

3) **Anomaly Detection**: An industrial application

Keywords: _Accelerometer, thermometer, gyroscope, TensorFlow_

The example demonstrates another fine and real-life solution to real problem in industries. The subject has been demonstrated several times and my application is based on the one given by [RealityAI](https://reality.ai/edge-ai-cortex-m7-and-m4/). The model is trained on motion, vibration and temperature data, obtained by placing the board on a motor, which is itself mounted on another semi-fixed plank/platform. This is done to emulate an industrial environment where the motor is the equipment being monitored. The model learns to classify the working condition (through analysing the data obtained at that instant) as: _Safe_ or _Unsafe_ for health of the machine/equipment. _Unsafe_ conditions are created by disturbing the motor-board assembly via another motor behind the semi-fixed plank. We demonstrate using only IMU and thermometer data but the example can be used to derive other application that possess other environmental parameters like Pressure. The methodology is `Gather Training Data via IMU and HTS221`->`Classify data among the 2 classes`->`Train Model using TensorFlow`->`Use the TF_Lite exported as byte array`->`Perform inference on real-time (processed) data`

4) **Object Counting**: An industrial application
5) **Function Approximation** or Mouse




## Schedule of Deliverables

_Here should come a list of your milestones. This list is a start based on the
difference phases of GSoC. Use it as a start. You can/should add more details
for each phase by breaking it down into weeks or set specific targets for each
phase. Each target should be split into sub task with a time estimate, [work
breakdown structures](https://en.wikipedia.org/wiki/Work_breakdown_structure) are helpful here._

### **Community Bonding Period**

_What will you do during the community bonding period?_
- Write Examples 1,2, 
* test Mergeability of code with the actual livrary
*  

### **Phase 1**

* Deliverable 1
* Deliverable 2
* ...

### **Phase 2**

* Deliverable 1
* Deliverable 2
* ...

### **Final Week**

_At this stage you should finish up your project. At this stage you should make
sure that you have code submitted to your organization. Our criteria to mark
your project as a success is to submit code before the end of GSoC._

## Development Experience

_Do you have code on GitHub? Can you show previous contributions to other projects?
Did you do other code related projects or university courses?_

_Do you have experience with Arduino?_

## Other Experiences

...


## Why this project?

_Why you want to do this project?_

## Do you have any other commitments during the GSoC period?

_Provide dates, such as holidays, when you will not be available._

