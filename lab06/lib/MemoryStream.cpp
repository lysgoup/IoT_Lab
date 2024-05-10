#include "HGUNet.h"

void OutputMemoryStream::ReallocBuffer(uint32_t inNewLength)
{
	mBuffer = static_cast< char* >( std::realloc( mBuffer, inNewLength ) );
	//handle realloc failure
	//...
	mCapacity = inNewLength;
}

void OutputMemoryStream::Write(const void* outData, size_t outByteCount)
{
	//make sure we have space...
	uint32_t resultHead = mHead + static_cast<uint32_t>(outByteCount);
	if (resultHead > mCapacity)
	{
		ReallocBuffer(std::max(mCapacity * 2, resultHead));
	}
	
	//copy into buffer at head
	std::memcpy(mBuffer + mHead, outData, outByteCount);
	
	//increment head for next write
	mHead = resultHead;
}

// Lab#6
void OutputMemoryStream::WriteBits(uint8_t outData, size_t outBitCount)
{
	// TODO
	
}	

void InputMemoryStream::Read(void* inData,
							 uint32_t inByteCount)
{
	uint32_t resultHead = mHead + inByteCount;
	if (resultHead > mCapacity)
	{
		//handle error, no data to read!
		//...
	}
	
	std::memcpy(inData, mBuffer + mHead, inByteCount);
	
	mHead = resultHead;
}


// Lab#6
void InputMemoryStream::ReadBits(uint8_t& inData, size_t inBitCount)
{
	// TODO
	
}