#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

typedef struct _message {
	uint8_t senderID: 4;
	uint8_t resendID: 3;
	uint8_t reserved: 1;
	uint16_t message: 16;
} Message;

typedef union _payload {
	Message msg;
	uint8_t binary[3];
} Payload;

typedef enum _msgtype {
  MSG_LO = 0,
  MSG_HI
} MsgType;

int validate(Payload *payload);
void setMsg(Payload *payload, MsgType type, uint8_t value);
uint8_t readMsg(Payload *payload, MsgType type);

#endif

