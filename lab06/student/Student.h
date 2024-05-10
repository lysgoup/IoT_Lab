class Student {	
public: 
    Student();
    Student(uint32_t id, string name);
    ~Student();

    uint32_t    getID()         const   { return mId; }
    uint32_t    getHealth()     const   { return mHealth; }
    uint32_t    getAbility()    const   { return mAbility; }
    string      getName()       const   { return mName; }
    vector<uint32_t> getFriends() const { return mFriends; }
       
    void        Eat();
    void        Study();
    void        Sleep();
    void        Play();
    void        AddFriend(uint32_t friendId);
    void        TestBits();
    void        Print();

    void        Write(OutputMemoryStream& inStream)     const;
    void        Read(InputMemoryStream& inStream);

    void        Send(TCPSocketPtr socket);
    void        Receive(TCPSocketPtr socket);

private:
    uint32_t            mId;
    uint32_t            mHealth;
    uint32_t            mAbility;     
    string              mName;
    vector<uint32_t>    mFriends;
    
    // Lab#4 
    uint8_t             mBitData1;      // 5bits 
    uint8_t             mBitData2;      // 6bits
    uint8_t             mBitData3;      // 8bits
};
