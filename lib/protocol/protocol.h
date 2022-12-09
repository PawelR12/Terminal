#ifndef _PROTOCOL_H_INCLUDED
#define _PROTOCOL_H_INCLUDED
// #include "../../include/header.h"
#include "../wireless/wireless.h"
#include <stdint.h>


typedef enum{
  voting_is_open,
  voting_is_close,
  during_voting,
  after_voting,
  after_voting_confirm,
} voting_status;

uint8_t get_lsb(uint8_t);
uint8_t get_msb(uint8_t);

// extern voting_status voting;
// extern uint8_t *device_array;
// extern uint8_t *ack_start;
// extern uint8_t *voting_results;
// extern uint8_t number_of_devices;

typedef enum vote_possibilites{
    vote_yes = 0b1111,
    vote_no = 0b0001,
    vote_no_decision = 0b1010,
    null_vote = 0,
} vote_possibilites;

class Protocol{
    public:        
        uint8_t ack_start_fill;
        uint8_t ack_vote_fill;
        uint8_t number_of_devices;
        uint8_t* device_array;
        uint8_t* ack_start;
        uint8_t* voting_results;
        uint8_t connected_devices;

        uint8_t yes_votes_number;
        uint8_t no_votes_number;
        uint8_t no_decision_votes_number;
        uint8_t voted_cast;

        voting_status voting;

        Protocol(uint8_t own_add);
        void divide_message(uint8_t* data);
        void data_validate(uint32_t data);
        void msg_execution();
        uint8_t check_sum_func(uint8_t val1, uint8_t val2);


        void voting_receive();
        uint8_t check_fill(uint8_t* arr,uint8_t number_of_elements);
        uint8_t check_ack();

        void send_voting_open(uint8_t destination_address, uint8_t encryption_key);
        void send_can_vote(uint8_t destination_address);
        void send_voice(vote_possibilites vote);
        void send_close_voting();
        uint8_t get_address();
        uint8_t get_msg_type();
        uint8_t get_msg();
        uint8_t get_check_sum();
        uint8_t get_validate();
        uint8_t check_yes_votes();
        uint8_t check_no_votes();
        uint8_t check_no_decision_votes();
        uint8_t check_votes_number();
        void clear_votes();
        vote_possibilites my_last_vote;
    private:
        uint8_t own_address; // address of the device
    // received
        uint8_t address; // address of last received message
        uint8_t msg_type;
        uint8_t msg;
        uint8_t check_sum;
        uint8_t validate;
        uint8_t *whole_data;
        uint8_t encryption;
        uint32_t whole_message;
        
        void voting_open_func();
        void ack_voting_open_func();
        void can_send_voices_func();
        void vote_send_func(uint8_t own_vote);
        void ack_vote_send_func(); // tbd
        void vote_end_func();
        uint8_t* data_encryption(uint8_t* msg);
        void data_decryption(uint8_t *data);
        
        
};


#endif