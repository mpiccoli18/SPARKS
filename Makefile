# Compiler
CXX = g++
CXXFLAGS = -Wall -Wextra -O3 -std=c++11 -I/usr/include/crypto++ -I/home/sparks/Desktop/msgpack/include

# Directories
SRC_DIR := src
BIN_DIR := bin

# Files
CPPS := $(SRC_DIR)/UAV.cpp $(SRC_DIR)/puf.cpp $(SRC_DIR)/utils.cpp $(SRC_DIR)/SocketModule.cpp $(SRC_DIR)/CycleCounter.cpp 
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BIN_DIR)/%.o, $(CPPS))

# Default target
all: scenarii

# Rule to compile measurement
measure : test_pmc measure_enrol_client measure_enrol_server measure_auth_client measure_auth_server measure_auth_client_key measure_auth_server_key measure_enrol_RAM_client measure_enrol_RAM_server measure_auth_RAM_client measure_auth_RAM_server measure_warmup measure_supp_auth_initial measure_supp_auth_supplementary measure_json_client measure_json_server

measure_json_client : $(OBJS) $(SRC_DIR)/measurement/json_impact_client.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

measure_json_server : $(OBJS) $(SRC_DIR)/measurement/json_impact_server.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

measure_supp_auth_supplementary : $(OBJS) $(SRC_DIR)/measurement/supp_auth_supplementary.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

measure_supp_auth_initial : $(OBJS) $(SRC_DIR)/measurement/supp_auth_initial.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

measure_warmup : $(OBJS) $(SRC_DIR)/measurement/warmup_impact.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

measure_auth_RAM_client : $(OBJS) $(SRC_DIR)/measurement/auth_RAM_client.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

measure_auth_RAM_server :  $(OBJS) $(SRC_DIR)/measurement/auth_RAM_server.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 
	
measure_enrol_RAM_client : $(OBJS) $(SRC_DIR)/measurement/enrol_RAM_client.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

measure_enrol_RAM_server : $(OBJS) $(SRC_DIR)/measurement/enrol_RAM_server.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

measure_auth_client : $(OBJS) $(SRC_DIR)/measurement/auth_client.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

measure_auth_server : $(OBJS) $(SRC_DIR)/measurement/auth_server.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

measure_auth_client_key : $(OBJS) $(SRC_DIR)/measurement/auth_client_key.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

measure_auth_server_key : $(OBJS) $(SRC_DIR)/measurement/auth_server_key.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

measure_enrol_client : $(OBJS) $(SRC_DIR)/measurement/enrol_client.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

measure_enrol_server : $(OBJS) $(SRC_DIR)/measurement/enrol_server.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

test_pmc : $(OBJS) $(SRC_DIR)/measurement/pmc_test.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

# Rule to compile the scenarios
scenarii: scenario1 scenario2 scenario3 scenario4

scenario1: scenario1_A scenario1_B

scenario2: scenario2_A scenario2_Base_Station scenario2_C

scenario3: scenario3_A scenario3_B

scenario4: scenario4_A scenario4_B

scenario1_A: $(OBJS) $(SRC_DIR)/scenario1/scenario1_A.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

scenario1_B: $(OBJS) $(SRC_DIR)/scenario1/scenario1_B.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

scenario2_A: $(OBJS) $(SRC_DIR)/scenario2/scenario2_A.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

scenario2_Base_Station: $(OBJS) $(SRC_DIR)/scenario2/scenario2_Base_Station.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

scenario2_C: $(OBJS) $(SRC_DIR)/scenario2/scenario2_C.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

scenario3_A: $(OBJS) $(SRC_DIR)/scenario3/scenario3_A.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

scenario3_B: $(OBJS) $(SRC_DIR)/scenario3/scenario3_B.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

scenario4_A: $(OBJS) $(SRC_DIR)/scenario4/scenario4_A.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

scenario4_B: $(OBJS) $(SRC_DIR)/scenario4/scenario4_B.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 


$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Ensure bin/ directory exists
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean up all compiled files
clean:
	rm -f $(BIN_DIR)/*.o tests scenario* attack* measure*
