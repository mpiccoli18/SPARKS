#include <gtest/gtest.h>
#include "CycleCounter.hpp"
#include "puf.hpp"
#include "SocketModule.hpp"
#include "UAV.hpp"
#include "utils.hpp"

/*
// CycleCounter tests
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}