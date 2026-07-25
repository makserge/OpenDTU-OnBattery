#pragma once

#include <BL0942.h>

namespace PowermeterBls {

// The upstream BL0942 library (santerilindfors/BL0942) has a constructor bug:
//   BL0942::BL0942(HardwareSerial &serial, uint8_t address)
//       : serial_(serial), address_(address_) {}
// address_ is initialized from itself instead of from the `address`
// parameter, so it ends up holding whatever garbage was already in that
// memory. address_ is OR'd into every command byte and used to validate the
// checksum of every received packet, so a non-zero garbage value makes all
// communication with the chip fail silently (readings stay at 0).
//
// patches/bl0942/address-init.patch fixes this at the source, but that only
// takes effect if the patch still applies cleanly after a library update.
// This wrapper fixes it independently of the patch: address_ is `protected`,
// so we can correct it ourselves right after the (buggy) base constructor
// runs, without touching the vendored library at all.
class BL0942Fixed : public bl0942::BL0942 {
public:
    explicit BL0942Fixed(HardwareSerial& serial, uint8_t address = 0)
        : bl0942::BL0942(serial, address)
    {
        address_ = address;
    }
};

} // namespace PowermeterBls
