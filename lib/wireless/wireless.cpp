#include "wireless.h"

// address
// destination + msg
// checking

uint8_t* createMessage(uint8_t address, uint8_t msg, uint8_t check_sum){
    uint8_t *ptr;
    ptr = (uint8_t *)calloc(3, sizeof(uint8_t));
    ptr[0] = address;
    ptr[1] = msg;
    ptr[2] = check_sum;

    return ptr;
}
void receiveMessage(uint8_t *data){
    int packetSize = LoRa.parsePacket();
    uint32_t msg = 0;
    for(int s = 0; s<3; s++){
        data[s] = 0;
    }
    if (packetSize){
        uint8_t n = 0;

        if (packetSize){
            while(LoRa.available()){
    
                data[n] = LoRa.read();
                n++;
            }
        }
        
        // uint32_t tmp1, tmp2, tmp3;
        // tmp1 = (msg & 0x00000F) << 20;
        // tmp2 = (msg & 0x0000F0) << 12;
        // tmp3 = tmp1 | tmp2;
        // tmp1 = (msg & 0x000F00) << 4;
        // tmp2 = (msg & 0x00F000) >> 4;
        // tmp3 = tmp3 | (tmp1 | tmp2);
        // tmp1 = (msg & 0x0F0000) >> 12;
        // tmp2 = (msg & 0xF00000) >> 20;
        // tmp3 = tmp3 | (tmp1 | tmp2); 
        // msg = tmp3;
        // Serial.print("----- MSG RECEIVED: ");
        // Serial.println(msg);
        // return msg;
    // }
    // else{
    //     return 0;
    }
}


void sendMessage(uint8_t* msg, uint8_t msgSize){
    
    Serial.println("----- MSG SENDED: ");
    LoRa.beginPacket();
    for(int n=0; n<msgSize; n++){
        LoRa.write(msg[n]);
        Serial.print(msg[n]);
    }
    LoRa.endPacket();
    
}