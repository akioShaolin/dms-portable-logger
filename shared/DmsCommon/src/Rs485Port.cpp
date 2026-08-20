#include "Rs485Port.h"
#ifdef ARDUINO
#include "Ed100Pins.h"
#include <driver/gpio.h>
#include <soc/gpio_sig_map.h>
#include <soc/gpio_pins.h>
#include <esp_rom_gpio.h>
namespace dms {
void Rs485Port::begin(uint32_t baud){pinMode(pins::RS485_DIR,OUTPUT);digitalWrite(pins::RS485_DIR,LOW);serial_.setRxBufferSize(2048);serial_.begin(baud,SERIAL_8N1,pins::UART2_RX,pins::UART2_TX);}
bool Rs485Port::transmit(const uint8_t*data,size_t length,TickType_t timeout){
  esp_rom_gpio_connect_in_signal(GPIO_MATRIX_CONST_ONE_INPUT,U2RXD_IN_IDX,false);
  gpio_set_level(gpio_num_t(pins::RS485_DIR),1);
  const bool ok=serial_.write(data,length)==length && uart_wait_tx_done(UART_NUM_2,timeout)==ESP_OK;
  uart_flush_input(UART_NUM_2);
  gpio_set_level(gpio_num_t(pins::RS485_DIR),0);
  esp_rom_gpio_connect_in_signal(pins::UART2_RX,U2RXD_IN_IDX,false);
  return ok;
}}
#endif
