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

  * **Resolver:** The `AllOpsResolver` class provides the operations required for inference on microcontrollers and achieves this by providing resolver objects/instances. Usually, all the available operations are pulled in but this is not recommended as this can cause wastage of memory.
```
// This pulls in all the operation implementations we need.
  static tflite::ops::micro::AllOpsResolver resolver;
```

  * **Interpreter:** The interpreter takes in the model and resolver and some other related parameters and allocates memory for the model's input and outpur tensors (called _tensor arena._). After allocation, it is used to access these tensors using pointers. 
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
  * **Invoke:** Although a member of the Interpreter object, it performs inference on the input data.

```
TfLiteStatus invokeStatus = tflInterpreter->Invoke();
if (invokeStatus != kTfLiteOk) 
    Serial.println("Invoke failed!");
```

To avoid hassle of dependency mismanagement, I have kept the new examples with the old ones. These common files are imported _as-it-is_ in the examples, unless stated. The examples are elaborated as follows:

1) **Activity Tracker**: Smart Wristband-based detection

Keywords: _Arduino NANO, Accelerometer, gyroscope, BLE, TensorFlow_

The example is based on the fact that the NANO board is fit for the smart wristband applications, owing to its size, power and computation requirements. Another favourable feature is the _BLE_. It has almost everything needed to develop one such application. The application tracks 3 activities: _walking, running and resting_. These 3 classes are determined at the output of the model inference. This is used to track the duration of each activity. So it becomes clear if the person has been jogging since the last hour. This data is shared to the user's smartphone connected to the device via _BLE_. This further creates possibilities on the smartphone side. The model is trained on data collected through on-board IMU (Inertial Measurement Unit): accelerometer and gyroscope. This saves us from the need of calibration before performing inference. The process is: 

`Gather Training Data via IMU -> Classify data among the 3 classes -> Train Model using TensorFlow -> Use the TF_Lite exported as byte array -> Perform inference on real-time (processed) data`

2) **Sound-based diarization/Classification**: Profiling of sources using Supervised Learning. 

Keywords: _Arduino NANO, Microphone(MP34DT05), TensorFlow_

As per the [GoogleAI blog](https://ai.googleblog.com/2018/11/accurate-online-speaker-diarization.html), diarization refers to the process of classifying partitions of audio stream with multiple sources/speakers, into different segments associated with each individual source. I propose to develop a related application; instead of breaking down the audio clip, we will be analysing it and displaying the class to which the source(s) are assigned. Just like other examples, we leverage CNN-based Supervised Learning to train a TensorFlow (Lite) model trained on spectrograms of audios generated from different sources like: a human talking, drilling machine at work, nail getting hammered, etc. The model+NANO assembly will be responsible for distinguishing between these sounds when subjected to such an environment, generally an industrial setup. It can be used to create a smart sensor that warns when there is too much of sound generation or rather pollution. It can also determine the number of equipments in work and can warn if there a set of jobs (like humans chatting around when a heavy machine is at work) may be risky for the workers. 

The unique part in this example is the part where the feature is generated from the audio, using _Fast Fourier Transform_ (FFT) and converting it into an array. This part is done by the `GenerateMicroFeatures()` pre-defined in `micro_features/micro_features_generator.h`. The part that needs deciding, remains the sample size and the sliding window size. Heuristically, a 30ms window is recommended by the authors of the TinyML book. The values obtained in this window is stored as an array using the FFT. The window is shifted by 20ms resulting in a 10ms overlap. Further optimization to reduce data is achieved by averaging these values to reduce the effect of overlapping values.

3) **Anomaly Detection**: An industrial application

Keywords: _Arduino NANO, Accelerometer, thermometer, gyroscope, TensorFlow_

The example demonstrates another fine and real-life solution to real problem in industries. The subject has been demonstrated several times and my application is based on the one given by [RealityAI](https://reality.ai/edge-ai-cortex-m7-and-m4/). The model is trained on motion, vibration and temperature data, obtained by placing the board on a motor, which is itself mounted on another semi-fixed plank/platform. This is done to emulate an industrial environment where the motor is the equipment being monitored. The model learns to classify the working condition (through analysing the data obtained at that instant) as: _Safe_ or _Unsafe_ for health of the machine/equipment. _Unsafe_ conditions are created by disturbing the motor-board assembly via another motor behind the semi-fixed plank. We demonstrate using only IMU and thermometer data but the example can be used to derive other application that possess other environmental parameters like Pressure. The methodology is:

`Gather Training Data via IMU and HTS221 -> Classify data among the 2 classes -> Train Model using TensorFlow -> Use the TF_Lite exported as byte array -> Perform inference on real-time (processed) data`

4) **On-Device Training (for Function Approximation)**: Simple Regression based application

Keywords: _Board-Independent, TensorFlow_

This application is simplest yet quite novel and doesn't require any extra sensor module(s). Neural Networks have been proved to be universal function approximators by [Cybenko](https://en.wikipedia.org/wiki/Universal_approximation_theorem) and Hornik et al. but with _varying degree of approximation_. I have implemented some random wiggly continuous functions on the Arduino UNO in bare C and plotted using C++ tools. The interesting part is that this was done using a shallow neural network using a single hidden layer with `< 10` hidden neurons. The related work can be found in my [repository](https://github.com/PrashantDandriyal/Neural-Networks-In-Cpp/tree/master/Wiggly_Functions). The training was done on the UNO board. The `.ino` file can be found in the same repository or [here](https://github.com/PrashantDandriyal/Neural-Networks-In-Cpp/blob/master/arduino_NNv2.0.ino). The results were discussed in a paper that I have drafted but is yet to be submitted. The result can be understood with the following plot.

![Results of Summation function](...)

I plan to port this function approximator to the NANO boards. I will be training two networks: a shallow simple Neural Network and a CNN-based network. The former will be slighly slow and not as accurate as the CNN one. But this is lesson we learn. The CNN part is implemented using TensorFlow, using the workflow similar to the other examples. The example explains such basic differences in a trivial manner. The on-device training (which takes about 100-200 epochs than the conventional 4GB RAM-based computer systems) is another star to this example. It demonstrates how and why the precision in such microcontrollers drops and how it affects the performance. 

5) Digital Mouse using NANO: To be done after advice from mentors and deadline permitted

Keywords: _Arduino NANO, IMU(LSM9DS1), Library: Mouse, ..._

The example is to be developed in the time saved during the three phases. Hence, it is subject to the conditions at that time and can will only be done if my mentor(s) give it a green signal. It is another simple example that attempts to demonstrate another application of the IMU. There are several related implementations floating around the internet. Hence, it cannot be considered as a novel concept. Much of the interface is handled by the `mouse` library. It provides simple interfaces to convert the acceleration data to mouse-motion. Using this, we only use the acceleration values in the X & Y axes. The complete workflow is as:

`Obtain acceleration in X & Y direction -> Calibrate the values -> Use Mouse.move() to move the mouse pointer`

### Demonstration Project:

To shed more light on the project and the proposed methodology, I have created a prototype project found on the GitHub with name [Activity Tracker](...). The entire worflow has been shown. The model has been trained on the [MotionSense](https://github.com/mmalekzadeh/motion-sense/tree/master/data) dataset, using Google Collaboratory. The process can be seen in the iPython Notebook I saved in the project repository. The project can be found [here](...). The trained model is then converted to TensorFlow Lite format and finally to a byte array, in the form of a header file. The next part of performing inference on the actual device has been shown in the corresponding `.ino` file of the project. The model has been tested and the performance can be seen at the end of the `ipynb` file. Due to inavailability of the NANO board, the complete assembly could not be tested in real-time, nor could the actual IMU data be obtained, but the dataset does well to emulate the project.

_Note: Another duplicate of the project is placed in the examples section of TensorFlow, to avoid the dependencies issues. This can be found at my [forked version](...) of TensorFlow_ 


## Schedule of Deliverables

_Here should come a list of your milestones. This list is a start based on the difference phases of GSoC. Use it as a start. You can/should add more details for each phase by breaking it down into weeks or set specific targets for each
phase. Each target should be split into sub task with a time estimate, [work
breakdown structures](https://en.wikipedia.org/wiki/Work_breakdown_structure) are helpful here._

___________________

- first example(Activity Tracker): 
-- Obtain training data using the on-board IMU
-- Train model using TensorFlow and convert it into TF Lite model
-- Interface the model with Arduino code and test assembly on real-time data

- Second example(Activity Tracker): 
-- Obtain training data using the on-board IMU
-- Train model using TensorFlow and convert it into TF Lite model
-- Interface the model with Arduino code and test assembly on real-time data

- Test Mergeability of code with the actual (Arduino_TensorFlowLite) library. 
-- Sort related issues
-- 
___________________
### **Community Bonding Period**

_What will you do during the community bonding period?_
- Write Examples 1,2, 
* test Mergeability of code with the actual livrary
* Share on project hub

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

>_Do you have code on GitHub? Can you show previous contributions to other projects? 

I have been a consistent contributor to the open-source community, the submissions to which can be found in contributions section of my [GitHub](https://github.com/PrashantDandriyal) profile. To state them in chronological order:

* TensorFlow: The Pull Request was related to issue #35072 opened by Pete Warden (co-founder of TensorFlow Lite and TinyML). It was simply an explanation to "How int8 input and output quantization conversion works in TensorFlow Lite". The Pull has been approved and is ready to pull.

* Speech-VINO (Intel's OpenVINO toolkit): The page is meant for contributions to Intel's OpenVINO-toolkit-based projects. These projects incoporate sound applications. I have shared my project (in process of the "Introduction to Intel's OpenVINO toolkit" course, in which I am a scholar)

* Arduino: My first contribution to Arduino has been the modificatios to its "Ciao" [library](https://github.com/arduino-libraries/Ciao). I have updated the `readme.md` in official format. The Pull Request has been reviewed but merging is not yet done as the responsible maintainers have not gone through it. 

* Miscellaneous: Other Open-Source contributions invlude the [OpenVINO](https://github.com/alihussainia/OpenDevLibrary) project. I have been an active contributor and now, a maintainer to the project. The project aims at creating a hassle-free portable version of the Intel OpenVINO (Open Visual inference and Neural Optimization) toolkit. Till now, the project has had successful runs and no issues.

>_Did you do other code related projects or university courses? Do you have experience with Arduino?_

I have 

## Other Experiences

... My team's participation in IICDC, 2018 was the first step towards ML. I tried learning more about it by connecting to some of the prominent members of the community (like Evgeni Gousev..., the co-founder of TinyML) and followed their guest lectures delivered at Harvard. 
I completed the _Introduction to TensorFlow_ course on Udacity and Andrew Ng's course on Coursera and participated in two of the ML competition on HackerEarth. 


## Why this project?

>_Why you want to do this project?_

The aim for the selection of this project is based on my interests and experience. 
I have been learning and involved with Machine Learning for more than eight months now. Past that I was involved with Embedded Systems (as described above). As I was learning ML, I had a peculiar urge to find a bridge between both my interests. I stumbled upon the [TinyML](...)group on LinkedIn and knew it was the exact thing I was looking for. Since then, I have been diving deeper into the field of **EdgeAI** and **TinyML**. I have been planning to submit an official library to Arduino for implementing simple (shallow) Neural Networks on classic Arduino boards like the Uno. But then, GSoC seemed to coincide with it and I took it as an opportunity to continue my initiative but with mentors! The projects I wished to take on were the _Writing Examples for Official Libraries_ and _Examples and Tools for Portenta Board_. I had some clarifications with the mentor on GitHub and he suggested that I go for this project instead of porting TensorFlow Lite for the Portenta, which I proposed initially.

I believe that the completion of this project will be a contribution to the TinyML and open source Arduino and TensorFlow communities. Both Arduino as well as TensorFlow have been the first choice for beginners of Embedded systems and Machine Learning. The project has the potential of strengthening that tie. The excellent Arduino_TensorFlowLite library deserves more support. This project will encourage other developers to realize the capabilities of the Arduino Boards and port it for other classic boards. 


## Do you have any other commitments during the GSoC period?

>_Provide dates, such as holidays, when you will not be available._

I will be unable on the some of the national gazetted holidays that are mentioned below. I also have my last semester exams on ...

Holiday | Date
---|---
Raksha Bandhan | 
Semester Exams | 

 

