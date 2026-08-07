#pragma once

#include <cstdint>
#include <bitset>

using Entity = std::uint32_t; // Entities are represented as 32-bit unsigned integers
constexpr Entity MAX_ENTITIES = 5000; // Maximum number of entities in the system

using ComponentType = std::uint8_t; // Component types are represented as 8-bit unsigned integers   
constexpr ComponentType MAX_COMPONENTS = 32; // Maximum number of components in the system

using Signature = std::bitset<MAX_COMPONENTS>; // Signature is a bitset representing the components an entity has