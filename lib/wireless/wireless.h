#ifndef _WIRELESS_H_INCLUDED
#define _WIRELESS_H_INCLUDED


#include <LoRa.h>
#include <SPI.h>

uint8_t* createMessage(uint8_t address, uint8_t msg, uint8_t check_sum);
void sendMessage(uint8_t* msg, uint8_t msgSize);
void receiveMessage(uint8_t *data);

#endif