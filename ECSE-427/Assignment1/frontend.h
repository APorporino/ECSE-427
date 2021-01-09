#include "rpc.h"

/**
 * Parse input will fill a request object with input data
 *
 * @params:
 *      message: a pointer to a request object
 *      input: a string inputted from user
 * 
 * No return value, instead the message object will be filled,
 * with appropriate command and paramter variables matching the input string.
 *      
 * Essentially performs paramter marshalling.
 **/
void parseInput(req *message, char *input);