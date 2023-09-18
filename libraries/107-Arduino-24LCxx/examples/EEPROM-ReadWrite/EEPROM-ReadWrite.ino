/*
 * Basic example demonstrating how to use this library. Heavily inspired by the official littlefs minimal example.
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <107-Arduino-24LCxx.hpp>

#include <Wire.h>

#include <array>
#include <algorithm>

/**************************************************************************************
 * GLOBAL CONSTANTS
 **************************************************************************************/

static uint8_t const EEPROM_I2C_DEV_ADDR = 0x50;

/**************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************/

EEPROM_24LCxx eeprom(EEPROM_24LCxx_Type::LC64,
                     EEPROM_I2C_DEV_ADDR,
                     [](uint8_t const dev_addr) { Wire.beginTransmission(dev_addr); },
                     [](uint8_t const data) { Wire.write(data); },
                     []() { return Wire.endTransmission(); },
                     [](uint8_t const dev_addr, size_t const len) -> size_t { return Wire.requestFrom(dev_addr, len); },
                     []() { return Wire.available(); },
                     []() { return Wire.read(); });

/**************************************************************************************
 * SETUP/LOOP
 **************************************************************************************/

void setup()
{
  Serial.begin(115200);
  while (!Serial) { }
  delay(1000);

  Wire.begin();

  Serial.println(eeprom);

  Serial.print("Connect to EEPROM ... ");
  if (!eeprom.isConnected()) {
    Serial.println("ERROR");
    return;
  }
  Serial.println("OK");

  Serial.print("Fill write buffer ... ");
  std::array<uint8_t, 32> page_0_write_buf, page_1_write_buf;
  uint8_t cnt = 0;
  std::generate(page_0_write_buf.begin(), page_0_write_buf.end(), [&cnt]() { return cnt++; });
  std::generate(page_1_write_buf.begin(), page_1_write_buf.end(), [&cnt]() { return cnt++; });
  Serial.println("OK");

  Serial.print("Write to EEPROM ..... ");
  eeprom.write_page(0 * eeprom.page_size(), page_0_write_buf.data(), page_0_write_buf.size());
  eeprom.write_page(1 * eeprom.page_size(), page_1_write_buf.data(), page_1_write_buf.size());
  Serial.println("OK");

  Serial.print("Read from EEPROM .... ");
  std::array<uint8_t, 32> page_0_read_buf, page_1_read_buf;
  eeprom.read_page(0 * eeprom.page_size(), page_0_read_buf.data(), page_0_read_buf.size());
  eeprom.read_page(1 * eeprom.page_size(), page_1_read_buf.data(), page_1_read_buf.size());
  Serial.println("OK");

  Serial.print("Compare buffers ..... ");

  auto printReadWriteBuf = [](auto const & rw_buf)
  {
    for(size_t b = 0; b < rw_buf.size(); b += 8) {
      char msg[64] = {0};
      snprintf(msg,
               sizeof(msg),
               "\t%02X %02X %02X %02X %02X %02X %02X %02X",
               rw_buf[b+0], rw_buf[b+1], rw_buf[b+2], rw_buf[b+3],
               rw_buf[b+4], rw_buf[b+5], rw_buf[b+6], rw_buf[b+7]);
      Serial.println(msg);
    }
  };

  bool buffer_comparison_error = false;
  if (!std::equal(page_0_read_buf.begin(), page_0_read_buf.end(), page_0_write_buf.begin()))
  {
    Serial.println("ERROR (page #0)");
    buffer_comparison_error = true;

    Serial.println("write_buf = ");
    printReadWriteBuf(page_0_write_buf);
    Serial.println("read_buf  = ");
    printReadWriteBuf(page_0_read_buf);
  }
  if (!std::equal(page_1_read_buf.begin(), page_1_read_buf.end(), page_1_write_buf.begin()))
  {
    Serial.println("ERROR (page #1)");
    buffer_comparison_error = true;

    Serial.println("write_buf = ");
    printReadWriteBuf(page_1_write_buf);
    Serial.println("read_buf  = ");
    printReadWriteBuf(page_1_read_buf);
  }

  if (!buffer_comparison_error)
  {
    Serial.println("OK");
    Serial.println("EEPROM Success.");
  }
  else
    Serial.println("EEPROM Error.");
}

void loop()
{

}
