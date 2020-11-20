#include "GMath.h"

static inline float clamp(float val, float min, float max) {
    return std::max(min, std::min(max, val));
}

static inline float manyMax(float* nums, int count) {
    assert(count >= 1);

    float max = nums[0];
    for (int i = 1; i < count; ++i) {
        max = std::max(max, nums[i]);
    }

    return max;
}

static inline float manyMin(float* nums, int count) {
    assert(count >= 1);

    float min = nums[0];
    for (int i = 1; i < count; ++i) {
        min = std::min(min, nums[i]);
    }

    return min;
}