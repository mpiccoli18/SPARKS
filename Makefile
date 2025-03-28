# Compiler
CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++11 -g

# Directories
SRC_DIR := src
BIN_DIR := bin

# Files
CPPS := $(SRC_DIR)/UAV.cpp $(SRC_DIR)/puf.cpp $(SRC_DIR)/utils.cpp $(SRC_DIR)/SocketModule.cpp 
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BIN_DIR)/%.o, $(CPPS))

# Default target (currently does nothing)
all:
	@echo "Run 'make test' to compile the test executable."

# Rule to compile the test program
test: tests

tests : $(OBJS) $(SRC_DIR)/test.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lcrypto

# Rule to compile measurement
measure : measure_client measure_server

measure_client : $(OBJS) $(SRC_DIR)/measurement/enrol_client.cpp | $(BIN_DIR)
	$(CXX) -Wall -Wextra -O0 -std=c++11 -g $^ -o $@ -lcrypto

measure_server : $(OBJS) $(SRC_DIR)/measurement/enrol_server.cpp | $(BIN_DIR)
	$(CXX) -Wall -Wextra -O0 -std=c++11 -g $^ -o $@ -lcrypto

# Rules to compile the attack scenarios
attack: attack_client attack_server

attack_client: $(OBJS) $(SRC_DIR)/attack_test/attack_client.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lcrypto

attack_server: $(OBJS) $(SRC_DIR)/attack_test/attack_server.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lcrypto

# Rule to compile the scenarios
scenarii: scenario1 scenario2 scenario3

scenario1: scenario1_client scenario1_server

scenario2: scenario2_initial scenario2_base_station scenario2_supplementary

scenario3: scenario3_client scenario3_server

scenario1_client: $(OBJS) $(SRC_DIR)/scenario1/scenario1_client.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lcrypto

scenario1_server: $(OBJS) $(SRC_DIR)/scenario1/scenario1_server.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lcrypto

scenario2_initial: $(OBJS) $(SRC_DIR)/scenario2/scenario2_initial.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lcrypto

scenario2_base_station: $(OBJS) $(SRC_DIR)/scenario2/scenario2_base_station.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lcrypto

scenario2_supplementary: $(OBJS) $(SRC_DIR)/scenario2/scenario2_supplementary.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lcrypto

scenario3_client: $(OBJS) $(SRC_DIR)/scenario3/scenario3_client.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lcrypto

scenario3_server: $(OBJS) $(SRC_DIR)/scenario3/scenario3_server.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lcrypto

# scenario2 : $(OBJS) $(SRC_DIR)/scenario2.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -lcrypto

# scenario3 : $(OBJS) $(SRC_DIR)/scenario3.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -lcrypto

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Ensure bin/ directory exists
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean up all compiled files
clean:
	rm -f $(BIN_DIR)/*.o tests scenario* attack*
