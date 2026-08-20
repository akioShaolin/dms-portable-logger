#include <unity.h>
#include <JbdCodec.h>
using namespace dms;
void test_write_is_valid_but_identifiable_and_error_encoder(){uint8_t w[]={0xDD,0x5A,0xE1,0,0xFF,0x1F,0x77};JbdFrame f;TEST_ASSERT_EQUAL(JbdResult::OK,validateJbdRequest(w,sizeof w,&f));TEST_ASSERT_EQUAL_HEX8(0x5A,w[1]);uint8_t e[7];encodeJbdError(0xE1,0x80,e,sizeof e);TEST_ASSERT_EQUAL_HEX8(0x80,e[2]);}
void test_noise_resync(){uint8_t payload[]={0x01,0x02};uint8_t r[]={0xDD,0x04,0,2,1,2,0xFF,0xFB,0x77};JbdStreamParser p;JbdFrame f;JbdResult e;TEST_ASSERT_FALSE(p.push(0,e==JbdResult::OK?f:f,e));bool ok=false;for(auto b:r)ok|=p.push(b,f,e);TEST_ASSERT_TRUE(ok);}
int main(int,char**){UNITY_BEGIN();RUN_TEST(test_write_is_valid_but_identifiable_and_error_encoder);RUN_TEST(test_noise_resync);return UNITY_END();}
