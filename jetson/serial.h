/**
 * @brief 
 * 
 * @file my_serial.h
 * @author Aakash
 * @author Miles
 * @date 2018-04-29
 */
#pragma once

#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define UART1 "/dev/ttyS1"
#define UART2 "/dev/ttyS2"
#define UART4 "/dev/ttyS4"
#define UART_TEST "/dev/ttyUSB0"
#define ARDUINO_SERIAL "/dev/ttyACM0"