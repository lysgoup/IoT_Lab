#include <cstdlib>
#include <cstdint>
#include <type_traits>

#define STREAM_ENDIANNESS 0
#define PLATFORM_ENDIANNESS 0

#include "ByteSwap.h"

class OutputMemoryStream
{
public:
	OutputMemoryStream() : mBuffer(nullptr), mHead(0), mCapacity(0), mBitHead(0)	// Lab#6
							{ ReallocBuffer(32); }
	~OutputMemoryStream()	{ std::free(mBuffer); }
	
	//get a pointer to the data in the stream
	const 	char*		GetBufferPtr()	const	{ return mBuffer; }
	//uint32_t			GetLength()		const	{ return mHead; }
	uint32_t			GetLength()		const		// Lab#6
	{ 
		if (mBitHead == 0)
			return mHead; 
		else 
			return mHead + 1;
	}
	
	void				Write(const void* outData, size_t outByteCount);

	template<typename T> 
	void Write(const T outData)
	{
		static_assert(std::is_arithmetic<T>::value ||
					std::is_enum<T>::value,
					"Generic Write only supports primitive data types");
		
		if (STREAM_ENDIANNESS == PLATFORM_ENDIANNESS)
		{
			Write(&outData, sizeof(outData));
		}
		else
		{
			T swappedData = ByteSwap(outData);
			Write(&swappedData, sizeof(swappedData));
		}
	}

	void Write(const std::string& outString)
	{
		size_t elementCount = outString.size();
		Write(elementCount);
		Write(outString.data(), elementCount * sizeof(char));
	}

	template<typename T>
	void Write(const std::vector<T>& outVector)
	{
		size_t elementCount = outVector.size();
		Write(elementCount);
		for (const T& element : outVector)
		{
			Write(element);
		}
	}

	// Lab#6
	void 				WriteBits(uint8_t outData, size_t outBitCount);
	
private:
	void				ReallocBuffer(uint32_t inNewLength);
	char*				mBuffer;
	uint32_t			mHead;
	uint32_t			mCapacity;
	uint8_t				mBitHead;	// Lab#6
};

class InputMemoryStream
{
public:
	InputMemoryStream(char* inBuffer, uint32_t inByteCount) :
				mBuffer(inBuffer), mCapacity(inByteCount), mHead(0), mBitHead(0) {} 	// Lab#6
	~InputMemoryStream()	{ std::free( mBuffer ); }
		
	uint32_t			GetRemainingDataSize() const
							{ return mCapacity - mHead; }
	
	void				Read(void* inData, uint32_t inByteCount);

	template<typename T> void Read(T& inData)
	{
		static_assert(std::is_arithmetic<T>::value ||
					  std::is_enum<T>::value,
					  "Generic Read only supports primitive data types");
		Read((void*)&inData, sizeof(inData));
	}

	void Read(std::string& inString)
	{
		size_t elementCount;
		char *str; 
		Read(elementCount);
		str = (char*) malloc(sizeof(char)*elementCount);
		Read(str, elementCount);
		inString = str;
	}
	
	template<typename T>
	void Read(std::vector<T>& inVector)
	{
		size_t elementCount;
		Read(elementCount);
		inVector.resize(elementCount);
		for (T& element : inVector)
		{
			Read(element);
		}
	}

	// Lab#6
	void 				ReadBits(uint8_t& inData, size_t inBitCount);


private:
	char*		mBuffer;
	uint32_t	mHead;
	uint32_t	mCapacity;
	uint8_t		mBitHead;	// Lab#6
};

