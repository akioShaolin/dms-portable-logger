#include <unity.h>
#include <JbdCodec.h>
using namespace dms;
void write_block(){uint8_t w[]={0xDD,0x5A,0xE1,0,0xFF,0x1F,0x77};JbdFrame f;TEST_ASSERT_EQUAL_INT((int)JbdResult::OK,(int)validateJbdRequest(w,sizeof w,&f));TEST_ASSERT_EQUAL_HEX8(0x5A,f.bytes[1]);uint8_t e[7];encodeJbdError(0xE1,0x80,e,sizeof e);TEST_ASSERT_EQUAL_HEX8(0x80,e[2]);}
void noise_resync(){uint8_t r[]={0xDD,0x04,0,2,1,2,0xFF,0xFB,0x77};JbdStreamParser p;JbdFrame f;JbdResult e=JbdResult::INCOMPLETE;TEST_ASSERT_FALSE(p.push(0,4,f,e));bool ok=false;for(auto b:r)ok|=p.push(b,4,f,e);TEST_ASSERT_TRUE(ok);TEST_ASSERT_EQUAL(4,f.command);}
void wrong_expected(){uint8_t r[]={0xDD,0x04,0,0,0,0,0x77};uint16_t c=jbdChecksum(r+2,2);r[4]=c>>8;r[5]=c;JbdFrame f;TEST_ASSERT_EQUAL_INT((int)JbdResult::BAD_COMMAND,(int)validateJbdResponse(r,sizeof r,3,&f));}
int main(){UNITY_BEGIN();RUN_TEST(write_block);RUN_TEST(noise_resync);RUN_TEST(wrong_expected);return UNITY_END();}
