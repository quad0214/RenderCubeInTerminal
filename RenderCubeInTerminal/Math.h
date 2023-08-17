#pragma once
#include <memory>
#include "Constants.h"
#include <iostream>
#include <cstdint>
#include <immintrin.h>
#include <intrin.h>
#include <cassert>

#define min(a, b) (a > b ? b : a)
#define max(a, b) (a > b ? a : b)

class Math {
public:
    static constexpr float Rad2Deg(float radian) { return radian * 180.0f / Constants::PI; }
    static constexpr float Deg2Rad(float degree) { return degree * Constants::PI / 180.0f; }            
};

template<int INT, int FRAC>
struct DoubleFixedPoint;
template<int INT, int FRAC>
struct FixedPoint {
    // max frac + int = 32
    static constexpr uint8_t FRAC_BITS_LEN = FRAC;
    static constexpr uint8_t INT_BITS_LEN = INT;
    static constexpr uint32_t BITS_LEN = INT_BITS_LEN + FRAC_BITS_LEN;
    static const FixedPoint ZERO;
    static const FixedPoint ONE;

    union {
        int32_t raw : INT_BITS_LEN + FRAC_BITS_LEN;
        struct {
            uint32_t fracValue : FRAC_BITS_LEN;
            int32_t intValue : INT_BITS_LEN;            
        };
    };

    FixedPoint() : raw(0) {}

    FixedPoint(float value) : raw(static_cast<int>(value * (1 << FRAC_BITS_LEN)))
    {
    }    

    inline int Floor() const {
        return intValue;
    }

    inline int Ceil() const {
        if (IsInt()) {
            return intValue;
        }

        return intValue + 1;
    }

    inline bool IsInt() const {
        return fracValue == 0;
    }

    inline bool IsNegative() const {
        return raw < 0;
    }

    // TODO : change rhs type to FixedPoint?
    //          reference size is always bigger than this type
    inline FixedPoint operator-(const FixedPoint& rhs) const {
        FixedPoint res;
        res.raw = raw - rhs.raw;
        return res;
    }

    inline FixedPoint operator+(const FixedPoint& rhs) const {
        FixedPoint res;
        res.raw = raw + rhs.raw;
        return res;
    }

    inline FixedPoint operator*(const FixedPoint& rhs) const {
        uint64_t mulBits = static_cast<int64_t>(raw) * rhs.raw;
        mulBits = mulBits >> FRAC_BITS_LEN;
        
        FixedPoint res;
        res.raw = mulBits;

        return res;        
    }

    inline FixedPoint operator/(const FixedPoint& rhs) const {
        uint64_t divBits = (static_cast<int64_t>(raw) << FRAC_BITS_LEN) / rhs.raw;

        FixedPoint res;
        res.raw = divBits;

        return res;
    }

    inline void operator+=(const FixedPoint& rhs) {        
        raw += rhs.raw;
    }

    inline std::strong_ordering operator<=>(const FixedPoint& rhs) const {
        return raw <=> rhs.raw;
    }

    inline bool operator==(const FixedPoint& rhs) const {
        return raw == rhs.raw;
    }

    inline bool operator!=(const FixedPoint& rhs) const {
        return raw != rhs.raw;
    }

    template<int CONV_INT, int CONV_FRAC>
    FixedPoint<CONV_INT, CONV_FRAC> Convert() {        
        int convFracMask = 1 << CONV_FRAC - 1;
        int convIntMask = 1 << (CONV_INT + CONV_FRAC) - 1;
        convIntMask &= ~convFracMask;
        
        int fracMask = 1 << FRAC_BITS_LEN - 1;
        int intMask = 1 << BITS_LEN - 1;
        intMask &= ~fracMask;

        int fracPart = fracValue;
        int intPart = intValue;

        FixedPoint<CONV_INT, CONV_FRAC> res;
        res.raw = 0;
        if (CONV_FRAC > FRAC_BITS_LEN) {
            int leftShiftNum = CONV_FRAC - FRAC_BITS_LEN;
            res.raw |= (fracPart << leftShiftNum) & convFracMask;
        }
        else {
            int rightShiftNum = FRAC_BITS_LEN - CONV_FRAC;
            res.raw |= (fracPart >> rightShiftNum) & convFracMask;
        }

        res.raw |= (intPart << CONV_INT) & convIntMask;

        return res;
    }

    DoubleFixedPoint<INT*2, FRAC*2> ToDoubleFixedPoint();


    float ToFloat() const {
        return static_cast<float>(intValue) + static_cast<float>(fracValue) / (1 << FRAC_BITS_LEN);
    }    

    friend std::ostream& operator<<(std::ostream& lhs, const FixedPoint& rhs) {
        lhs << rhs.ToFloat();
        return lhs;

    }
        
};

template <int INT, int FRAC>
struct DoubleFixedPoint {
    // max frac + int = 64
    // max length of integer, fraction bits is 32
    static constexpr uint8_t FRAC_BITS_LEN = FRAC;
    static constexpr uint8_t INT_BITS_LEN = INT;
    static constexpr uint8_t BITS_LEN = INT_BITS_LEN + FRAC_BITS_LEN;
    static const DoubleFixedPoint ZERO;
    static const DoubleFixedPoint ONE;

    union {
        int64_t raw : INT_BITS_LEN + FRAC_BITS_LEN;
        struct {
            uint64_t fracValue : FRAC_BITS_LEN;
            int64_t intValue : INT_BITS_LEN;
        };
    };

    DoubleFixedPoint() : raw(0) {}

    DoubleFixedPoint(double value) : raw(static_cast<int64_t>(value * (1ULL << FRAC_BITS_LEN)))
    {
    }

    inline int Floor() const {
        return intValue;
    }

    inline int Ceil() const {
        if (IsInt()) {
            return intValue;
        }

        return intValue + 1;
    }

    inline bool IsInt() const {
        return fracValue == 0;
    }

    inline bool IsNegative() const {
        return raw < 0;
    }

    // TODO : change rhs type to FixedPoint?
    //          reference size is always bigger than this type
    inline DoubleFixedPoint operator-(const DoubleFixedPoint& rhs) const {
        DoubleFixedPoint res;
        res.raw = raw - rhs.raw;
        return res;
    }

    inline DoubleFixedPoint operator+(const DoubleFixedPoint& rhs) const {
        DoubleFixedPoint res;
        res.raw = raw + rhs.raw;
        return res;
    }

    inline DoubleFixedPoint operator*(const DoubleFixedPoint& rhs) const {
        int64_t lowBits;
        int64_t highBits;
        lowBits = _mul128(raw, rhs.raw, &highBits);                

        DoubleFixedPoint res;
        res.raw = __shiftright128(lowBits, highBits, FRAC_BITS_LEN);;

        return res;
    }

    inline DoubleFixedPoint operator/(const DoubleFixedPoint& rhs) const {
        int64_t lowBits = raw;
        int64_t highBits = 0;
        
        highBits = __shiftleft128(lowBits, highBits, FRAC_BITS_LEN);
        // for sign extension
        // TODO : int, frac bit가 작으면 
        //        음수임에도 highBits가 0이라 시프트해도 부호 표시가 안 되는 거 아닌가?
        highBits = highBits << (64 - FRAC_BITS_LEN);
        highBits = highBits >> (64 - FRAC_BITS_LEN);
        lowBits = lowBits << FRAC_BITS_LEN;



        DoubleFixedPoint res;
        res.raw = _div128(highBits, lowBits, rhs.raw, nullptr);

        return res;
    }

    inline void operator+=(const DoubleFixedPoint& rhs) {
        raw += rhs.raw;
    }

    inline std::strong_ordering operator<=>(const DoubleFixedPoint& rhs) const {
        return raw <=> rhs.raw;
    }

    inline bool operator==(const DoubleFixedPoint& rhs) const {
        return raw == rhs.raw;
    }

    inline bool operator!=(const DoubleFixedPoint& rhs) const {
        return raw != rhs.raw;
    }

    //template<int CONV_INT, int CONV_FRAC>
    //DoubleFixedPoint<CONV_INT, CONV_FRAC> Convert() {
    //    uint64_t convFracMask = 1 << CONV_FRAC - 1;
    //    uint64_t convIntMask = 1 << (CONV_INT + CONV_FRAC) - 1;
    //    convIntMask &= ~convFracMask;

    //    uint64_t fracMask = 1 << FRAC_BITS_LEN - 1;
    //    uint64_t intMask = 1 << BITS_LEN - 1;
    //    intMask &= ~fracMask;

    //    uint64_t fracPart = raw & fracMask;
    //    uint64_t intPart = raw & intMask;

    //    DoubleFixedPoint<CONV_INT, CONV_FRAC> res;
    //    res.raw = 0;
    //    res.raw |= fracPart & convFracMask;
    //    if (CONV_FRAC > FRAC_BITS_LEN) {
    //        int leftShiftNum = CONV_FRAC - FRAC_BITS_LEN;
    //        res.raw |= (intPart << leftShiftNum) & convIntMask;
    //    }
    //    else {
    //        int rightShiftNum = CONV_FRAC - FRAC_BITS_LEN;
    //        res.raw |= (intPart >> rightShiftNum) & convIntMask;
    //    }

    //    return res;
    //}

    FixedPoint<INT/2, FRAC/2> ToFixedPoint() const;

    double ToDouble() const {
        return static_cast<double>(intValue) + static_cast<double>(fracValue) / (1ULL << FRAC_BITS_LEN);
    }

    friend std::ostream& operator<<(std::ostream& lhs, const DoubleFixedPoint& rhs) {
        lhs << rhs.ToDouble();
        return lhs;

    }

};


template <int INT, int FRAC>
FixedPoint<INT/2, FRAC/2> DoubleFixedPoint<INT, FRAC>::ToFixedPoint() const {
    typedef FixedPoint<INT / 2, FRAC / 2> FP;

    uint64_t convFracMask = (1ULL << FP::FRAC_BITS_LEN) - 1;
    uint64_t convIntMask = (1ULL << FP::BITS_LEN) - 1;
    convIntMask &= ~convFracMask;

    uint64_t fracPart = static_cast<uint64_t>(fracValue);
    int64_t intPart = static_cast<int64_t>(intValue);

    FixedPoint res;
    res.raw = 0;
    if (FRAC_BITS_LEN < FP::FRAC_BITS_LEN) {
        int leftShiftNum = FP::FRAC_BITS_LEN - FRAC_BITS_LEN;
        res.raw |= (fracPart << leftShiftNum) & convFracMask;
    }
    else {
        int rightShiftNum = FRAC_BITS_LEN - FP::FRAC_BITS_LEN;
        res.raw |= (fracPart >> rightShiftNum) & convFracMask;
    }

    res.raw |= (intPart << FP::FRAC_BITS_LEN) & convIntMask;

    return res;
}

template<int INT, int FRAC>
DoubleFixedPoint<INT*2, FRAC*2> FixedPoint<INT, FRAC>::ToDoubleFixedPoint() {
    typedef DoubleFixedPoint<INT * 2, FRAC * 2> DF;

    uint64_t convFracMask = (1ULL << DF::FRAC_BITS_LEN) - 1;
    uint64_t convIntMask = (1ULL << DF::BITS_LEN)-1;
    convIntMask &= ~convFracMask;

    uint64_t fracPart = static_cast<uint64_t>(fracValue);    
    int64_t intPart = static_cast<int64_t>(intValue);

    DF res;
    res.raw = 0;        
    if (FRAC_BITS_LEN < DF::FRAC_BITS_LEN) {
        int leftShiftNum = DF::FRAC_BITS_LEN - FRAC_BITS_LEN;
        res.raw |= (fracPart << leftShiftNum) & convFracMask;
    }
    else {
        int rightShiftNum = FRAC_BITS_LEN - DF::FRAC_BITS_LEN;
        res.raw |= (fracPart >> rightShiftNum) & convFracMask;     
    }

    res.raw |= (intPart << DF::FRAC_BITS_LEN) & convIntMask;

    return res;
}

template<int INT, int FRAC>
const FixedPoint<INT, FRAC> FixedPoint<INT, FRAC>::ZERO = { 0 };
template<int INT, int FRAC>
const FixedPoint<INT, FRAC> FixedPoint<INT, FRAC>::ONE = { 1.0f };
template<int INT, int FRAC>
const DoubleFixedPoint<INT, FRAC> DoubleFixedPoint<INT, FRAC>::ZERO = { 0 };
template<int INT, int FRAC>
const DoubleFixedPoint<INT, FRAC> DoubleFixedPoint<INT, FRAC>::ONE = { 1.0f };

//typedef FixedPoint<2, 22> SNORM;
typedef FixedPoint<16, 8> FP;
typedef DoubleFixedPoint<FP::INT_BITS_LEN * 2, FP::FRAC_BITS_LEN * 2> DF;
typedef FixedPoint<8, 16> SNORM;

//struct SNORM {    
//    static constexpr int32_t MAX_RAW{ (1 << (32 - 1)) - 1};
//    static constexpr int32_t MIN_RAW{ -(1 << (32 - 1)) };
//    static const SNORM MIN;
//    static const SNORM MAX;
//    static const SNORM ZERO;
//
//    int32_t raw;
//
//    SNORM() : raw(0) {}
//
//    SNORM(float value) {
//        if (value <= -1.0f) {
//            raw = MIN_RAW;
//        }
//        else if (value >= 1.0f) {
//            raw = MAX_RAW;
//        }
//        else {
//            float temp = value * MAX_RAW;
//            if (temp >= 0) {
//                raw = static_cast<int32_t>(temp + 0.5f);
//            }
//            else {
//                raw = static_cast<int32_t>(temp - 0.5f);
//            }
//        }
//    }
//
//    SNORM(double value) {
//        if (value <= -1.0f) {
//            raw = MIN_RAW;
//        }
//        else if (value >= 1.0f) {
//            raw = MAX_RAW;
//        }
//        else {
//            double temp = value * MAX_RAW;
//            if (temp >= 0) {
//                raw = static_cast<int32_t>(temp + 0.5f);
//            }
//            else {
//                raw = static_cast<int32_t>(temp - 0.5f);
//            }
//        }
//    }
//
//    inline SNORM operator*(const int32_t rhs) const {
//        int64_t mulBits = raw * rhs;
//        if (mulBits > MAX_RAW) {
//            return MAX;
//        }
//        else if (mulBits < MIN_RAW) {
//            return MIN;
//        }
//
//        SNORM res;
//        res.raw = static_cast<int32_t>(mulBits);
//
//        return res;        
//    }
//
//    inline SNORM operator+(const SNORM rhs) const {
//        // TODO : remove. test overflow                 
//        //assert(raw >= 0 && rhs.raw <= INT_MAX - raw
//        //    || raw < 0 && rhs.raw >= INT_MIN - raw);
//
//        SNORM res;
//        res.raw = raw + rhs.raw;                    
//
//        return res;
//    }
//
//    inline SNORM operator-(const SNORM rhs) const {
//        // TODO : remove. test overflow                
//        //assert(
//        //    (raw >= 0 && rhs.raw >= INT_MIN + raw)
//        //    || (raw < 0 && rhs.raw <= INT_MAX + raw));
//
//        SNORM res;
//        res.raw = raw - rhs.raw;
//        
//        return res;
//    }
//
//    inline void operator+=(const SNORM rhs) {
//        raw += rhs.raw;
//    }
//
//    inline std::strong_ordering operator<=>(const SNORM rhs) const {
//        return raw <=> rhs.raw;
//    }
//
//    inline bool operator==(const SNORM rhs) const {
//        return raw == rhs.raw;
//    }
//
//    inline bool operator!=(const SNORM rhs) const {
//        return raw != rhs.raw;
//    }
//
//    inline float ToFloat() {
//        return max(static_cast<float>(raw) / MAX_RAW, -1.0f);
//    }
//};



struct Vec3 {
    float x;
    float y;
    float z;

    Vec3(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Vec3 operator+(const Vec3& rhs) const {
        return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    Vec3 operator-(const Vec3& rhs) const {
        return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    // static
    static inline Vec3 Cross(const Vec3& v1, const Vec3& v2) { return Vec3(v1.y * v2.z - v2.y * v1.z, v2.x * v1.z - v1.x * v2.z, v1.x * v2.y - v2.x * v1.y); }    
    static inline float Dot(const Vec3& v1, const Vec3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

    static const Vec3 ZERO;
};

struct Vec4 {
    float x;
    float y;
    float z;
    float w;

    Vec4(float x, float y, float z, float w) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

    Vec4 operator+(const Vec4& rhs) const {
        return Vec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
    }

    Vec4 operator-(const Vec4& rhs) const {
        return Vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
    }

    Vec4 operator*(const float value) const {
        return Vec4(x * value, y * value, z * value, w * value);
    }

    friend std::ostream& operator<<(std::ostream& lhs, const Vec4& rhs) {
        lhs << "(" << rhs.x << "," << rhs.y << "," << rhs.z  << "," << rhs.w << ")";
        return lhs;
        
    }

    // static
    static inline float Dot(const Vec4& v1, const Vec4& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }
    static inline Vec4 Lerp(const Vec4& v1, const Vec4& v2, float t) { return v1 * (1.0f - t) + v2 * (t); }
    
    static const Vec4 ZERO;
};

struct FixedVec4 {
    FP x;
    FP y;
    FP z;
    FP w;

    inline FixedVec4(FP x, FP y, FP z, FP w) : x(x), y(y), z(z), w(w) {
    }

    inline FixedVec4 operator+(const FixedVec4& rhs) const {
        return FixedVec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
    }

    inline FixedVec4 operator-(const FixedVec4& rhs) const {
        return FixedVec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
    }

    inline FixedVec4 operator*(const FP& value) const {
        return FixedVec4(x * value, y * value, z * value, w * value);
    }

    inline Vec4 ToVec4() const {
        return Vec4(x.ToFloat(), y.ToFloat(), z.ToFloat(), w.ToFloat());
    }

    friend std::ostream& operator<<(std::ostream& lhs, const FixedVec4& rhs) {
        lhs << "(" << rhs.x.ToFloat() << "," << rhs.y.ToFloat() << "," << rhs.z.ToFloat() << "," << rhs.w.ToFloat() << ")";
        return lhs;

    }

    // static
    static inline FP Dot(const FixedVec4& v1, const FixedVec4& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }
    static inline FixedVec4 Lerp(const FixedVec4& v1, const FixedVec4& v2, FP t) { return v1 * (FP::ONE - t) + v2 * (t); }    

    static const FixedVec4 ZERO;
};

struct Mat4x4 {
    union {
        float e[16];
        float m[4][4];
        struct {
            float m00; float m01; float m02; float m03;
            float m10; float m11; float m12; float m13;
            float m20; float m21; float m22; float m23;
            float m30; float m31; float m32; float m33;
        };
        struct {
            Vec4 row0;
            Vec4 row1;
            Vec4 row2;
            Vec4 row3;
        };
    };

    Mat4x4(float e[16]) {
        memcpy(this->e, e, sizeof(float) * 16);
    }

    Mat4x4(Vec4 r0, Vec4 r1, Vec4 r2, Vec4 r3) {
        row0 = r0;
        row1 = r1;
        row2 = r2;
        row3 = r3;
    }
};