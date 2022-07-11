#pragma once
#include <limits>
#include <algorithm> // for int16_t
#include <cmath>


#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

// The conversion functions use the following naming convention:
// Int16:      int16_t [-32768, 32767]
// Float64:	   double  [-1.0, 1.0]
// Float32:    float   [-1.0, 1.0]
// FloatS16:   float   [-32768.0, 32767.0]
// Dbfs: float [-20.0*log(10, 32768), 0] = [-90.3, 0]
// The ratio conversion functions use this naming convention:
// Ratio: float (0, +inf)
// Db: float (-inf, +inf)
class KuAudioUtil
{
public:

	static inline int16_t float64ToInt16(double v) {
		v *= 32768.f;
		v = std::min(v, 32767.);
		v = std::max(v, -32768.);
		return static_cast<int16_t>(v + std::copysign(0.5, v));
	}

	static inline int16_t float32ToInt16(float v) {
		return float64ToInt16(v);
	}

	static inline double int16ToFloat64(int16_t v) {
		constexpr double kScaling = 1. / 32768.;
		return v * kScaling;
	}

	static inline float int16ToFloat32(int16_t v) {
		return (float)int16ToFloat64(v);
	}

	static void float64ToInt16(const double* f64, int16_t* s16, size_t count) {
		for (size_t n = 0; n < count; n++)
			s16[n] = float64ToInt16(f64[n]);
	}

	static inline void float32ToInt16(const float* f32, int16_t* s16, size_t count) {
		for (size_t n = 0; n < count; n++)
			s16[n] = float32ToInt16(f32[n]);
	}

	static inline void int16ToFloat64(const int16_t* s16, double* f64, size_t count) {
		for (size_t n = 0; n < count; n++)
			f64[n] = int16ToFloat64(s16[n]);
	}

	static inline void int16ToFloat32(const int16_t* s16, float* f32, size_t count) {
		for (size_t n = 0; n < count; n++)
			f32[n] = int16ToFloat32(s16[n]);
	}

	static inline double floatToDbfs(double v) {
		// kMinDbfs is equal to -20.0 * log10(-limits_int16::min())
		static const double kMinDbfs = -20.0 * std::log10(32768.); //  -90.30899869919436f

		if (v <= 0)
			return kMinDbfs;

		return std::max(20.0 * std::log10(v), kMinDbfs);
	}

	static inline double int16ToDbfs(int16_t v) {
		return floatToDbfs(int16ToFloat64(v));
	}

private:
	KuAudioUtil() { }
	~KuAudioUtil() { }
};


