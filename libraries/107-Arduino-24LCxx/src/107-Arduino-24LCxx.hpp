/**
 * This software is distributed under the terms of the MIT License.
 * Copyright (c) 2023 LXRobotics.
 * Author: Alexander Entinger <alexander.entinger@lxrobotics.com>
 * Contributors: https://github.com/107-systems/107-Arduino-24LCxx/graphs/contributors.
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <array>

/**************************************************************************************
 * TYPEDEF
 **************************************************************************************/

enum class EEPROM_24LCxx_Type
{
  LC32, LC64, LC128, LC256, LC512
};

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

template<typename WireBeginFunc, typename WireWriteFunc, typename WireEndFunc, typename WireRequestFromFunc, typename WireAvailableFunc, typename WireReadFunc>
class EEPROM_24LCxx final : public
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_SAMD_ADAFRUIT)
Printable
#else
arduino::Printable
#endif
{
public:
  EEPROM_24LCxx(EEPROM_24LCxx_Type const type,
                uint8_t const dev_addr,
                WireBeginFunc && wire_begin,
                WireWriteFunc && wire_write,
                WireEndFunc && wire_end,
                WireRequestFromFunc && wire_request_from,
                WireAvailableFunc && wire_available,
                WireReadFunc && wire_read)
  : _type{type}
  , _dev_addr{dev_addr}
  , _wire_begin{std::forward<WireBeginFunc>(wire_begin)}
  , _wire_write{std::forward<WireWriteFunc>(wire_write)}
  , _wire_end{std::forward<WireEndFunc>(wire_end)}
  , _wire_request_from{std::forward<WireRequestFromFunc>(wire_request_from)}
  , _wire_available{std::forward<WireAvailableFunc>(wire_available)}
  , _wire_read{std::forward<WireReadFunc>(wire_read)}
  { }

  virtual ~EEPROM_24LCxx() { }

  bool isConnected()
  {
    return isEepromReady();
  }

  virtual size_t printTo(Print & p) const override
  {
    char msg_buf[64] = {0};
    snprintf(msg_buf, sizeof(msg_buf), "%s:\n\r\tdev  size: %5d\n\r\tpage size: %5d", device_name(), page_size(), device_size());
    return p.write(msg_buf);
  }

  size_t fill_page(uint16_t const mem_addr, uint8_t const val)
  {
    while (!isEepromReady()) yield();

    /* Write memory page address. */
    _wire_begin(_dev_addr);
    writeMemoryAddress(mem_addr);

    /* Write memory page data. */
    size_t bytes_written = 0;
    for(bytes_written = 0; bytes_written < page_size(); bytes_written++)
      _wire_write(val);

    _wire_end();

    return bytes_written;
  }

  size_t write_page(uint16_t const mem_addr, uint8_t const * data, size_t const num_bytes)
  {
    while (!isEepromReady()) yield();

    /* Write memory page address. */
    _wire_begin(_dev_addr);
    writeMemoryAddress(mem_addr);

    /* Write memory page data. */
    size_t const bytes_to_write = std::min(num_bytes, page_size());
    size_t bytes_written = 0;
    for(bytes_written = 0; bytes_written < bytes_to_write; bytes_written++)
      _wire_write(data[bytes_written]);

    _wire_end();

    return bytes_written;
  }

  size_t read_page(uint16_t const mem_addr, uint8_t * data, size_t const num_bytes)
  {
    while (!isEepromReady()) yield();

    /* Write memory page address. */
    _wire_begin(_dev_addr);
    writeMemoryAddress(mem_addr);
    _wire_end();

    /* Read memory page data. */
    size_t const bytes_to_read = std::min(num_bytes, page_size());
    _wire_request_from(_dev_addr, bytes_to_read);

    size_t bytes_read = 0;
    for (; (bytes_read < bytes_to_read) && _wire_available(); bytes_read++)
      data[bytes_read] = _wire_read();

    return bytes_read;
  }

  constexpr char const * device_name() const
  {
    switch(_type)
    {
      case EEPROM_24LCxx_Type::LC32:  return  "LC32";
      case EEPROM_24LCxx_Type::LC64:  return  "LC64";
      case EEPROM_24LCxx_Type::LC128: return "LC128";
      case EEPROM_24LCxx_Type::LC256: return "LC256";
      case EEPROM_24LCxx_Type::LC512: return "LC512";
      default: __builtin_unreachable();
    }
  }

  constexpr size_t device_size() const
  {
    switch(_type)
    {
      case EEPROM_24LCxx_Type::LC32:  return  4096;
      case EEPROM_24LCxx_Type::LC64:  return  8192;
      case EEPROM_24LCxx_Type::LC128: return 16384;
      case EEPROM_24LCxx_Type::LC256: return 32768;
      case EEPROM_24LCxx_Type::LC512: return 65536;
      default: __builtin_unreachable();
    }
  }

  constexpr size_t page_size() const
  {
    switch(_type)
    {
      case EEPROM_24LCxx_Type::LC32:  return  32;
      case EEPROM_24LCxx_Type::LC64:  return  32;
      case EEPROM_24LCxx_Type::LC128: return  64;
      case EEPROM_24LCxx_Type::LC256: return  64;
      case EEPROM_24LCxx_Type::LC512: return 128;
      default: __builtin_unreachable();
    }
  }

private:
  EEPROM_24LCxx_Type const _type;
  uint8_t const _dev_addr;
  WireBeginFunc _wire_begin;
  WireWriteFunc _wire_write;
  WireEndFunc _wire_end;
  WireRequestFromFunc _wire_request_from;
  WireAvailableFunc _wire_available;
  WireReadFunc _wire_read;

  typedef union {
    struct {
      uint8_t low_byte;
      uint8_t high_byte;
    } bytes;
    uint16_t word;
  } UMemoryAddress;

  void writeMemoryAddress(uint16_t const mem_addr)
  {
    UMemoryAddress block_addr;
    block_addr.word  = (mem_addr & (device_size() - 1));  /* Limit to 13 address bits. */
    block_addr.word &= ~(page_size() - 1);                /* Align to block size. */

    _wire_write(block_addr.bytes.high_byte);
    _wire_write(block_addr.bytes.low_byte);
  }

  bool isEepromReady()
  {
    _wire_begin(_dev_addr);
    auto const rc_end = _wire_end();
    return (rc_end == 0);
  }
};
