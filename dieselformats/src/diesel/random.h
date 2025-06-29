#pragma once

///
/// Implementation of Diesel's Linear Congruential Generator (LCG). Autodesk Stingray/BitSquid use the same algorithm with the same constants.
///

#include <stdint.h>

namespace diesel {
  class DieselLCG {
  public:
    // Creates an LCG with the starting state 0.
    DieselLCG();
  public:
    
    void set_seed(uint32_t seed) { _state = seed; }

    // Returns a random number between 0 and 1
    double random();
    // Returns a random number between 0 and max
    double random(double max);
    // Returns a random number between min and max
    double random(double min, double max);

    // Returns a random number between 0 and 1
    float randomf() { return (float)random(); }
    // Returns a random number between 0 and max
    float randomf(float max) { return (float)random((double)max); }
    // Returns a random number between min and max
    float randomf(float min, float max) { return (float)random((double)min, (double)max); }
  private:
    uint32_t _state;
  };
}