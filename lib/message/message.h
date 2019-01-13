#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

typedef struct _message {
	uint8_t senderID: 5;
	uint8_t resendID: 3;
	
	uint8_t data[2];
} Message;

typedef union _payload {
	Message message;
	uint8_t binary[3];
} Payload;

int payload_validate(const Payload* payload);
Payload payload_create();

#endif

