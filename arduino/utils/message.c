#include <stdio.h>
#include "message.h"

void debug_printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--) {
        for (j=7;j>=0;j--) {
          byte = (b[i] >> j) & 1;
          printf("%u", byte);
        }
        printf(" ");
    }
    printf("\n");
}

int validate(Message *msg) {
    return (msg->senderID >= 0 && msg->senderID <= 15) &&
           (msg->resendID >=0 && msg->resendID <= 7) &&
           (msg->message >= 0 && msg->message <= 255);
}
