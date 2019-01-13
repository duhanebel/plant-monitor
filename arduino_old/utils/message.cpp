#include <stdio.h>
#include "message.h"

#define LOW_VALUE_MASK 0x00ff
#define HIGH_VALUE_MASK 0xff00


int validate(Payload *payload) {
    return (payload->msg.senderID >= 0 && payload->msg.senderID < 16) &&
           (payload->msg.resendID >=0 && payload->msg.resendID < 8) &&
           (payload->msg.message >= 0 && payload->msg.message < 65536);
}

void setMsg(Payload *payload, MsgType type, uint8_t value)
{
  switch(type) {
    case MSG_LO:
      payload->msg.message &= ~LOW_VALUE_MASK;
      payload->msg.message |= value;
      break;
    case MSG_HI:
      payload->msg.message &= ~HIGH_VALUE_MASK;
      payload->msg.message |= (value << 8);
      break;
  }
}


uint8_t readMsg(Payload *payload, MsgType type)
{
    return (type == MSG_LO)? (payload->msg.message & LOW_VALUE_MASK) : (payload->msg.message >> 8);
}


