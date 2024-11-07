#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>

using namespace std;

bool haltt = false;  // Global halt flag

class Register {
protected:
    int intRegisters[16] = {0};

public:
    int getIntCell(int address) {
        if (address < 0 || address >= 16) {
            throw out_of_range("Integer register address out of bounds.");
        }
        return intRegisters[address];
    }

    void setIntCell(int address, int val) {
        if (address < 0 || address >= 16) {
            throw out_of_range("Integer register address out of bounds.");
        }
        intRegisters[address] = val;
    }
};

class Memory {
public:
    int memory[256] = {0};

    int getCell(int address) {
        if (address < 0 || address >= 256) {
            throw out_of_range("Memory address out of bounds.");
        }
        return memory[address];
    }

    void setCell(int address, int val) {
        if (address < 0 || address >= 256) {
            throw out_of_range("Memory address out of bounds.");
        }
        memory[address] = val;
    }
};

class ALU {
public:
    int hexToDec(int hex) {
        return hex;  // Since we're directly working with integers
    }
};

class CU {
public:
    void load1(int regAddr, int memAddr, Register &reg, Memory &mem) {
        try {
            int value = mem.getCell(memAddr);
            cout << "LOAD1: Loading value " << hex << value
                 << " from memory address [" << hex << memAddr
                 << "] into register R" << dec << regAddr << endl;
            reg.setIntCell(regAddr, value);
        } catch (const out_of_range &e) {
            cout << "Error: Memory address [" << hex << memAddr << "] out of bounds." << endl;
        }
    }

    void load2(int regAddr, int value, Register &reg) {
        cout << "LOAD2: Loading immediate value " << hex << value
             << " into register R" << dec << regAddr << endl;
        reg.setIntCell(regAddr, value);  // Store the value directly in the register
    }

    void store(int regAddr, int memAddr, Register &reg, Memory &mem) {
        int value = reg.getIntCell(regAddr);
        cout << "STORE: Storing value " << value
             << " from register R" << regAddr
             << " into memory address [" << hex << memAddr << "]" << endl;
        mem.setCell(memAddr, value);
    }

    void add(int regIdx1, int regIdx2, int destRegIdx, Register &reg) {
        int result = reg.getIntCell(regIdx1) + reg.getIntCell(regIdx2);
        cout << "ADD: Adding value from register R" << regIdx1
             << " and register R" << regIdx2
             << ", result stored in register R" << destRegIdx << endl;
        reg.setIntCell(destRegIdx, result);
    }

    void move(int srcReg, int destReg, Register &reg) {
        int value = reg.getIntCell(srcReg);
        cout << "MOVE: Moving value " << value
             << " from register R" << srcReg
             << " to register R" << destReg << endl;
        reg.setIntCell(destReg, value);
    }

    void jump(int addr, int &programCounter) {
        cout << "JUMP: Setting program counter to address " << hex << addr << endl;
        programCounter = addr;
    }

    void halt(bool &haltt) {
        cout << "HALT: Program execution has stopped." << endl;
        haltt = true;
    }
};

class CPU {
protected:
    Register reg;
    ALU alu;
    CU cu;

public:
    void fetch(int &instruction, Memory &mem, int &programCounter) {
        if (programCounter >= 256) {
            cout << "Error: Program counter out of bounds: " << programCounter << endl;
            instruction = 0;
            return;
        }

        instruction = (mem.getCell(programCounter) << 8) | mem.getCell(programCounter + 1);

        cout << "Fetched instruction: " << hex << instruction
             << " at PC: " << programCounter << endl;

        programCounter += 2;
    }

    void execute(int instruction, Memory &mem, Register &reg, CU &cu, int &programCounter, bool &haltt) {
        int opCode = (instruction >> 12) & 0xF;
        int R = (instruction >> 8) & 0xF;
        int S = (instruction >> 4) & 0xF;
        int T = instruction & 0xF;

        cout << "Executing instruction: " << hex << instruction
             << " | opCode: " << opCode << ", R: " << R
             << ", S: " << S << ", T: " << T << endl;

        switch (opCode) {
            case 0x1:  // LOAD1
                cu.load1(R, (S << 4 | T), reg, mem);
                break;
            case 0x2:  // LOAD2
                cu.load2(R, (S << 4 | T), reg);
                break;
            case 0x3:  // STORE
                cu.store(R, (S << 4 | T), reg, mem);
                break;
            case 0x5:  // ADD
                cu.add(S, T, R, reg);
                break;
            case 0x4:  // MOVE
                cu.move(S, R, reg);
                break;
            case 0xB:  // JUMP
                cu.jump((S << 4 | T), programCounter);
                break;
            case 0xC:  // HALT
                cu.halt(haltt);
                break;
            default:
                cout << "Invalid instruction: " << hex << instruction << endl;
                break;
        }
    }

    void runNextStep(Memory &mem, int &programCounter, bool &haltt) {
        int instruction;

        fetch(instruction, mem, programCounter);

        execute(instruction, mem, reg, cu, programCounter, haltt);
    }

    bool &isHalted() const {
        return haltt;
    }
};

class Machine : public CPU {
    Memory memory;
    int programCounter = 0x0A;  // Start storing in memory from address 0A

public:
    void loadProgramFile(const string &fileName) {
        ifstream file(fileName);
        if (!file) {
            cout << "Error: Could not open file " << fileName << endl;
            return;
        }

        string line;
        int address = 0;

        while (getline(file, line) && address < 256) {
            try {
                int instruction = stoi(line, nullptr, 16);
                memory.setCell(address++, instruction);
            } catch (const invalid_argument &e) {
                cout << "Invalid instruction format in file: " << line << endl;
                continue;
            }
        }

        file.close();
        cout << "Program loaded from " << fileName << " with " << address << " instructions." << endl;
    }

    void outputState() {
        cout << "Integer Registers:\n";
        for (int i = 0; i < 16; ++i) {
            cout << "R" << i << ": " << dec << reg.getIntCell(i) << " (dec), " << hex << reg.getIntCell(i) << " (hex)" << endl;
        }

        cout << "Memory contents:\n";
        for (int i = 0; i < 16; ++i) {
            cout << "Memory[" << hex << i << "]: " << dec << memory.getCell(i) << " (dec), "
                 << hex << memory.getCell(i) << " (hex)" << endl;
        }
    }

    void run() {
        while (!isHalted()) {
            runNextStep(memory, programCounter, haltt);
            outputState();
        }
    }
};

int main() {
    Machine machine;
    string fileName;
    cout << "Enter the file name: ";
    cin >> fileName;

    machine.loadProgramFile(fileName);
    machine.run();

    return 0;
}
