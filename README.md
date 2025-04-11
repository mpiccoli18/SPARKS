# Distributed PUF Authentication System

This repository contains the code related to the work on the UAV authentication protocol using Physical Unclonable Functions (PUFs). It is composed of three different scenarios testing the algorithm in different situations, measurement files to ease the measurement of some aspects of the algorithm and the code of the formal proof of the algorithm written for Tamarin prover. 

---

## 🗂️ Project Structure

```
.
├── bin/                    # Compiled binaries (if any)
├── Makefile               # Build instructions
├── src/                   # Source code
│   ├── puf.*              # PUF implementation
│   ├── UAV.*              # UAV simulation logic
│   ├── SocketModule.*     # Socket communication module
│   ├── utils.*            # Utility functions
│   ├── measurement/       # Code used for measuring overheads and performance
│   ├── scenario1/         # Basic client-server authentication
│   ├── scenario2/         # Extended scenario with base station
│   └── scenario3/         # Multi-UAV / distributed architecture
├── tamarin/               # Formal proof models
│   └── simple.spthy       # Tamarin model for protocol verification
```

---

## 🚀 How to Build and Run

### 🔨 Build Everything (default)
To compile all the scenario binaries (Scenario 1, 2, and 3), just run:

```bash
make
```

This runs the default `all` rule, which builds:
- `scenario1_client`, `scenario1_server`
- `scenario2_initial`, `scenario2_base_station`, `scenario2_supplementary`
- `scenario3_client`, `scenario3_server`

All compiled files are placed in the `bin/` directory.

---

### 📜 Scenarii
To compile all the different scenarii, run: 

```bash
make scenarii
```

It is also possible to compile one scenario only by running: 

```bash
make scenarioX
```
with `X` being the number of the scenario to compile. 

#### Scenario 1: 
This scenario represents a simple authentication process between a UAV A and B. 

To run the scenario, launch `scenario1_B` then `scenario1_A`. `scenario1_A` takes the other UAV IP in argument, ex : `./scenario1_A "127.0.0.1"` or `./scenario1_A "192.168.193.215"`

#### Scenario 2: 
This scenario represents a supplementary authentication process between a UAV C and A. A is the initial UAV. A will first register supplementary challenges with the base station. Then C will gather a challenge for A from the base station and finally attempt a connection with A following the supplementary authentication process. 

To run the scenario, launch `scenario2_Base_Station` then `scenario2_A` finally `scenario2_C`. `scenario2_A` takes the IP address of the base station and `scenario2_C` takes two arguments, first the base station IP address then A's IP address. Ex : `./scenario2_A "127.0.0.1"` and `./scenario2_C "127.0.0.1" "192.168.193.215"`

#### Scenario 3: 
This scenario represents a simple authentication process between a UAV A and B where the connection was interrupted, demonstrating the desynchronization recovery process. 

To run the scenario, launch `scenario3_B` then `scenario3_A`. `scenario3_A` takes the other UAV IP in argument, ex : `./scenario3_A "127.0.0.1"` or `./scenario3_A "192.168.193.215"`.

### 📊 Run Measurement Tools
To compile all performance and measurement-related binaries, run:

```bash
make measure
```

This builds binaries located in `src/measurement/`, such as:
- `auth_client`, `auth_server`, `enrol_client`, etc.
- `*_RAM_*` versions (optimized or modified for RAM performance)
- `pmc_test`, `warmup_impact`, and `json_impact_*`

---

### 🧹 Clean Build Artifacts
To remove all compiled binaries and object files:

```bash
make clean
```

This deletes all:
- `.o` files from `bin/`
- `scenario*`, `measure*`, and `attack*` binaries in the root directory

---

## 📐 Formal Verification

The `tamarin/simple.spthy` file contains a formal model of the authentication protocol. This model was verified using [Tamarin Prover](https://tamarin-prover.com/), ensuring protocol soundness under certain assumptions.

To run the model, first install tamarin-prover, then :
```bash
tamarin-prover interactive tamarin/simple.spthy
```
---

## 📄 License / Academic Context

This codebase supports an academic study and is intended for non-commercial research and educational use.
