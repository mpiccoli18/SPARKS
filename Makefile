# Compiler
CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -g -std=c++11 -I/home/sparks/Desktop/msgpack/include

ifdef MEASUREMENTS_DETAILLED
PROCFLAGS = -DMEASUREMENTS_DETAILLED
else
PROCFLAGS = -DMEASUREMENTS
endif


# Directories
SRC_DIR := src
BIN_DIR := bin

# Files
CPPS := $(SRC_DIR)/UAV.cpp $(SRC_DIR)/puf.cpp $(SRC_DIR)/utils.cpp $(SRC_DIR)/SocketModule.cpp $(SRC_DIR)/CycleCounter.cpp 
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BIN_DIR)/%.o, $(CPPS))
OBJS_MEASURE := $(patsubst $(SRC_DIR)/%.cpp, $(BIN_DIR)/measure_%.o, $(CPPS))

.SECONDARY: $(OBJS) $(OBJS_MEASURE)

SCENARIO1_BIN := scenario1_A scenario1_B

SCENARIO2_BIN := scenario2_A scenario2_Base_Station scenario2_C

SCENARIO3_BIN := scenario3_A scenario3_B

SCENARIO4_BIN := scenario4_A scenario4_B

SCENARII_BIN := \
    $(SCENARIO1_BIN) \
    $(SCENARIO2_BIN) \
    $(SCENARIO3_BIN) \
    $(SCENARIO4_BIN) \

MEASUREMENT_BIN := \
    1_enrol_overheads_client \
    1_enrol_overheads_server \
	2_auth_overheads_client \
    2_auth_overheads_server \
	3_auth_key_overheads_client \
	3_auth_key_overheads_server \
	4_supp_auth_overheads_initial \
	4_supp_auth_overheads_supplementary \
	5_pmc_test \
	6_warmup_impact \
	7_msgPack_impact_client \
	7_msgPack_impact_server \

# Default target
all: scenarii

# Rule to compile measurement
measure: $(MEASUREMENT_BIN)

%_client: $(OBJS_MEASURE) $(SRC_DIR)/measurement/%_client.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -fstack-usage $(PROCFLAGS) $^ -o $@ -ltomcrypt

%_server: $(OBJS_MEASURE) $(SRC_DIR)/measurement/%_server.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(PROCFLAGS) $^ -o $@ -ltomcrypt

%_initial: $(OBJS_MEASURE) $(SRC_DIR)/measurement/%_initial.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(PROCFLAGS) $^ -o $@ -ltomcrypt

%_supplementary: $(OBJS_MEASURE) $(SRC_DIR)/measurement/%_supplementary.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(PROCFLAGS) $^ -o $@ -ltomcrypt

5_pmc_test: $(OBJS_MEASURE) $(SRC_DIR)/measurement/5_pmc_test.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(PROCFLAGS) $^ -o $@ -ltomcrypt

6_warmup_impact: $(OBJS_MEASURE) $(SRC_DIR)/measurement/6_warmup_impact.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(PROCFLAGS) $^ -o $@ -ltomcrypt

# 1_enrol_overheads_client : $(OBJS) $(SRC_DIR)/measurement/$@.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt

# 1_enrol_overheads_server : $(OBJS) $(SRC_DIR)/measurement/$@.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt

# measure_json_client : $(OBJS) $(SRC_DIR)/measurement/json_impact_client.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

# measure_json_server : $(OBJS) $(SRC_DIR)/measurement/json_impact_server.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

# measure_supp_auth_supplementary : $(OBJS) $(SRC_DIR)/measurement/supp_auth_supplementary.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

# measure_supp_auth_initial : $(OBJS) $(SRC_DIR)/measurement/supp_auth_initial.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

# measure_warmup : $(OBJS) $(SRC_DIR)/measurement/warmup_impact.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

# measure_auth_RAM_client : $(OBJS) $(SRC_DIR)/measurement/auth_RAM_client.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

# measure_auth_RAM_server :  $(OBJS) $(SRC_DIR)/measurement/auth_RAM_server.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 
	
# measure_enrol_RAM_client : $(OBJS) $(SRC_DIR)/measurement/enrol_RAM_client.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

# measure_enrol_RAM_server : $(OBJS) $(SRC_DIR)/measurement/enrol_RAM_server.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

# measure_auth_client : $(OBJS) $(SRC_DIR)/measurement/auth_client.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

# measure_auth_server : $(OBJS) $(SRC_DIR)/measurement/auth_server.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

# measure_auth_client_key : $(OBJS) $(SRC_DIR)/measurement/auth_client_key.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

# measure_auth_server_key : $(OBJS) $(SRC_DIR)/measurement/auth_server_key.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

# measure_enrol_client : $(OBJS) $(SRC_DIR)/measurement/enrol_client.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

# measure_enrol_server : $(OBJS) $(SRC_DIR)/measurement/enrol_server.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

# test_pmc : $(OBJS) $(SRC_DIR)/measurement/7_pmc_test.cpp | $(BIN_DIR)
# 	$(CXX) $(CXXFLAGS) $^ -o $@ -ltomcrypt 

# Rule to compile the scenarios
scenarii: $(SCENARII_BIN)

scenario1: $(SCENARIO1_BIN)

scenario2: $(SCENARIO2_BIN)

scenario3: $(SCENARIO3_BIN)

scenario4: $(SCENARIO4_BIN)

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

# Normal object rule
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Measurement object rule with -DMEASUREMENTS
$(BIN_DIR)/measure_%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(PROCFLAGS) -c $< -o $@

# Clean up all compiled files
clean:
	rm -f $(BIN_DIR)/*.o 
	rm -f $(SCENARII_BIN)
	rm -f $(MEASUREMENT_BIN)
	rm -f test*
