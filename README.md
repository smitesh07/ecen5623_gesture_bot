
# Final project for ECEN 5623 Real Time Embedded Systems
# Project Name : Hand Gesture Controlled Robot
## Authors - Smitesh Modak and Aakash Kumar


![Alt text](./Robot.png?raw=true "Hand Gesture Controlled Robot")

# Description
The project aims at building a Real-Time Embedded design which comprises of more than two services functioning and obeying the definition of Real-Time Embedded System i.e. being deterministic as well as functionally correct.

The software consists total of two services in the design. One service works on processing the frame captured and generating the desired signal. The other service will grab the desired signal and control the motor. The control of motor is achieved using UART sending out signal to Arduino from NVIDIA Jetson TK1 and further using motor drive shield for Arduino to drive the motor.

The entire system was build using prior experience with Arduino, OpenCV references and using the real-time concepts learned during the course.

# Future Scope

• Wireless communication between NVIDIA Jetson TK1 and Arduino

• Better recognition of hand using hue and saturation to correctly identify human hand within a frame

• Recognition of more and better gestures

• Addition of voice commands

# References

### OpenCV installation on NVIDIA Jetson TK1

• https://elinux.org/Jetson/Installing_OpenCV

### Hand Detection and Gesture Recognition

• https://s-ln.in/2013/04/18/hand-tracking-and-gesture-detection-opencv/

• https://github.com/udit043/Hand-Recognition-using-OpenCV

• https://medium.com/@muehler.v/simple-hand-gesture-recognition-using-opencv-and-javascript-eb3d6ced28a0

• https://www.intorobotics.com/9-opencv-tutorials-hand-gesture-detection-recognition/

• https://docs.opencv.org/3.1.0/d9/db7/group__datasets__gr.html

• http://opencvexamples.blogspot.com/2013/10/convex-hull.html

### Threading and Cyclic Executive

• http://mercury.pr.erau.edu/~siewerts/cec450/code/sequencer_generic/

### Completion and Scheduling Point Tests

• http://mercury.pr.erau.edu/~siewerts/cec450/code/Feasibility/feasibility_tests.c

### Real-Time Concepts and Linux

• Real Time Embedded Components and Systems with Linux and RTOS, By Sam Siewert and John Pratt

• https://linux.die.net/man/ - Linux Man Pages
