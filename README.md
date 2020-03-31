# GSoC 2020 | Writing Examples for Official Libraries

## A Project Proposal submitted to Arduino

**Name:** Prashant Dandriyal

**GitHub:** [PrashantDandriyal](https://github.com/PrashantDandriyal)

**Email:** prashantdandriyal7@gmail.com

## Abstract

>Short description of your project

The project proposes to develop examples for the **Arduino_TensorFlowLite** library. The library is supported by several Arduino Boards like the NANO (Classical), NANO BLE Sense and the UNO. All the examples involving the library are demonstrated for the NANO Sense board, but can be deployed on the classical NANO but with externally connected sensors. The examples aim to showcase the practical utility of the on-board sensors of the NANO Sense board and the ease with which almost any lightweight TensorFlow model can be interfaced with it. The examples are explained into two sections: the common **Model Training** part and the device-sensor dependent **Inference** part. We cover all the sensors present and demonstrate the industrial applications. A novel and niche implementation of "On-device training" is also proposed to revisit the need and aspects of TinyML or EmbeddedAI.

## Technical Details

>Long description of the project. 

The **Arduino_TensorFlowLite** library has implicitly been associated with the _Arduino Nano 33 BLE Sense_ board with most efforts from Pete Warden, Daniel Situnayake, Don Coleman, Sandeep Mistry and Dominic Pajak. All the developers have done an excellent job by providing three finely detailed examples like _hello_world, magic_wand, Gesture to emoji, person_detection and micro_speech_. The board is one of the most prominent work-horses for _TinyML_ that operate under the 1 mW range. These examples have successfully demonstrated the utility and capabilities of the Nano. They use the ported [version](https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/micro/examples/micro_speech) of TensorFlowLite and make use of on-board sensors like the digital microphone (MP34DT05), Gesture Sensor for color, ambiance illumination (APDS9960) and IMU (LSM9DS1). The examples provide a clear interface method between the Machine Learning software stack and the hardware sensors. This is exactly what a newcomer seeks; a developer finds it welcoming. I plan to extend this legacy by following the same plan but with newer examples. I will be developing **five examples** for the Nano (classic) and the Nano BLE Sense board. [Pete Warden](https://petewarden.com/) and Daniel Situnayake have explained the _TensorFlow Lite_ part in their [book](https://books.google.com/books/about/TinyML.html?id=sB3mxQEACAAJ&source=kp_book_description) on TinyML. They have termed these as experimental and mainly all the tests have been done on the _SparkFun_ Board. In one of the instances, they say: 
>Because the default numbers were calibrated for the SparkFun Edge, the Arduino implementation needs its own set of numbers.

I have taken only a single library owing to its importance and complications that newcomers face when using Machine Learning Models even on local systems. The deployment of these model onto embedded systems is another daunting task to accomplish. I propose to simplify that process by covering all major areas of EdgeAI that the previous contributors may have missed.

The development setup is summarised as:

Category | Tools
--|--
Target Platform(s) | Arduino NANO, Arduino NANO 33 BLE Sense
IDE | Arduino Web IDE, Arduino Desktop IDE
Libraries | **TensorFlow-Arduino Interface** (Arduino_TensorFlowLite, experimental headers, gesture_predictor, etc), **Sensor interface libraries** (ArduinoLSM9DS1, ArduinoSound, ArduinoAPDS9960, ArduinoLPS22HB, ArduinoHTS221), **Pre-installed Board Libraries** (Arduino.h), Miscellaneous

The examples have various common functionalities (like the `micro_error_reporter` for logging debug information) mostly because all of them use the same "Arduino_TensorFlowLite" library. Hence they share certain dependencies with the pre-existing examples. A common import process seen in all the examples is: 

```
#include <TensorFlowLite.h>
#include <tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h>
#include <tensorflow/lite/experimental/micro/micro_error_reporter.h>
#include <tensorflow/lite/experimental/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>
```

The workflow I will be using is broken into two parts: **1) Model Training** (using TensorFlow Lite API) and **2) Inference** (on Arduino Platform). They are detailed as:

 **1) Model Training:** TensorFlow 2.0 and TensorFlow Lite APIs are used to train the model using conventional methods. One of the models created in the (later discussed) demo example is created as:
 ```
 import tensorflow as tf

 model = tf.keras.Sequential()
 model.add(tf.keras.layers.Dense(30, activation='relu', dtype='float32')) 
 model.add(tf.keras.layers.Dense(15, activation='relu', dtype='float32'))
 model.add(tf.keras.layers.Dense(4, activation='softmax',     dtype='float32')) # 4 Classes in output 

 ```

 The Trained model is then converted to TensorFlow Lite model. 
 > Note: The data must be of float32 type 

 The model is converted to a header file which is then imported into the _main_ module. It is handled by the "Arduino_TensorFlowLite" library.

 **2) Inference:** The steps involved are broadly contained in the    `Main()` module. Each step is implemented through some method provided by the common libraries mentioned above. The steps are:
 * **Model Handler:** The role of this object is to create a handle that allows us to use the model which is basically binary data stored in a byte array which is itself packed in a header file after the training process. It also provides the model description like the schema version. 

   ```
   if (tf_model->version() != TFLITE_SCHEMA_VERSION) 
   {
     error_reporter->Report(
        "Model provided is schema version %d not equal "
        "to supported version %d.",
        tf_model->version(), TFLITE_SCHEMA_VERSION);
     return;	// Exit
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

____


### Examples

**1) On-Device Training (for Function Approximation)**: Demonstration of training on microcontrollers

_Keywords: Board-Independent, Library Independent_

This application is simplest yet quite novel and doesn't require any extra sensor module(s). Neural Networks have been proved to be universal function approximators by [Cybenko](https://en.wikipedia.org/wiki/Universal_approximation_theorem) and Hornik et al. but with _varying degree of approximation_. I have implemented some random wiggly continuous functions on the Arduino UNO in bare C and plotted using C++ tools. The interesting part is that this was done using a shallow neural network using a single hidden layer with `< 10` hidden neurons. The related work can be found in my [repository](https://github.com/PrashantDandriyal/Neural-Networks-In-Cpp/tree/master/Wiggly_Functions). The training was done on the UNO board. The `.ino` file can be found in the same repository or [here](https://github.com/PrashantDandriyal/Neural-Networks-In-Cpp/blob/master/arduino_NNv2.0.ino). The results were discussed in a paper that I have drafted but is yet to be submitted. The result can be understood with the following plot.

  ![Results of Summation function](https://github.com/PrashantDandriyal/arduino_GSoC2020/blob/master/DeviceTrain_results.png)

It can be clearly observed that model trained on Arduino UNO took 1090 epochs to compete with the accuracy of the one trained on a local system. I plan to port this function approximator to the NANO boards. I will be training a shallow simple Neural Network (approximating a simple summation function), implemented in C with only a single hidden layer. The data is generated using the color sensor. We generate different _R, G, B_ values by keeping different color objects before saving the RBG values. This forms the input vector of the network. The output vector is created by summing up these values (SUM = R+G+B). The model is created within the ino file and trained on this data. Tests are conducted on similar data from the color sensor in a similar fashion. The results can be compared with that of another deeper model. The former will be slighly slow and not as accurate as the deep one. But this is lesson we learn. The second part is implemented using TensorFlow, using the workflow similar to the other examples. The example explains such basic differences in a trivial manner. The on-device training (which takes about 100-200 epochs than the conventional 4GB RAM-based computer systems) is another star to this example. It demonstrates how and why the precision in such microcontrollers drops and how it affects the performance. Almost no microcontrollers demonstrate such a thing, but I would like to do so, which is nothing but an extended version of my previous work.


**2) Function Plotter:** Simple regression application based on _function approximation_

_Keywords: Arduino NANO, Library {Arduino_TensorFlowLite}_

The example is to be developed in continuation of the previous one, i.e., based on the concept of Function Approximation but implemented as a regression problem using TensorFlow. We train a model on a data generated after creating a random function like _f(p) = 0.2+0.4(p<sup>2</sup>) + 0.3(p Sin(15p) + 0.05 Cos(50p) .
![wiggly func](https://github.com/PrashantDandriyal/arduino_GSoC2020/blob/master/wiggly.png)

The model is trained on the data generated using this function i.e., sets of _(x, f(x))_. A 3 layer keras model is capable of approximating this function. The results can seen in the below plot, with the BLUE plot as the regressor line predicted by the model. The model is converted and asked to predict on similar real-time data generated within the board. The results are plotted using the terminal. The training part has been demonstrated in the Google Collab [notebook](https://github.com/PrashantDandriyal/arduino_GSoC2020/blob/master/Wiggly_function_approximation.ipynb) in the proposal repository. The workflow is as:

`
Create a function in using Python and generate training data -> Create a Keras model -> Train -> Obtain TF Lite model -> Obtain byte array model -> Use model to make prediction on similar data on device -> Plot results on terminal` 


**3) Anomaly Detection**: An industrial application

_Keywords: Arduino NANO, Accelerometer(LSM9DS1), thermometer(HTS221), gyroscope(LSM9DS1), TensorFlow, Library { ArduinoLSM9DS1, ArduinoHTS221, Arduino_TensorFlowLite}_

The example demonstrates another fine and real-life solution to real problem in industries. The subject has been demonstrated several times and my application is based on the one given by [RealityAI](https://reality.ai/edge-ai-cortex-m7-and-m4/). The model is trained on motion, vibration and temperature data, obtained by placing the board on a motor, which is itself mounted on another semi-fixed plank/platform. This is done to emulate an industrial environment where the motor is the equipment being monitored. The model learns to classify the working condition (through analysing the data obtained at that instant) as: _Safe_ or _Unsafe_ for health of the machine/equipment. _Unsafe_ conditions are created by disturbing the motor-board assembly via another motor behind the semi-fixed plank. We demonstrate using only IMU and thermometer data but the example can be used to derive other application that possess other environmental parameters like Pressure. The methodology is:

`Gather Training Data via IMU and HTS221 -> Manually label data into the 2 classes -> Process data -> Train Model using TensorFlow -> Use the TF_Lite model exported as byte array -> Perform inference on real-time (processed) data`

**4) Activity Tracker**: Smart Wristband-based detection

_Keywords: Arduino NANO, Accelerometer(LSM9DS1), gyroscope(LSM9DS1), BLE, TensorFlow, Library{ArduinoLSM9DS1, Arduino_TensorFlowLite}_

The example is based on the fact that the NANO board is fit for the smart wristband applications, owing to its size, power and computation requirements. Another favourable feature is the _BLE_. It has almost everything needed to develop one such application. The application tracks 4 activities: _walking, jogging standing and siting. These classes are determined at the output of the model inference. This is used to track the duration of each activity. So it becomes clear if the person has been jogging since the last hour. This data is shared to the user's smartphone connected to the device via _BLE_. This further creates possibilities on the smartphone side. The model is trained on data collected through on-board IMU (Inertial Measurement Unit): accelerometer and gyroscope. This saves us from the need of calibration before performing inference. The process is: 

`Gather Training Data via IMU -> Classify data into classes -> Train Model using TensorFlow -> Use the TF_Lite exported as byte array -> Perform inference on real-time (processed) data`

**5) Sound-based diarization/Classification**: Profiling of sources using Supervised Learning. 

_Keywords: Arduino NANO, Microphone(MP34DT05), TensorFlow, Library{ PDM, ArduinoSound, Arduino_TensorFlowLite}_

As per the [GoogleAI blog](https://ai.googleblog.com/2018/11/accurate-online-speaker-diarization.html), diarization refers to the process of classifying partitions of audio stream with multiple sources/speakers, into different segments associated with each individual source. I propose to develop a related application; instead of breaking down the audio clip, we will be analysing it and displaying the class to which the source(s) are assigned. Just like other examples, we leverage CNN-based Supervised Learning to train a TensorFlow (Lite) model trained on spectrograms of audios generated from different sources like: a human talking, drilling machine at work, nail getting hammered, etc. The model+NANO assembly will be responsible for distinguishing between these sounds when subjected to such an environment, generally an industrial setup. It can be used to create a smart sensor that warns when there is too much of sound generation or rather pollution. It can also determine the number of equipments in work and can warn if there a set of jobs (like humans chatting around when a heavy machine is at work) may be risky for the workers. 

The unique part in this example is the part where the feature is generated from the audio, using _Fast Fourier Transform_ (FFT) and converting it into an array. This part is done by the `GenerateMicroFeatures()` pre-defined in `micro_features/micro_features_generator.h`. The part that needs deciding, remains the sample size and the sliding window size. Heuristically, a 30ms window is recommended by the authors of the TinyML book. The values obtained in this window is stored as an array using the FFT. The window is shifted by 20ms resulting in a 10ms overlap. Further optimization to reduce data is achieved by averaging these values to reduce the effect of overlapping values. Workflow:

`Generate audio data using MP34DT05 -> Obtain spectrogram using FFT -> Process it (Perform Down Sampling/Upsampling and normalization/standardisation) -> Train TF model -> Obtain TF Lite model -> Obtain byte-array model -> Perform inference`


### Demonstration Project:

To shed more light on the project and the proposed methodology, I have created a prototype project found on the GitHub with name [Activity Tracker](https://github.com/PrashantDandriyal/arduino_GSoC2020/tree/master/activity_tracker). The entire worflow has been shown. The model has been trained on the [MotionSense](https://github.com/mmalekzadeh/motion-sense/tree/master/data) dataset, using Google Collaboratory. The process can be seen in the iPython Notebook I saved in the project repository. The notebook can be found [here](https://github.com/PrashantDandriyal/arduino_GSoC2020/blob/master/activity_tracker_motionSense_gsoc20.ipynb). The trained model is then converted to TensorFlow Lite format and finally to a byte array, in the form of a header file. The next part of performing inference on the actual device has been shown in the corresponding `.ino` file of the project. The model has been tested and the performance can be seen at the end of the `ipynb` file. Due to inavailability of the NANO board, the complete assembly could not be tested in real-time, nor could the actual IMU data be obtained, but the dataset does well to emulate the project.

_Note: Another duplicate of the project is placed in the examples section of TensorFlow, to avoid the dependencies issues. This can be found at my [forked version](https://github.com/PrashantDandriyal/tensorflow/tree/master/tensorflow/lite/micro/examples/activity_tracker) of TensorFlow_ 

____


## Schedule of Deliverables

>Here should come a list of your milestones

### **Community Bonding Period (May 4, 2020 - June 1, 2020)**
* Discuss approach and weekly targets for Phase 1 with mentors.
* Setup working environment for development.
* Study about the `Arduino_TensorFlowLite` and its backend on TensorFlow repository.
* Document the dependency network for better understanding.
* Obtain the required hardware tools.
* Decide mode of communication with mentors and setup channels.
 

### **Phase 1 (June 1, 2020 - July 3, 2020)**

* Deliverable 1 : **Example 1 (On-Device Training)**
  * Generate fake training data using the Summation function (`SUM =f(R,B,G)` ). (locally)
  * Train model on local system (locally)
  * Port training process into the `void setup()` and `void loop()` interface of Arduino.
  * Collect data on-device using the color sensor (APDS9960)
  * Perform training and display the performance on console.
  * Test model on real-time data
  * Compare performance and document it

* Deliverable 2 : **Example 2 (Function Plotter)**
  * Obtain fake training data using a random (wiggly) function using Python.
  * Create and Train a simple neural network using TensorFlow/Keras API.
  * Convert TF model -> TF Lite model -> `model.h`
  * Create the `.ino` file after importing the model file.
  * Generate real-time data (_x_) and use model to predict (_f(x)_).
  * Plot the predicted vs actual result.
  * Test the system, document issues.

### **Phase 2 (July 4, 2020 - July 27, 2020)**

* Deliverable 3 : **Example 3 (Anomaly Detection)**
  * Design and make the motor-plank assembly.
  * Use the on-board sensor mounted on the assembly to obtain training data. Generate different conditions using the assembly to procure adequate data for both the classes.
  * Process data (sample it and normalize/standardise).
  * Train model on data and obtain `model.h` file as done previously.
  * Create .ino file. Design data pipeline to be used in it. 
  * Add warning feature for `unsafe` class (when detected)
  * Test the system, document issues.

* Deliverable 4 : **Example 4 (Activity Tracker)**
  * Obtain training data using the IMU for 3-4 classes indicating motion/no-motion conditions.
  * Process data and train data using TensorFlow and Python.
  * Convert model to header format (as done before)
  * Deploy model to .ino file. Add data handling part to create similar data (as used for training). 
  * Test the system in real-time.
  * Add BLE feature by connecting to and sending data to another BLE-powered device (smartphone).
  * Test and debug. Document the observations.

### **Phase 3 (July 28, 2020 - August 23, 2020)**

* Deliverable 5 : **Example 5 (Sound-based diarization/Classification)**
  * Obtain training data using on-board microphone.
  * Process it (Downsample and average values) using FFT to get a numeric array.
  * Train and convert model to header format.
  * Import model into .ino file and create data intake pipeline for the audio data. Process it to suit the model.
  * Test the system in real-time
  * Debug errors and document.

* Deliverable 6: Project Publishing
  * Create project related to the examples and publish on the Arduino project Hub.
  * Test Mergeability of code with the actual (Arduino_TensorFlowLite) library. 
  * Provide tutorial to use the library.
  * Specify the future scope. 

### **Final Week (August 24, 2020 - August 31, 2020)**

* Deliverable 7: **Final Project Report** 
  * Discuss progress with mentors and seek suggestions on final report.
  * Get feedback on project. 
  * Make suggested edits.
  * Prepare final report and get it reviewed by mentors.

___


## Development Experience

> Do you have code on GitHub? Can you show previous contributions to other projects? 

I could not find a official repository for the library but have studied it well. Hence, I could not make contributions to it. Besides that, I have been a consistent contributor to the open-source community, the submissions to which can be found in contributions section of my [GitHub](https://github.com/PrashantDandriyal) profile. To state them in chronological order:

* TensorFlow: The Pull Request was related to issue #35072 opened by Pete Warden (co-founder of TensorFlow Lite and TinyML). It was simply an explanation to "How int8 input and output quantization conversion works in TensorFlow Lite". The Pull has been approved and is ready to pull.

* Speech-VINO (Intel's OpenVINO toolkit): The page is meant for contributions to Intel's OpenVINO-toolkit-based projects. These projects incoporate sound applications. I have shared my project (in process of the "Introduction to Intel's OpenVINO toolkit" course, in which I am a scholar)

* Arduino: 
  * My first contribution to Arduino has been the modificatios to its "Ciao" [library](https://github.com/arduino-libraries/Ciao). I have updated the `readme.md` in official format. The Pull Request has been reviewed but merging is not yet done as the responsible maintainers have not gone through it. 
  * Next, I created an issues on Arduino_JSON [#6](https://github.com/arduino-libraries/Arduino_JSON/issues/6)
  * Participated in discussions on issues [#566](https://github.com/arduino/arduino-cli/issues/566) and [#549](https://github.com/arduino/arduino-cli/issues/549) in Arduino-cli.

* Miscellaneous: Other Open-Source contributions include the [OpenVINO](https://github.com/alihussainia/OpenDevLibrary) project. I have been an active contributor and now, a maintainer to the project. The project aims at creating a hassle-free portable version of the Intel OpenVINO (Open Visual inference and Neural Optimization) toolkit. Till now, the project has had successful runs and no issues.

> Did you do other code related projects or university courses? Do you have experience with Arduino?

In the University, I am enrolled in the Bachelor in Technology (B.Tech) course with specialization in Electronics and Communications Engineering. I have studied Microcontroller like the 8051 and Embedded Systems as credit courses. In my current semester, I have Neural Networks as a credit subject although I am already working on the topic in relation to "On-device learning for low-computation-capable devices". 
My initial experience with microcontrollers began with Arduino; some of the initial projects were related to "Home Automation" and bootloading the ATmega328p manually. I have used the board like the UNO, BluePill (by STM) and several 32 bit microcontroller-based boards by Texas Instruments. 

## Other Experiences

I am familiar with microcontrollers (both 8 bit & 32 bit) since my sophomore year. I have been a quarter-finalist (top 4% among >10,000 participants) in the India Innovation Challenge and Design Contest (IICDC-2018) during which our team was provided with Texas Instruments tools like the CC26X2R1 and TIVA LAUNCHPAD (EK-TM4C123GXL) EVM. My team's participation in IICDC, 2018 was the first step towards ML. I tried learning more about it by connecting to some of the prominent members of the community (like Evgeni Gousev, the co-founder of TinyML) and followed their guest lectures delivered at Harvard. 

I completed the _Introduction to TensorFlow_ course on Udacity and Andrew Ng's course on Coursera and participated in two of the ML competition on HackerEarth. I have been studying Machine Learning for about an year now. Mostly, I have used TensorFlow-Keras as the primary API. Currently I am enrolled into a scholarship programme offered by Udacity in collaboration with Intel, on OpenVINO toolkit, which is another method of implementing edgeAI. Regarding the languages, I have good experience of C, C++ and Python, all of which are he primary languages needed for this project. I have used C++ for several coding competitions held by Google.


## Why this project?

>Why you want to do this project ?

The aim for the selection of this project is based on my interests and experience. 
I have been learning and involved with Machine Learning for more than eight months now. Past that I was involved with Embedded Systems (as described above). As I was learning ML, I had a peculiar urge to find a bridge between both my interests. I stumbled upon the [TinyML](https://www.linkedin.com/groups/13694488?lipi=urn%3Ali%3Apage%3Ad_flagship3_feed%3BN5JXEs0NS4mp083miQjE2A%3D%3D&licu=urn%3Ali%3Acontrol%3Ad_flagship3_feed-feed_list_group)group on LinkedIn and knew it was the exact thing I was looking for. Since then, I have been diving deeper into the field of **EdgeAI** and **TinyML**. I have been planning to submit an official library to Arduino for implementing simple (shallow) Neural Networks on classic Arduino boards like the Uno. But then, GSoC seemed to coincide with it and I took it as an opportunity to continue my initiative but with mentors! The projects I wished to take on were the _Writing Examples for Official Libraries_ and _Examples and Tools for Portenta Board_. I had some clarifications with the mentor on GitHub and he suggested that I go for this project instead of porting TensorFlow Lite for the Portenta, which I proposed initially.

I believe that the completion of this project will be a contribution to the TinyML and open source Arduino and TensorFlow communities. Both Arduino as well as TensorFlow have been the first choice for beginners of Embedded systems and Machine Learning. The project has the potential of strengthening that tie. The excellent Arduino_TensorFlowLite library deserves more support. This project will encourage other developers to realize the capabilities of the Arduino Boards and port it for other classic boards. 


## Do you have any other commitments during the GSoC period?

> Provide dates, such as holidays, when you will not be available.

I will be unable on the some of the national gazetted holidays that are mentioned below. I also have my last semester exams in this period.

Holiday | Date in 2020
---|---
Good Friday | 10 April
Buddha Purnima | 7 May
Ramzan Id |25 May
Semester Exams | Somewhere in June (not disclosed yet)
Raksha Bandhan | 3 August
Janmashtmi | 11 August
Indian Independence Day | 15 August


 
