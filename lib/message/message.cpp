#include <stdio.h>
#include "message.h"

Payload payload_create() {
    Payload payload = { message: { senderID: 0, resendID: 0, data: {0, 0}}};
    return payload;
}

int payload_validate(const Payload* payload) {
    return (payload->message.senderID >= 0 && payload->message.senderID < 32) &&
           (payload->message.resendID >=0 && payload->message.resendID < 8);
}
