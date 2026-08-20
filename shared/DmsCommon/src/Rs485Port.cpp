#include "Rs485Port.h"
#ifdef ARDUINO
#include "Ed100Pins.h"
#include <driver/gpio.h>
#include <esp32-hal-matrix.h>
#include <soc/gpio_sig_map.h>
#include <soc/gpio_pins.h>
namespace dms { class TransmitModeGuard { public:TransmitModeGuard(){pinMatrixInAttach(GPIO_MATRIX_CONST_ONE_INPUT,U2RXD_IN_IDX,false);gpio_set_level(gpio_num_t(pins::RS485_DIR),1);}~TransmitModeGuard(){uart_wait_tx_done(UART_NUM_2,pdMS_TO_TICKS(250));uart_flush_input(UART_NUM_2);gpio_set_level(gpio_num_t(pins::RS485_DIR),0);pinMatrixInAttach(pins::UART2_RX,U2RXD_IN_IDX,false);} };void Rs485Port::begin(uint32_t b){pinMode(pins::RS485_DIR,OUTPUT);digitalWrite(pins::RS485_DIR,LOW);serial_.begin(b,SERIAL_8N1,pins::UART2_RX,pins::UART2_TX);}bool Rs485Port::transmit(const uint8_t*d,size_t n,TickType_t t){TransmitModeGuard g;if(serial_.write(d,n)!=n)return false;return uart_wait_tx_done(UART_NUM_2,t)==ESP_OK;} }
#endif
