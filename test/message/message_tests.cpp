#include <message.h>
#include <unity.h>

void test_payload_creation(void) {
    Payload payload = payload_create();
    TEST_ASSERT_EQUAL(payload.binary[0], 0);
    TEST_ASSERT_EQUAL(payload.binary[1], 0);
    TEST_ASSERT_EQUAL(payload.binary[2], 0);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_payload_creation);
    UNITY_END();

    return 0;
}