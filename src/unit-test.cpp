#include "unit-test.hpp"

// CycleCounter tests
/* This has to be commented out because of VirtualBox
TEST(CycleCounterTest, BasicCounting) {
    CycleCounter cc;
    cc.start();
    cc.stop();
    EXPECT_GE(cc.cycles(), 0);
}

TEST(CycleCounterTest, Reset) {
    CycleCounter cc;
    cc.start();
    cc.stop();
    cc.reset();
    EXPECT_EQ(cc.cycles(), 0);
}*/

// puf tests
TEST(PufTest, GenerateAndVerify) {
    puf p;
    std::string challenge = "test_challenge";
    std::string response = (challenge);
    EXPECT_TRUE(p.puf_verify(challenge, response));
}

TEST(PufTest, WrongResponse) {
    puf p;
    std::string challenge = "test_challenge";
    std::string response = "wrong_response";
    EXPECT_FALSE(p.puf_verify(challenge, response));
}

// SocketModule tests
TEST(SocketModuleTest, OpenCloseSocket) {
    SocketModule sm;
    EXPECT_TRUE(sm.initiateConnection("127.0.0.1", 8080));
    EXPECT_TRUE(sm.isOpen());
    sm.closeConnection();
    EXPECT_FALSE(sm.isOpen());
}

TEST(SocketModuleTest, SendReceiveMessage) {
    SocketModule sm;
    EXPECT_TRUE(sm.initiateConnection("127.0.0.1", 8080));
    std::unordered_map<std::string, std::string> msg = {{"key", "value"}}; 
    sm.sendMsgPack(msg);
    std::unordered_map<std::string, std::string> receivedMsg = sm.receiveMsgPack();
    EXPECT_EQ(msg["key"], receivedMsg["key"]);
    EXPECT_TRUE(sm.isOpen());
    sm.closeConnection();
}

// UAV tests
TEST(UAVTest, Initialization) {
    UAV uav("test_uav");
    EXPECT_EQ(uav.getId(), "test_uav");
}

TEST(UAVTest, AddAndRetrieveUAVData) {
    UAV uav("test_uav");
    unsigned char x[32] = {0};
    unsigned char c[32] = {0};
    unsigned char r[32] = {0};
    unsigned char xLock[32] = {0};
    unsigned char secret[32] = {0};
    uav.addUAV("neighbour_uav", x, c, r, xLock, secret);
    UAVData* data = uav.getUAVData("neighbour_uav");
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->getX(), x);
    EXPECT_EQ(data->getC(), c);
    EXPECT_EQ(data->getR(), r);
    EXPECT_EQ(data->getXLock(), xLock);
    EXPECT_EQ(data->getSecret(), secret);
    uav.removeUAV("neighbour_uav");
}

TEST(UAVTest, CallPUF) {
    UAV uav("test_uav");
    unsigned char input[32] = {0};
    unsigned char response[32] = {0};
    uav.callPUF(input, response);
    EXPECT_NE(response, nullptr);
}

// utils tests
TEST(UtilsTest, GenerateRandomBytes) {
    unsigned char buffer[32];
    generate_random_bytes(buffer, sizeof(buffer));
    EXPECT_EQ(sizeof(buffer), 32);
    for (size_t i = 0; i < sizeof(buffer); ++i) {
        EXPECT_GE(buffer[i], 0);
        EXPECT_LE(buffer[i], 255);
    }
}

TEST(UtilsTest, XorBuffers) {
    unsigned char input1[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    unsigned char input2[32] = {0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01};
    unsigned char output[32];
    xor_buffers(input1, input2, sizeof(input1), output);
    for (size_t i = 0; i < sizeof(output); ++i) {
        EXPECT_EQ(output[i], input1[i] ^ input2[i]);
    }
}

TEST(UtilsTest, DeriveHKDF) {
    unsigned char NA[32] =                  {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                            0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
                                            0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                            0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};
    unsigned char NB[32] =                  {0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
                                            0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
                                            0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
                                            0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40};
    unsigned char salt[32] =               {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                            0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                                            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                            0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
    unsigned char derived_key[64];
    deriveKeyUsingHKDF(NA, NB, salt, sizeof(derived_key), derived_key);
    EXPECT_EQ(sizeof(derived_key), 64);
    for (size_t i = 0; i < sizeof(derived_key); ++i) {
        EXPECT_GE(derived_key[i], 0);
        EXPECT_LE(derived_key[i], 255);
    }
}

TEST(UtilsTest, ExtractValueFromMap) {
    std::unordered_map<std::string, std::string> map = {{"key1", "value1"}, {"key2", "value2"}};
    unsigned char output[32];
    bool result = extractValueFromMap(map, "key1", output, sizeof(output));
    EXPECT_TRUE(result);
    EXPECT_EQ(std::string(reinterpret_cast<const char*>(output), 32), "value1");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}