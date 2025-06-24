#include "SocketModule.hpp"

int main() {
    SocketModule sm;
    std::unordered_map<std::string, std::string> msg;
    msg.reserve(1);
    int i = 0;
    while(true){
        if(i > 10)
        {
            break;
        }
        sm.waitForConnection(8080);
        msg = sm.receiveMsg();
        printMsg(msg);
        sm.sendMsg(msg);
        std::cout<< sm.getSocketFd() << " " << sm.getConnectionFd() << std::endl;
        sm.closeConnection();
        i++;
    }    
}