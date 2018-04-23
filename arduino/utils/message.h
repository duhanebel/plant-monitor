typedef struct _message {
	unsigned char senderID: 4;
	unsigned char resendID: 3;
	unsigned char reserved: 1;
	unsigned char message: 8;
} Message;

typedef union _payload {
	Message msg;
	char *binary;
} Payload;

#ifdef DEBUG
void debug_printBits(size_t const size, void const * const ptr);
#endif

int validate(Message *msg);
