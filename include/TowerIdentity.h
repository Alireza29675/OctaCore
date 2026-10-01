#ifndef OCTACORE_TOWER_IDENTITY_H
#define OCTACORE_TOWER_IDENTITY_H

#include <stdint.h>

#ifndef OCTACORE_TOWER_ID
#error "Select an explicit left, right, or tower1 through tower4 build."
#endif
#ifndef OCTACORE_ROLE
#error "The selected build must define its hostname suffix."
#endif
static_assert(OCTACORE_TOWER_ID >= 1 && OCTACORE_TOWER_ID <= 4, "Tower ID must be 1 through 4.");

#define OCTACORE_STRINGIFY_VALUE(value) #value
#define OCTACORE_STRINGIFY(value) OCTACORE_STRINGIFY_VALUE(value)
#define OCTACORE_HOSTNAME "octacore-" OCTACORE_ROLE
#define OCTACORE_TOWER_LABEL OCTACORE_STRINGIFY(OCTACORE_TOWER_ID)
static const uint8_t OCTACORE_WIRE_ROLE = OCTACORE_TOWER_ID - 1;

#endif
