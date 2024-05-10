#include "HGUNet.h"
#include "Student.h"

const uint32_t kMaxPacketSize = 1470;

Student::Student() 
{
    mId = 0; 
    mHealth = 0;
    mAbility = 0;
    mName = "";
}

Student::Student(uint32_t id, string name)
{
    mId = id;
    mHealth = 100;
    mAbility = 0;     
    mName = name;

    mBitData1 = 13; // Lab#6
    mBitData2 = 52;
    mBitData3 = 100;
}

Student::~Student()
{

}

void Student::Eat() 
{
    mHealth += 10; 
    printf("%s is eating \n", mName.c_str());
} 

void Student::Study()
{
    mHealth -= 5;
    mAbility += 10; 
    printf("%s is studying \n", mName.c_str());
}

void Student::Sleep()
{
    mHealth += 10; 
    printf("%s is sleeping \n", mName.c_str());
}

void Student::Play() 
{
    mHealth -= 3;
    mAbility += 3; 
    printf("%s is playing \n", mName.c_str());
}

void Student::AddFriend(uint32_t friendId) 
{
    printf("New friend %d is added\n", friendId);
    mFriends.push_back(friendId);
}

void Student::TestBits() 
{
    mBitData1 += 2;
    mBitData2 += 10;
    mBitData3 -= 5;
}

void Student::Print()
{
    printf("Name=%s / ID=%d / Health=%d / Ability=%d / Friends= ", 
        mName.c_str(), mId, mHealth, mAbility);
    for (auto id : mFriends)
        cout << id << ' '; 
    printf("/ mBitData1=%d / mBitData2=%d / mBitData3=%d \n", mBitData1, mBitData2, mBitData3);
}

void Student::Write(OutputMemoryStream& inStream)     const
{
    inStream.Write(mId);
    inStream.Write(mHealth);
    inStream.Write(mAbility);
    inStream.Write(mName);
    inStream.Write(mFriends);

    // Lab#6
    inStream.WriteBits(mBitData1, 5);
    inStream.WriteBits(mBitData2, 6);
    inStream.WriteBits(mBitData3, 8);    
}

void Student::Read(InputMemoryStream& inStream)
{
    inStream.Read(mId);
    inStream.Read(mHealth);
    inStream.Read(mAbility);
    inStream.Read(mName);
    inStream.Read(mFriends);

    // Lab#6
    inStream.ReadBits(mBitData1, 5);
    inStream.ReadBits(mBitData2, 6);
    inStream.ReadBits(mBitData3, 8);
}

void Student::Send(TCPSocketPtr socket) 
{
    OutputMemoryStream stream; 
    Write(stream);
    socket->Send(stream.GetBufferPtr(), stream.GetLength());
}

void Student::Receive(TCPSocketPtr socket)
{
    char* temporaryBuffer = static_cast<char*>(std::malloc(kMaxPacketSize)); 
    size_t receivedByteCount = socket->Receive(temporaryBuffer, kMaxPacketSize); 

    if (receivedByteCount > 0)
    {
        InputMemoryStream stream(temporaryBuffer, 
                                 static_cast<uint32_t>(receivedByteCount));
        Read(stream);
    }
    else 
    {
        std::free(temporaryBuffer);
    }
}
