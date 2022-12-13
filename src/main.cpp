#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h>
#include "wireless.h"
// #include "../../CentralVoting/lib/protocol/protocol.h"
#include "protocol.h"


// Definition for RFM95W 
// #define ss 5
// #define rst 14
// #define dio0 2
// static uint8_t voting = 0;

#define YES_BUTTON 4
#define NO_BUTTON 5
#define NO_DECISION_BUTTON 6

#define GREEN_LED A3
#define YELLOW_LED A4
#define RED_LED A5

void led_blink(uint8_t led, uint8_t interval, uint8_t times);
Protocol protocol(0x01);
void setup() {
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);

  digitalWrite(YELLOW_LED, HIGH);
  pinMode(2, INPUT); // 
  // Set serial communication and communication with LoRa transceiver
  Serial.begin(9600);
  // LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(866E6)){
    Serial.println("Lora Communication Fail");
    delay(300);
  }
  LoRa.setSyncWord(0xA7);
  Serial.println("Terminal connected with LoRa");
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  // LoRa.setSyncWord(0xA7); // We can only receive and send messages to the devices
  // with the same SyncWord number
  pinMode(YES_BUTTON, INPUT);
  pinMode(NO_BUTTON, INPUT);
  pinMode(NO_DECISION_BUTTON, INPUT);
}

void loop() {
  protocol.voting_receive();
  
  if(protocol.voting == voting_is_close){
    protocol.voting_receive();
    if(protocol.get_validate() == true){
      led_blink(GREEN_LED, 50, 4);
      Serial.print("Address is: ");
      Serial.println(protocol.get_address());
      Serial.println("Voting start");
      Serial.print("Voting status: "); Serial.println(protocol.voting);
    }
  }
  else if(protocol.voting == during_voting){
    Serial.println("You can choose your voice");
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, HIGH);
    while(!((digitalRead(YES_BUTTON)) || (digitalRead(NO_BUTTON)) || (digitalRead(NO_DECISION_BUTTON)))){
      delay(20); //standard delay for pick the voice
      digitalWrite(GREEN_LED, LOW);
    } 
    if(digitalRead(YES_BUTTON)){
      protocol.send_voice(vote_yes);
      Serial.println("Yes vote is sended");
      led_blink(GREEN_LED, 50, 4);
    }
    else if(digitalRead(NO_BUTTON)){
      protocol.send_voice(vote_no);
      Serial.println("No vote is sended");
      led_blink(GREEN_LED, 50, 4);
    }
    else if(digitalRead(NO_DECISION_BUTTON)){
      protocol.send_voice(vote_no_decision);
      Serial.println("No vote is sended");
      led_blink(GREEN_LED, 50, 4);
    }
    unsigned long timer1 = millis() + 500;
    Serial.println(timer1);
    digitalWrite(RED_LED, HIGH);
    while(protocol.get_msg() != protocol.my_last_vote){
      if(millis() < timer1){
        protocol.voting_receive();
      }
      else{
        protocol.send_voice(protocol.my_last_vote);
        timer1 = millis() + 500;
      }
      if(protocol.get_msg() == protocol.my_last_vote && protocol.my_last_vote != 0){
        digitalWrite(RED_LED, LOW);
        digitalWrite(YELLOW_LED, LOW);
        led_blink(GREEN_LED, 50, 4);
        digitalWrite(GREEN_LED, HIGH);
      }
    }
    
  }
  // delay(20);

  
}


void led_blink(uint8_t led, uint8_t interval, uint8_t times){
  for(int i = 0; i< times; i++){
    digitalWrite(led, !digitalRead(led));
    delay(interval);
  }
}