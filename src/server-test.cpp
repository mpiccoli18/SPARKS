#include "SocketModule.hpp"

int main(int argc, char **argv) {
    SocketModule sm;
    std::unordered_map<std::string, std::string> msg;
    msg.reserve(1);
    while(true){
        sm.waitForConnection(8080);
        msg = sm.receiveMsg();
        printMsg(msg);
        sm.sendMsg(msg);
        sm.closeConnection();
    }    
}