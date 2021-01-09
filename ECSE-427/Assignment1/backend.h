#ifndef BACKEND
#define BACKEND

#include <stdint.h>

#include "rpc.h"

/**
 * Adds 2 integers
 * 
 *  @params:
 *      a: the 1st integer to be added
 *      b: the 2nd integer to be added
 * 
 *  @return: The return value will be the int resulting from the expression a + b
 * 
 **/
int addInts(int a, int b);

/**
 * Multiply 2 integers
 * 
 *  @params:
 *      a: the 1st integer to be multiplied
 *      b: the 2nd integer to be multiplied
 * 
 *  @return: The return value will be the int resulting from the expression a * b
 * 
 **/
int multiplyInts(int a, int b);

/**
 * Divides 2 integers
 * 
 *  @params:
 *      a: the dividend
 *      b: the divisor
 * 
 *  @return: The return value will be the int resulting from the expression a / b. Or -1 if b is 0.
 * 
 **/
float divideFloats(float a, float b);

/**
 * Sleeps for certain amount of seconds
 * 
 *  @params:
 *      a: the number of seconds to sleep for
 * 
 *  @return: The return value will be 0
 * 
 **/
int sleepFor(int x);

/**
 *  Factorial of a uint64_t
 * 
 *  @params:
 *      a: the uint64_t to be used
 * 
 *  @return: The return value will be a! (a factorial)
 * 
 **/
uint64_t factorial(int x);

/**
 * Will handle a request
 * 
 * @params: 
 *      request: req object containg a string cmd and possibly 2 paramters
 *      socket: the cleint socket file descirptor to send the response message
 **/
void handleCommand(req request, int socket);

/**
 * Will check paramters
 * 
 * @params: 
 *      request: req object containg a string cmd and possibly 2 paramters
 *      socket: the cleint socket file descirptor to send the response message
 * 
 * @return: Will return 0 if paramters for specified command are present and -1 otherwise
 *    
 **/
int checkParameters(req request, int socket);

/**
 * Will perform requested command
 * 
 * @params: 
 *      request: req object containg a string cmd and possibly 2 paramters
 *      socket: the cleint socket file descirptor to send the response message
 * 
 *  Note that this function is what calls addInts, multiplyInts....
 * 
 **/
void performCommand(req request, int socket);

#endif