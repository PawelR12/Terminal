#include "protocol.h"



typedef enum msg_destination{
    voting_open = 10,
    ack_voting_open = 11,
    can_send_voices = 12,
    vote_send = 13,
    ack_vote_send = 14,
    vote_end = 15
}  msg_destination;


uint8_t get_lsb(uint8_t figure){
    figure = figure & 0x0F;
    return figure;
}

uint8_t get_msb(uint8_t figure){
    figure = figure >> 4;
    return figure;
}

Protocol :: Protocol(uint8_t own_add){
    ack_start_fill = 0;
    ack_vote_fill = 0;
    own_address = own_add;
    address = 0;
    msg_type = 0;
    msg = 0;
    check_sum = 0;
    connected_devices = 0;
    validate = false;
    my_last_vote = null_vote;
    
    yes_votes_number = 0;
    no_votes_number = 0;
    no_decision_votes_number = 0;
    voted_cast = 0;

    whole_data = (uint8_t*)calloc(3,sizeof(uint8_t));

    voting = voting_is_close;
    number_of_devices = 10;
    encryption = 0;
    device_array = (uint8_t*)calloc(number_of_devices, sizeof(uint8_t));

    for(int n = 1; n<= number_of_devices; n++){
        device_array[n-1] = n;
    }

    ack_start = (uint8_t*)calloc(number_of_devices, sizeof(uint8_t));
    voting_results= (uint8_t*)calloc(number_of_devices, sizeof(uint8_t));

}

void Protocol :: voting_receive(){

    receiveMessage(whole_data);
    uint32_t msg_before_encryption;
    msg_before_encryption = (whole_data[0] << 16) + (whole_data[1] << 8) + whole_data[2];
    if(msg_before_encryption != 0){
        Serial.print("Before encrypt: "); Serial.println(whole_data[1]);
        data_decryption(whole_data);
        Serial.print("After encrypt: "); Serial.println(whole_data[1]);
        whole_message = (whole_data[0] << 16) + (whole_data[1] << 8) + whole_data[2];
        if(whole_message != 0){
            Serial.print("Message: "); Serial.println(msg);
            Serial.print("Whole data: ");
            Serial.print(whole_data[0]); Serial.print(whole_data[1]); Serial.println(whole_data[2]);
        } 
        data_validate(whole_message);
    } else{
        validate = false;
    }
}

void Protocol :: data_validate(uint32_t data){
    validate = true;
    uint8_t received_address = whole_data[0];
    uint8_t sender_address = received_address & 0x0F;
    uint8_t receiver_address = received_address >> 4;
    uint8_t received_message = whole_data[1];
    uint8_t received_check_sum = whole_data[2];
    // ADDITIONAL CHECKING IN CASE
    if(data != 0){
        Serial.print("Data: ");
        Serial.print(data);
        Serial.print("  Sender address: ");
        Serial.print(sender_address);
        Serial.print("  Receiver address: ");
        Serial.print(receiver_address);
        Serial.print("  Msg: ");
        Serial.print(received_message);
        Serial.print("  check sum: ");
        Serial.print(received_check_sum);
        // Serial.print("Validate before address check: ");
        // Serial.println(validate);
    }
    if(data == 0){
        validate = false;
    }
    if(receiver_address != 0 && receiver_address != own_address){
        validate = false;
    }
    
    // if address is correct
    uint8_t cs = check_sum_func(received_address,received_message);
    if(cs != received_check_sum){
        validate = false;
    }
    if(data != 0){
    Serial.print("calculated check sum: ");
    Serial.println(cs);
    Serial.print("Received check sum: ");
    Serial.println(received_check_sum);
    
    Serial.print("Address receveid: ");
    Serial.println(receiver_address);
    Serial.print("Own address: ");
    Serial.println(own_address);
    Serial.print("Validate: ");
    Serial.println(validate);
    
    }
    // if check sum is correct
    if(!validate && data != 0) Serial.println("Validate: no");
    if(validate){
        Serial.println("Validate: yes");
        address = sender_address; // sender address
    
        if(get_msb(received_message) == 0){
            msg_type = get_lsb(received_message);
            msg = 0;
        }
        else{
            msg_type = get_msb(received_message);
            msg = get_lsb(received_message);
        }
        Serial.print("msg_type is: "); Serial.println(msg_type);
        check_sum = received_check_sum;
        Serial.println("Message is validated");
        msg_execution();
    }
}

void Protocol :: msg_execution(){
    Serial.println("Open msg_execution");
    Serial.print("Voting: ");
    Serial.println(voting);
    Serial.print("Msg type: ");
    Serial.println(msg_type);
    Serial.print("Msg lsb: ");
    Serial.println(msg);
    Serial.print("Votinng: ");
    Serial.println(voting);
    if(voting == voting_is_close && msg_type == voting_open){
        encryption = msg;
        voting_open_func();
        
        // Serial.println("I was there...");
    }
    else if(msg_type == ack_voting_open){
        ack_voting_open_func();
    }
    else if(voting == voting_is_open){
        Serial.print("Msg type: "); Serial.println(msg_type);
        
        switch(msg_type){    
            case can_send_voices:
                can_send_voices_func();
                break;
            case vote_send:
                vote_send_func(msg);
                break;
            default:
                break;
        }
    }

    else if(voting == after_voting){
        switch(msg_type){
        case ack_vote_send:
            ack_vote_send_func();
            break;
        default:
            break;
        }
    }

    else if(voting == after_voting || voting == after_voting_confirm){
        if(msg_type == vote_end)    vote_end_func();

    }
        
}

uint8_t Protocol :: check_sum_func(uint8_t val1, uint8_t val2){
    int16_t sum;
    int8_t ans;
    sum = (uint16_t)val1 + (uint16_t)val2;
    if(sum % 2)     sum++;
    ans = (uint8_t)(sum/2);
    return ans;
}

uint8_t Protocol :: get_address(){
    return address;
}

uint8_t Protocol :: get_msg_type()
{
    return msg_type;
}

uint8_t Protocol :: get_msg(){
    return msg;
}

uint8_t Protocol :: get_check_sum(){
    return check_sum;
}

uint8_t Protocol :: get_validate(){
    return validate;
}
void Protocol :: voting_open_func(){
    voting = voting_is_open; // switch on voting mode

    uint8_t response_msg = (ack_voting_open << 4);
    response_msg |= own_address;
    uint8_t destination_addr = 0;
    uint8_t my_addr = (destination_addr << 4);
    my_addr |= own_address;
    uint8_t new_check_sum = check_sum_func(my_addr, response_msg);
    uint8_t* response = createMessage(my_addr, response_msg, new_check_sum);
    Serial.print("Response_msg: "); Serial.println(response_msg);
    Serial.print("Check sum: "); Serial.println(new_check_sum);
    Serial.print("Address: "); Serial.println(my_addr);
    response = data_encryption(response);
    sendMessage(response, 3);
}

void Protocol :: ack_voting_open_func(){
    if(address > 0 && address <= number_of_devices){
        ack_start[address] = address;
    }
    
}

void Protocol :: vote_send_func(uint8_t last_vote){
    // add this vote to list of votes
    if(msg == vote_yes || msg == vote_no || msg == vote_no_decision){
        if(address > 0 && address <= number_of_devices){
            voting_results[address] = last_vote;
        }
        Serial.println(last_vote);
        // voting = during_voting;
        // send ack msg
        uint8_t voting_msg = (ack_vote_send << 4);
        voting_msg |= last_vote;
        uint8_t new_check_sum = check_sum_func(address, voting_msg);
        uint8_t* ack_your_vote = createMessage(address, voting_msg , new_check_sum);
        ack_your_vote = data_encryption(ack_your_vote);
        sendMessage(ack_your_vote, 3);
        Serial.print("MESSAGE SENDED: "); Serial.print(ack_your_vote[0]); Serial.print(ack_your_vote[1]); Serial.println(ack_your_vote[2]);

        // Checking votes
        check_votes_number();
        check_yes_votes();
        check_no_votes();
        check_no_decision_votes();
    }
}

void Protocol :: can_send_voices_func(){
    // sth lights
    voting = during_voting;
    Serial.println("During voting");
}
void Protocol :: ack_vote_send_func(){
    Serial.println("Proper message received");
    if(msg != my_last_vote){
        Serial.print("Wrong message received");
        send_voice(my_last_vote);
    }
    voting = after_voting_confirm;
}

void Protocol :: vote_end_func(){
    voting = voting_is_close;
    encryption = 0;
    
}

uint8_t Protocol :: check_fill(uint8_t* arr,uint8_t number_of_elements){
    uint8_t num;
    for(int n = 0; n < number_of_devices; n++){
        if(!arr[n]) num++;
    }
    return num;
}

void Protocol :: send_voting_open(uint8_t destination_address, uint8_t encryption_key){
    encryption = encryption_key;
    uint8_t msg = voting_open << 4 | encryption_key;
    uint8_t check_sum = check_sum_func(destination_address, msg);
    
    uint8_t *ptr = createMessage(destination_address, msg, check_sum);
    
    sendMessage(ptr, 3);
    uint32_t x = (uint32_t)destination_address << 16 | (uint32_t)msg << 8 | (uint32_t)check_sum;
    Serial.println(x);
    Serial.print("Address is: ");
    Serial.println(destination_address >> 16);

}

void Protocol :: send_can_vote(uint8_t destination_address){
    voting == during_voting;

    uint8_t msg = can_send_voices << 4;
    uint8_t check_sum = check_sum_func(destination_address, msg);
    
    uint8_t *ptr = createMessage(destination_address, msg, check_sum);
    ptr = data_encryption(ptr);
    sendMessage(ptr, 3);
    uint32_t x = (uint32_t)destination_address << 24 | (uint32_t)msg << 16 | (uint32_t)check_sum << 8;

}

void Protocol :: send_voice(vote_possibilites vote){
    uint8_t msg = (vote_send << 4) | vote;
    uint8_t addr = own_address;
    uint8_t check_sum = check_sum_func(addr, msg);
    uint8_t *ptr = createMessage(addr, msg, check_sum);
    ptr = data_encryption(ptr);
    sendMessage(ptr,3);
    uint32_t x = (uint32_t)msg << 16 | (uint32_t)check_sum << 8;
    voting = after_voting;
    my_last_vote = vote;

}

void Protocol :: send_close_voting(){
    uint8_t msg = vote_end << 4;
    uint8_t check_sum = check_sum_func(0,msg);

    uint8_t *ptr = createMessage(0, msg, check_sum);
    ptr = data_encryption(ptr);
    sendMessage(ptr,3);
    voting = voting_is_close;
}

uint8_t Protocol :: check_ack(){
    uint8_t number = 0;
    
    for(int n=0; n<number_of_devices; n++){
        if(ack_start[n] != 0)   number++;
    }
    connected_devices = number;
    return number;
}

uint8_t Protocol :: check_yes_votes(){
     uint8_t number = 0;
    
    for(int n=0; n<number_of_devices; n++){
        if(voting_results[n] == vote_yes)   number++;
    }
    yes_votes_number = number;
    return number;
}

uint8_t Protocol :: check_no_votes(){
     uint8_t number = 0;
    
    for(int n=0; n<number_of_devices; n++){
        if(voting_results[n] == vote_no)   number++;
    }
    no_votes_number = number;
    return number;
}

uint8_t Protocol :: check_no_decision_votes(){
     uint8_t number = 0;
    
    for(int n=0; n<number_of_devices; n++){
        if(voting_results[n] == vote_no_decision)   number++;
    }
    no_decision_votes_number = number;
    return number;
}

uint8_t Protocol :: check_votes_number(){
     uint8_t number = 0;
    
    for(int n=0; n<number_of_devices; n++){
        if(voting_results[n] != 0)   number++;
    }
    voted_cast = number;
    return number;
}


uint8_t* Protocol :: data_encryption(uint8_t* msg){
    if(encryption != 0){
        uint8_t *ptr;
        ptr = (uint8_t *)calloc(3, sizeof(uint8_t));
        ptr[0] = msg[0] + (encryption*15);
        ptr[1] = msg[1] + (encryption*15);
        ptr[2] = msg[2] + (encryption*15);
        return ptr;
    }
    else    return msg;
}

void Protocol :: data_decryption(uint8_t *data){
    if(encryption != 0){
        data[0] = data[0] - (encryption*15);
        data[1] = data[1] - (encryption*15);
        data[2] = data[2] - (encryption*15);
    }
}

void Protocol :: clear_votes(){
    address = 0;
    msg_type = 0;
    msg = 0;
    connected_devices = 0;
    yes_votes_number = 0;
    no_votes_number = 0;
    no_decision_votes_number = 0;
    voted_cast = 0;
    encryption = 0;
}