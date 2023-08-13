#ifndef LIST_HPP
#define LIST_HPP

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <cstring>

class List {
public:	
	List(uint64_t stride, uint64_t capacity = 1, float expandRatio = 2)
		: mCapacityByte(capacity * stride),
		mSizeByte(0),
		mExpandRatio(expandRatio)
	{
		assert(stride > 0);
		assert(capacity >= 1);
		assert(expandRatio >= 0.0f);

		mArr = new uint8_t[mCapacityByte];
	}

	template<typename T>
	List(const T* copyArr, uint64_t copyLen, float expandRatio = 2)
		: mCapacityByte(copyLen * sizeof(T)),
		mSizeByte(0),
		mExpandRatio(expandRatio) 
	{
		assert(copyArr != nullptr);

		memcpy(mArr, copyArr, copyLen * sizeof(T));
	}

	~List() {
		if (mArr != nullptr) {
			delete[] mArr;
			mArr = nullptr;
		}
	}

	template <typename T>
	void Add(const T& value) {
		assert(mStride == sizeof(T));

		if (mSizeByte == mCapacityByte) {
			uint64_t newCapacityByte = static_cast<uint64_t>(mCapacityByte * (1 + mExpandRatio));
			newCapacityByte = newCapacityByte == mCapacityByte ? newCapacityByte + 1 * mStride : newCapacityByte;

			uint8_t* newArr = new uint8_t[newCapacityByte];
			memcpy(newArr, mArr, mSizeByte);

			delete[] mArr;
			mArr = newArr;
			mCapacityByte = newCapacityByte;
		}

		memcpy(mArr + mSizeByte, &value, mStride);
		mSizeByte += mStride;
	}

	template <typename T>
	void Add(const T* arr, uint64_t copyLen) {
		assert(copyLen > 0);
		assert(arr != nullptr);
		assert(sizeof(T) == mStride);

		uint64_t neededCapacityByte = mSizeByte + mStride * copyLen;
		if (mCapacityByte < neededCapacityByte) {
			uint64_t newCapacityByte = mCapacityByte;
			for (; newCapacityByte < neededCapacityByte; newCapacityByte *= (1 + mExpandRatio));

			uint8_t* newArr = new uint8_t[newCapacityByte];
			memcpy(newArr, mArr, mSizeByte);

			delete mArr;
			mArr = newArr;
			mCapacityByte = newCapacityByte;
		}

		memcpy(mArr + mSizeByte, arr, mStride * copyLen);
		mSizeByte += mStride * copyLen;
	}

	bool Remove(uint64_t index) {
		if (index * mStride >= mSizeByte) {
			return false;
		}

		for (uint64_t i = index * mStride; i < mSizeByte - mStride; i += mStride) {
			memcpy(mArr + i, mArr + i + mStride, mStride);
		}

		return true;
	}

	void Reset(uint64_t newStride) {
		assert(newStride > 0);


		mStride = newStride;
		mSizeByte = 0;
	}

	inline uint64_t GetSize() const {
		return mSizeByte / mStride;
	}
	inline uint64_t GetStride() const {
		return mStride;
	}

	template<typename T>
	inline T& At(uint64_t index) const {
		assert(sizeof(T) == mStride);
		assert(index * mStride < mSizeByte);

		return reinterpret_cast<T*>(mArr)[index];
	}

private:
	uint64_t mCapacityByte = 0;
	const float mExpandRatio = 0;
	uint8_t* mArr;
	uint64_t mSizeByte = 0;
	uint64_t mStride = 0;
};

#endif
