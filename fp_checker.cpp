#include <bitset>
#include <cstdint>
#include <cstring>
#include <exception>
#include <iostream>
#include <limits>
#include <string>

/*
 * FloatBits
 *
 * This class helps us look at the raw IEEE 754 bits of a float.
 * It lets us easily grab the sign, exponent, and fraction parts,
 * and also print the bits in the format required by the assignment.
 */

class FloatBits {
public:
    // Store the float and copy its raw bits into an integer
    explicit FloatBits(float value) : value_(value) {
        std::memcpy(&bits_, &value_, sizeof(float));
    }

    // Return full 32-bit representation
    uint32_t raw() const { return bits_; }

    // Get sign bit (0 = positive, 1 = negative)
    int sign() const {
        return static_cast<int>((bits_ >> 31) & 0x1);
    }

    // Get the stored exponent (still biased)
    int exponentField() const {
        return static_cast<int>((bits_ >> 23) & 0xFF);
    }

    // Convert exponent to real value by removing bias (127)
    int unbiasedExponent() const {
        return exponentField() - 127;
    }

    // Get the fraction (mantissa bits)
    uint32_t fraction() const {
        return bits_ & 0x7FFFFF;
    }

    // Check if value is zero (+0 or -0)
    bool isZero() const {
        return (bits_ & 0x7FFFFFFF) == 0;
    }

    // Check if value is subnormal
    bool isSubnormal() const {
        return exponentField() == 0 && fraction() != 0;
    }

    // Check if value is infinity or NaN
    bool isInfOrNaN() const {
        return exponentField() == 0xFF;
    }

    // Format bits like: sign exponent fraction
    std::string formattedBits() const {
        std::bitset<32> bitset(bits_);
        std::string s = bitset.to_string();
        return s.substr(0, 1) + " " + s.substr(1, 8) + " " + s.substr(9, 23);
    }

private:
    float value_;
    uint32_t bits_;
};

/*
 * Prints how to use the program if input is wrong
 */
void printUsage(const std::string& programName) {
    std::cout << "usage:\n";
    std::cout << programName << " loop_bound loop_counter\n\n";
    std::cout << "loop_bound is a positive floating-point value\n";
    std::cout << "loop_counter is a positive floating-point value\n";
}

/*
 * Finds floor(log2(x)) using bit shifts
 * (used for handling subnormal numbers)
 */
int floorLog2Unsigned(uint32_t x) {
    int pos = -1;
    while (x != 0) {
        x >>= 1;
        ++pos;
    }
    return pos;
}

/*
 * Computes ceil(log2(value)) without using log2().
 *
 * Instead of calling a math function, this code looks at the float's
 * exponent and fraction bits directly. That follows the assignment rules
 * and keeps everything in bit form.
 */
int ceilLog2Float(float value) {
    FloatBits fb(value);

    if (fb.isZero()) {
        return std::numeric_limits<int>::min();
    }

    if (fb.isInfOrNaN()) {
        return std::numeric_limits<int>::max();
    }

    // Normal case
    if (!fb.isSubnormal()) {
        // If fraction = 0, the value is already an exact power of 2.
        // Otherwise, round up to the next exponent.
        return fb.unbiasedExponent() + (fb.fraction() == 0 ? 0 : 1);
    }

    // Subnormal case
    uint32_t frac = fb.fraction();
    int highestBit = floorLog2Unsigned(frac);
    bool exactPowerOfTwo = (frac & (frac - 1)) == 0;

    int floorLog = highestBit - 149;
    return floorLog + (exactPowerOfTwo ? 0 : 1);
}

/*
 * Builds the float value 2^exponent using raw bits.
 *
 * This avoids using pow() and also gives us the exact IEEE 754 bit pattern
 * needed for the program output.
 */
float makePowerOfTwo(int exponent) {
    uint32_t raw = 0;

    if (exponent > 127) {
        // Too big -> becomes infinity
        raw = 0x7F800000u;
    } else if (exponent >= -126) {
        // Normal float
        uint32_t expField = static_cast<uint32_t>(exponent + 127);
        raw = expField << 23;
    } else if (exponent >= -149) {
        // Subnormal float
        uint32_t fracBit = 1u << (exponent + 149);
        raw = fracBit;
    } else {
        // Too small -> becomes 0
        raw = 0u;
    }

    float result;
    std::memcpy(&result, &raw, sizeof(float));
    return result;
}

/*
 * Finds the smallest value where adding the increment
 * will no longer change the number.
 *
 * The code does this by reading the increment's exponent bits,
 * figuring out the needed threshold exponent, and then building
 * that power of two directly from raw IEEE 754 bits.
 *
 * The 24 comes from float precision:
 * 23 stored fraction bits plus 1 hidden leading bit.
 */

float computeOverflowThreshold(float increment) {
    int ceilLog = ceilLog2Float(increment);
    int thresholdExponent = 24 + ceilLog;
    return makePowerOfTwo(thresholdExponent);
}


/*
 * Checks if overflow (precision loss) can happen
 */

bool willOverflow(float loopBound, float loopCounter) {
    float threshold = computeOverflowThreshold(loopCounter);

    // If threshold is infinity, we'll never reach it
    if (FloatBits(threshold).isInfOrNaN()) {
        return false;
    }

    return loopBound >= threshold;
}

/*
 * Main program
 *
 * Steps:
 * 1. Check input
 * 2. Convert arguments to float
 * 3. Print bit representations
 * 4. Check for overflow
 * 5. Print result
 */

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printUsage(argv[0]);
        return 1;
    }

    float loopBound = 0.0f;
    float loopCounter = 0.0f;

    try {
        loopBound = std::stof(argv[1]);
        loopCounter = std::stof(argv[2]);
    } catch (const std::exception&) {
        printUsage(argv[0]);
        return 1;
    }


    // Only allow positive values

    if (loopBound <= 0.0f || loopCounter <= 0.0f) {
        printUsage(argv[0]);
        return 1;
    }

    FloatBits boundBits(loopBound);
    FloatBits counterBits(loopCounter);

    std::cout << "Loop bound:    " << boundBits.formattedBits() << "\n";
    std::cout << "Loop counter:  " << counterBits.formattedBits() << "\n\n";

    if (!willOverflow(loopBound, loopCounter)) {
        std::cout << "No overflow!\n";
        return 0;
    }

    float threshold = computeOverflowThreshold(loopCounter);
    FloatBits thresholdBits(threshold);

    std::cout << "Warning: Possible overflow!\n";
    std::cout << "Overflow threshold:\n";
    std::cout << threshold << "\n";
    std::cout << thresholdBits.formattedBits() << "\n";

    return 0;
}
