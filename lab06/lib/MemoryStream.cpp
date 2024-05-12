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
	uint16_t resultBitHead = mBitHead + outBitCount;
	uint32_t resultHead = mHead;
	if(resultBitHead > 7){
		resultBitHead = resultBitHead%8;
		resultHead++;
		if (resultHead > mCapacity)
		{
			ReallocBuffer(std::max(mCapacity * 2, resultHead));
		}

		uint8_t first_seg_len = 8-mBitHead;
		uint8_t second_seg_len = outBitCount-first_seg_len;
		uint8_t mask = (1 << first_seg_len)-1;
		uint8_t write_data = (outData & mask) << mBitHead;
		*(mBuffer+mHead) = ((*(mBuffer+mHead)) | write_data);

		write_data = (outData & ~mask) >> first_seg_len;
		*(mBuffer+resultHead) = write_data;
		printf("write_data : %u\n",write_data);
	}
	else{
		uint8_t write_data = (outData << mBitHead);
		*(mBuffer+mHead) = ((*(mBuffer+mHead)) | write_data);
		printf("write_data : %u\n",write_data);
	}
	printf("1st: %d\n",*(mBuffer+mHead));
	mBitHead = (uint8_t)resultBitHead;
	mHead = resultHead;
	printf("2nd: %d\n",*(mBuffer+mHead));
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
	uint16_t resultBitHead = mBitHead + inBitCount;
	uint32_t resultHead = mHead;
	if(resultBitHead > 7){
		resultBitHead = resultBitHead%8;
		resultHead++;

		uint8_t first_seg_len = 8-mBitHead;
		uint8_t second_seg_len = inBitCount-first_seg_len;
		uint8_t read_data = (*(mBuffer+mHead) & ~((1 << mBitHead)-1)) >> mBitHead;
		read_data = ((*(mBuffer+resultHead) & ((1<<second_seg_len)-1)) << first_seg_len) | read_data;
		inData = read_data;
	}
	else{
		uint8_t mask = ((1 << (mBitHead + inBitCount))-1) & ~((1 << mBitHead)-1);
		uint8_t read_data = (*(mBuffer+mHead) & mask) >> mBitHead;
		inData = read_data;
	}
	printf("resultBitHead: %d\n",resultBitHead);
	printf("resultHead: %d\n",resultHead);
	printf("1st: %d\n",*(mBuffer+mHead));
	mBitHead = (uint8_t)resultBitHead;
	mHead = resultHead;
	printf("2nd: %d\n",*(mBuffer+mHead));
	printf("read_data : %u\n", inData);
}