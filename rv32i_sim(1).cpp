// Computer organization and Assembly Language
// Project 1
// RISC-V Simulator
// Ahmed Refaay (900141806) & Hossam El Samanody (900143231)
// Implemented using the skeleton provided by Dr. Shalan

// Standard: a0 -> x10, v0 -> x17

#include <iostream>
#include <fstream>
#include "stdlib.h"
#include <iomanip>
using namespace std;

int regs[32]={0}; // All x registers
unsigned int pc = 0x0; // Program counter register

char memory[8*1024];	// Only 8KB of memory located at address 0

void emitError(char *s) // Prints error messages
{
	cout << s;
	exit(0);
}

void printPrefix(unsigned int instA, unsigned int instW){ // Prints instructions' prefix as program counter and hexa-decimal instruction
	cout << "0x" << hex << std::setfill('0') << std::setw(8) << instA << "\t0x" << std::setw(8) << instW;
}

bool instDecExec(unsigned int instWord){ // Instructions excusion
	unsigned int rd, rs1, rs2, funct3, funct7, opcode; // Instruction decoding parts
	unsigned int I_imm, S_imm, B_imm, U_imm, J_imm; // All immediate types
	unsigned int address; // Needed address
	unsigned int instPC = pc - 4; // PC for current instruction

	bool terminated = false; // Changes when SCALL by v0 = 10
	char tempc; // Temp char
	int counter; // String counter

	opcode = instWord & 0x0000007F; // Get opcode
	rd = (instWord >> 7) & 0x0000001F; // Get rd
	funct3 = (instWord >> 12) & 0x00000007; // Get funct3
	rs1 = (instWord >> 15) & 0x0000001F; // Get rs1
	rs2 = (instWord >> 20) & 0x0000001F; // Get rs2
	funct7 = (instWord >> 25) & 0x0000007F; // Get funct7

	I_imm = ((instWord >> 20) & 0x7FF) | (((instWord >> 31) ? 0xFFFFF800 : 0x0)); // Sets I-Immediate: — inst[31] — inst[30:25] inst[24:21] inst[20]
	S_imm = ((instWord >> 7) & 0x1) | ((instWord >> 8) & 0xF) | ((instWord >> 25) & 0x3F) | (((instWord >> 31) ? 0xFFFFF800 : 0x0)); // Sets S-Immediate: — inst[31] — inst[30:25] inst[11:8] inst[7]
	B_imm = (((((instWord >> 8) & 0xF) << 1) + (((instWord >> 25) & 0x3F) << 5) + (((instWord >> 7) & 0x1) << 11) + (((instWord >> 31) ? 0xFFFFF000 : 0x0)))); // Sets B-Immediate: — inst[31] — inst[7] inst[30:25] inst[11:8] 0 
	U_imm = (((instWord >> 12) & 0xFF) | ((instWord >> 20) & 0x7FF) | ((instWord >> 31) & 0x1)) << 12; // Sets U-Immediate: inst[31] inst[30:20] inst[19:12] — 0 — 
	J_imm = ((((instWord >> 21) & 0x3FF) << 1) + (((instWord >> 20) & 0x1) << 11) + (((instWord >> 12) & 0xFF) << 12) + (((instWord >> 31) ? 0xFFF00000 : 0x0))); // Sets J-Immediate: — inst[31] — inst[19:12] inst[20] inst[24:21] inst[11:8] 0

	if (opcode == 0x33){ // R Instructions
		switch (funct3){ // Which R function
		case 0: if (funct7 == 32) { // Case 0: Add (funct7 = 0) or Sub (funct7 = 32)
			regs[rd] = regs[rs1] - regs[rs2]; // Excute: subtraction
		}
				else {
					regs[rd] = regs[rs1] + regs[rs2]; // Excute: addition
				}
				break;
		case 1:  // Case 1: Sll (funct7 = 0)
			regs[rd] = (regs[rs1] << (regs[rs2] & 0x001F)); // Excute: shift left logical rs1 by the lower 5 bits in rs2
			break;
		case 2:  // Case 2: Slt (funct7 = 0)
			regs[rd] = (regs[rs1] < regs[rs2]); // Excute: rd = 1 if rs1 < rs2 and 0 otherwise
			break;
		case 3:  // Case 3: Sltu (funct7 = 0)
			regs[rd] = ((unsigned int)regs[rs1] < (unsigned int)regs[rs2]); // Excute: unsigned set on less than
			break;
		case 4:  // Case 4: Xor (funct7 = 0)
			regs[rd] = regs[rs1] ^ regs[rs2]; // Excute: rd = rs1 XOR rs2
			break;
		case 5: if (funct7 == 32) { // Case 5: Srl (funct7 = 0) or Sra (funct7 = 32)
			regs[rd] = (regs[rs1] >> (regs[rs2] & 0x001F)); // Excute: shift right arthimetic rs1 by the lower 5 bits in rs2
		}
				else {
					regs[rd] = ((unsigned int)regs[rs1] >> (regs[rs2] & 0x001F)); // Excute: shift right logical rs1 by the lower 5 bits in rs2
				}
				break;
		case 6:  // Case 6: Or (funct7 = 0)
			regs[rd] = regs[rs1] | regs[rs2]; // Excute rd = rs1 OR rs2
			break;
		case 7:  // Case 7: And (funct7 = 0)
			regs[rd] = regs[rs1] & regs[rs2]; // Excute rd = rs1 AND rs2
			break;
		default:;
		}
	}
	else if (opcode == 0x13){	// I instructions
		switch (funct3){ // Which I function
		case 0:	// Case 0: Addi
			regs[rd] = regs[rs1] + (int)I_imm; // Excute: immediate addition
			break;
		case 1:  // Case 1: Slli
			regs[rd] = (regs[rs1] << rs2); // Excute: shift left logical rs1 by shamt5(rs2)
			break;
		case 2:  // Case 2: Slti
			regs[rd] = (regs[rs1] < (int)I_imm); // Excute: rd = 1 if rs1 < I_imm and 0 otherwise
			break;
		case 3:  // Case 3: Sltiu
			regs[rd] = ((unsigned int)regs[rs1] < (unsigned int)I_imm); // Excute: unsigned set on less than immediate
			break;
		case 4:  // Case 4: Xori
			regs[rd] = regs[rs1] ^ (int)I_imm; // Excute: rd = rs1 XOR I_imm
			break;
		case 5: if (funct7 == 32) { // Case 5: Srli (funct7 = 0) or Srai (funct7 = 32)
			regs[rd] = (regs[rs1] >> rs2); // Excute: shift right arthimetic rs1 by by shamt5(rs2)
		}
				else {
					regs[rd] = ((unsigned int)regs[rs1] >> rs2); // Excute: shift right logical rs1 by shamt5(rs2)
				}
				break;
		case 6:  // Case 6: Ori
			regs[rd] = regs[rs1] | (int)I_imm; // Excute rd = rs1 OR I_imm
			break;
		case 7:  // Case 7: Andi
			regs[rd] = regs[rs1] & (int)I_imm; // Excute rd = rs1 AND I_imm
			break;
		default:;
		}
	}
	else if (opcode == 0x23){	// S instructions
		switch (funct3){ // Which S function
		case 0:	// Case 0: SB
			memory[regs[rs1] + (int)S_imm] = (regs[rs2] & 0xFF); // Excute: store byte in rs2 to mem [rs1+imm]
			break;
		case 1:  // Case 1: Sh
			memory[regs[rs1] + (int)S_imm] = (regs[rs2] & 0xFF); // Excute: store lowest byte in rs2 to mem [rs1+imm]
			memory[regs[rs1] + (int)S_imm + 1] = ((regs[rs2] >> 8) & 0xFF); // Excute: store highest byte in rs2 to mem [rs1+imm+1]
			break;
		case 2:  // Case 2: Sw
			memory[regs[rs1] + (int)S_imm] = (regs[rs2] & 0xFF); // Excute: store lowest byte in rs2 to mem [rs1+imm]
			memory[regs[rs1] + (int)S_imm + 1] = ((regs[rs2] >> 8) & 0xFF); // Excute: store highest byte in rs2 to mem [rs1+imm+1]
			memory[regs[rs1] + (int)S_imm + 2] = ((regs[rs2] >> 16) & 0xFF); // Excute: store lowest byte in rs2 to mem [rs1+imm+2]
			memory[regs[rs1] + (int)S_imm + 3] = ((regs[rs2] >> 24) & 0xFF); // Excute: store highest byte in rs2 to mem [rs1+imm+3]
			break;
		default:;
		}
	}
	else if (opcode == 0x03){	// I + L instructions
		switch (funct3){ // Which I + L function
		case 0:	// Case 0: LB
			regs[rd] = (memory[regs[rs1] + (int)I_imm] | (((memory[regs[rs1] + (int)I_imm] >> 7) ? 0xFFFFFF00 : 0x0))); // Excute: load byte from memory to rd
			break;
		case 1:  // Case 1: Lh
			regs[rd] = memory[regs[rs1] + (int)I_imm] | ((memory[regs[rs1] + (int)I_imm + 1]) << 8) | (((memory[regs[rs1] + (int)I_imm + 1] >> 7) ? 0xFFFF0000 : 0x0)); // Excute: load half word from memory to rd
			break;
		case 2:  // Case 2: Lw
			regs[rd] = memory[regs[rs1] + (int)I_imm] | ((memory[regs[rs1] + (int)I_imm + 1]) << 8) | ((memory[regs[rs1] + (int)I_imm + 2]) << 16) | ((memory[regs[rs1] + (int)I_imm + 3]) << 24); // Excute: load word from memory to rd
			break;
		case 4:  // Case 3: LBu
			regs[rd] = memory[regs[rs1] + (int)I_imm] | 0x0; // Excute: load byte unsigned from memory to rd
			break;
		case 5:  // Case 4: Lhu
			regs[rd] = memory[regs[rs1] + (int)I_imm] | ((memory[regs[rs1] + (int)I_imm + 1]) << 8) | 0x0; // Excute: load half word unsigned from memory to rd
			break;
		default:;
		}
	}
	else if (opcode == 0x63){	// Sb instructions
		switch (funct3){ // Which Sb function
		case 0:	// Case 0: Beq
			if (regs[rs1] == regs[rs2]) pc = instPC + (int)B_imm; // Excute: branch by B_imm if (rs1 == rs2)
			break;
		case 1:  // Case 1: Bne
			if (regs[rs1] != regs[rs2]) pc = instPC + (int)B_imm; // Excute: branch by B_imm if (rs1 != rs2)
			break;
		case 4:  // Case 4: Blt
			if (regs[rs1] < regs[rs2]) pc = instPC + (int)B_imm; // Excute: branch by B_imm if (rs1 < rs2)
			break;
		case 5:	// Case 5: Bge
			if (regs[rs1] >= regs[rs2]) pc = instPC + (int)B_imm; // Excute: branch by B_imm if (rs1 >= rs2)
			break;
		case 6:  // Case 6: Bltu
			if ((unsigned int)regs[rs1] < (unsigned int)regs[rs2]) pc = instPC + (int)B_imm; // Excute: branch by B_imm if (rs1 < rs2) unsigned
			break;
		case 7:  // Case 7: Bgeu
			if ((unsigned int)regs[rs1] >= (unsigned int)regs[rs2]) pc = instPC + (int)B_imm; // Excute: branch by B_imm if (rs1 >= rs2) unsigned
			break;
		default:;
		}
	}
	else if (opcode == 0x37){	// Lui instruction
		regs[rd] = (int)U_imm; // Excute: load U_imm in the upper 20 bits of rd and the lower 12 bits = 0
	}
	else if (opcode == 0x17){	// Auipc instruction
		regs[rd] = pc + (int)U_imm; // Excute: loads U_imm to the upper 20 bits of rd and adds pc
	}
	else if (opcode == 0x6F){	// Jal instruction
		regs[rd] = pc; // Store pc of following instruction in rd
		pc = instPC + (int)J_imm; // Excute: jump by offest from current instruction
	}
	else if (opcode == 0x67){	// Jalr instruction
		regs[rd] = pc; // Store pc of following instruction in rd
		pc = ((regs[rs1] + (int)I_imm) & 0xFFFFFFFE); // Excute: jump by offest from the instruction address in rs1
	}
	else if (opcode == 0x73){	// Scall instruction: $v0 is x17, $a0 is x10
		switch (regs[17]){ // Which S function by $v0 as the choosing argument
		case 1: // Print an integer
			cout << dec << regs[10]; // Output integer from x10($a0)
			break;
		case 4: // Print a string
			tempc = memory[regs[10]]; // Temp char
			counter = 0; // String counter
			while (tempc){ // Till NULL is hit
				cout << memory[regs[10]+counter]; // Output string from memory by address in x10($a0)
				counter++;
				tempc = memory[regs[10] + counter]; // To compare next character
			}
			break;
		case 5: // Read an integer
			cin >> regs[17]; // Input integer into x17($v0)
			break;
		case 10: // Terminate execution
			terminated = true;
			break;
		default:;
		}
	}
	else {
	}
	regs[0] = 0;
	return terminated;
}

void instDecPrint(unsigned int instWord){ // Instructions full print
	unsigned int rd, rs1, rs2, funct3, funct7, opcode; // Instruction decoding parts
	unsigned int I_imm, S_imm, B_imm, U_imm, J_imm; // All immediate types
	unsigned int address; // Needed address
	unsigned int instPC = pc - 4; // PC for current instruction

	char tempc; // Temp char
	int counter; // String counter

	opcode = instWord & 0x0000007F; // Get opcode
	rd = (instWord >> 7) & 0x0000001F; // Get rd
	funct3 = (instWord >> 12) & 0x00000007; // Get funct3
	rs1 = (instWord >> 15) & 0x0000001F; // Get rs1
	rs2 = (instWord >> 20) & 0x0000001F; // Get rs2
	funct7 = (instWord >> 25) & 0x0000007F; // Get funct7

	I_imm = ((instWord >> 20) & 0x7FF) | (((instWord >> 31) ? 0xFFFFF800 : 0x0)); // Sets I-Immediate: — inst[31] — inst[30:25] inst[24:21] inst[20]
	S_imm = ((instWord >> 7) & 0x1) | ((instWord >> 8) & 0xF) | ((instWord >> 25) & 0x3F) | (((instWord >> 31) ? 0xFFFFF800 : 0x0)); // Sets S-Immediate: — inst[31] — inst[30:25] inst[11:8] inst[7]
	B_imm = (((((instWord >> 8) & 0xF) << 1) + (((instWord >> 25) & 0x3F) << 5) + (((instWord >> 7) & 0x1) << 11) + (((instWord >> 31) ? 0xFFFFF000 : 0x0)))); // Sets B-Immediate: — inst[31] — inst[7] inst[30:25] inst[11:8] 0 
	U_imm = (((instWord >> 12) & 0xFF) | ((instWord >> 20) & 0x7FF) | ((instWord >> 31) & 0x1)) << 12; // Sets U-Immediate: inst[31] inst[30:20] inst[19:12] — 0 — 
	J_imm = ((((instWord >> 21) & 0x3FF) << 1) + (((instWord >> 20) & 0x1) << 11) + (((instWord >> 12) & 0xFF) << 12) + (((instWord >> 31) ? 0xFFF00000 : 0x0))); // Sets J-Immediate: — inst[31] — inst[19:12] inst[20] inst[24:21] inst[11:8] 0

	printPrefix(instPC, instWord); // Prints memory data as locations and data in these locations

	if (opcode == 0x33){ // R Instructions
		switch (funct3){ // Which R function
		case 0: if (funct7 == 32) { // Case 0: Add (funct7 = 0) or Sub (funct7 = 32)
			cout << "\tSUB\tx" << dec << rd << ", x" << rs1 << ", x" << rs2 << "\n"; // Prints RISC-V assembly code
		}
				else {
					cout << "\tADD\tx" << dec << rd << ", x" << rs1 << ", x" << rs2 << "\n"; // Prints RISC-V assembly code
				}
				break;
		case 1:  // Case 1: Sll (funct7 = 0)
			cout << "\tSLL\tx" << dec << rd << ", x" << rs1 << ", x" << rs2 << "\n"; // Prints RISC-V assembly code
			break;
		case 2:  // Case 2: Slt (funct7 = 0)
			cout << "\tSLT\tx" << dec << rd << ", x" << rs1 << ", x" << rs2 << "\n"; // Prints RISC-V assembly code
			break;
		case 3:  // Case 3: Sltu (funct7 = 0)
			cout << "\tSLTU\tx" << dec << rd << ", x" << rs1 << ", x" << rs2 << "\n"; // Prints RISC-V assembly code
			break;
		case 4:  // Case 4: Xor (funct7 = 0)
			cout << "\tXOR\tx" << dec << rd << ", x" << rs1 << ", x" << rs2 << "\n"; // Prints RISC-V assembly code
			break;
		case 5: if (funct7 == 32) { // Case 5: Srl (funct7 = 0) or Sra (funct7 = 32)
			cout << "\tSRA\tx" << dec << rd << ", x" << rs1 << ", x" << rs2 << "\n"; // Prints RISC-V assembly code
		}
				else {
					cout << "\tSRL\tx" << dec << rd << ", x" << rs1 << ", x" << rs2 << "\n"; // Prints RISC-V assembly code
				}
				break;
		case 6:  // Case 6: Or (funct7 = 0)
			cout << "\tOR\tx" << dec << rd << ", x" << rs1 << ", x" << rs2 << "\n"; // Prints RISC-V assembly code
			break;
		case 7:  // Case 7: And (funct7 = 0)
			cout << "\tAND\tx" << dec << rd << ", x" << rs1 << ", x" << rs2 << "\n"; // Prints RISC-V assembly code
			break;
		default:
			cout << "\tUnkown R Instruction \n"; // Other R instructions
		}
	}
	else if (opcode == 0x13){	// I instructions
		switch (funct3){ // Which I function
		case 0:	// Case 0: Addi
			cout << "\tADDI\tx" << dec << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n"; // Prints RISC-V assembly code
			break;
		case 1:  // Case 1: Slli
			cout << "\tSLLI\tx" << dec << rd << ", x" << rs1 << ", " << hex << "0x" << (int)rs2 << "\n"; // Prints RISC-V assembly code
			break;
		case 2:  // Case 2: Slti
			cout << "\tSLTI\tx" << dec << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n"; // Prints RISC-V assembly code
			break;
		case 3:  // Case 3: Sltiu
			cout << "\tSLTIU\tx" << dec << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n"; // Prints RISC-V assembly code
			break;
		case 4:  // Case 4: Xori
			cout << "\tXORI\tx" << dec << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n"; // Prints RISC-V assembly code
			break;
		case 5: if (funct7 == 32) { // Case 5: Srli (funct7 = 0) or Srai (funct7 = 32)
			cout << "\tSRAI\tx" << dec << rd << ", x" << rs1 << ", " << hex << "0x" << (int)rs2 << "\n"; // Prints RISC-V assembly code
		}
				else {
					cout << "\tSRLI\tx" << dec << rd << ", x" << rs1 << ", " << hex << "0x" << (int)rs2 << "\n"; // Prints RISC-V assembly code
				}
				break;
		case 6:  // Case 6: Ori
			cout << "\tORI\tx" << dec << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n"; // Prints RISC-V assembly code
			break;
		case 7:  // Case 7: Andi
			cout << "\tANDI\tx" << dec << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n"; // Prints RISC-V assembly code
			break;
		default:
			cout << "\tUnkown I Instruction \n"; // Other I instructions
		}
	}
	else if (opcode == 0x23){	// S instructions
		switch (funct3){ // Which S function
		case 0:	// Case 0: SB
			cout << "\tSB\tx" << dec << rs1 << ", x" << rs2 << ", " << hex << "0x" << (int)S_imm << "\n"; // Prints RISC-V assembly code
			break;
		case 1:  // Case 1: Sh
			cout << "\tSH\tx" << dec << rs1 << ", x" << rs2 << ", " << hex << "0x" << (int)S_imm << "\n"; // Prints RISC-V assembly code
			break;
		case 2:  // Case 2: Sw
			cout << "\tSW\tx" << dec << rs1 << ", x" << rs2 << ", " << hex << "0x" << (int)S_imm << "\n"; // Prints RISC-V assembly code
			break;
		default:
			cout << "\tUnkown S Instruction \n"; // Other S instructions
		}
	}
	else if (opcode == 0x03){	// I + L instructions
		switch (funct3){ // Which I + L function
		case 0:	// Case 0: LB
			cout << "\tLB\tx" << dec << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n"; // Prints RISC-V assembly code
			break;
		case 1:  // Case 1: Lh
			cout << "\tLH\tx" << dec << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n"; // Prints RISC-V assembly code
			break;
		case 2:  // Case 2: Lw
			cout << "\tLW\tx" << dec << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n"; // Prints RISC-V assembly code
			break;
		case 4:  // Case 3: LBu
			cout << "\tLBU\tx" << dec << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n"; // Prints RISC-V assembly code
			break;
		case 5:  // Case 4: Lhu
			cout << "\tLHU\tx" << dec << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n"; // Prints RISC-V assembly code
			break;
		default:
			cout << "\tUnkown I + L Instruction \n"; // Other I + L instructions
		}
	}
	else if (opcode == 0x63){	// Sb instructions
		switch (funct3){ // Which Sb function
		case 0:	// Case 0: Beq
			cout << "\tBEQ\tx" << dec << rs1 << ", x" << rs2 << ", " << hex << "0x" << (int)B_imm << "\n"; // Prints RISC-V assembly code
			break;
		case 1:  // Case 1: Bne
			cout << "\tBNE\tx" << dec << rs1 << ", x" << rs2 << ", " << hex << "0x" << (int)B_imm << "\n"; // Prints RISC-V assembly code
			break;
		case 4:  // Case 4: Blt
			cout << "\tBLT\tx" << dec << rs1 << ", x" << rs2 << ", " << hex << "0x" << (int)B_imm << "\n"; // Prints RISC-V assembly code
			break;
		case 5:	// Case 5: Bge
			cout << "\tBGE\tx" << dec << rs1 << ", x" << rs2 << ", " << hex << "0x" << (int)B_imm << "\n"; // Prints RISC-V assembly code
			break;
		case 6:  // Case 6: Bltu
			cout << "\tBLTU\tx" << dec << rs1 << ", x" << rs2 << ", " << hex << "0x" << (int)B_imm << "\n"; // Prints RISC-V assembly code
			break;
		case 7:  // Case 7: Bgeu
			cout << "\tBGEU\tx" << dec << rs1 << ", x" << rs2 << ", " << hex << "0x" << (int)B_imm << "\n"; // Prints RISC-V assembly code
			break;
		default:
			cout << "\tUnkown Sb Instruction \n"; // Other Sb instructions
		}
	}
	else if (opcode == 0x37){	// Lui instruction
		cout << "\tLUI\tx" << dec << rd << ", " << hex << "0x" << (int)U_imm << "\n"; // Prints RISC-V assembly code
	}
	else if (opcode == 0x17){	// Auipc instruction
		cout << "\tAUIPC\tx" << dec << rd << ", " << hex << "0x" << (int)U_imm << "\n"; // Prints RISC-V assembly code
	}
	else if (opcode == 0x6F){	// Jal instruction
		cout << "\tJAL\tx" << dec << rd << ", " << hex << "0x" << (int)J_imm << "\n"; // Prints RISC-V assembly code
	}
	else if (opcode == 0x67){	// Jalr instruction
		cout << "\tJALR\tx" << dec << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n"; // Prints RISC-V assembly code
	}
	else if (opcode == 0x73){	// Scall instruction: $v0 is x17, $a0 is x10
		cout << "\tSCALL" << "\n"; // Prints RISC-V assembly code
		switch (regs[17]){ // Which S function by $v0 as the choosing argument
		case 1: // Print an integer
			break;
		case 4: // Print a string
			break;
		case 5: // Read an integer
			break;
		case 10: // Terminate execution
			break;
		default:;
		}
	}
	else {
		cout << "\tUnkown Instruction \n"; // Other instructions
	}
}

int main(int argc, char *argv[]){ // Arguments for command line operation

	unsigned int instWord=0; // To fetch each instruction
	ifstream inFile; // Input file
	ofstream outFile; // Output file

	if(argc<1) emitError("use: rv32i_sim <machine_code_file_name>\n"); // No arguments in the command line input

	inFile.open(argv[1], ios::in | ios::binary | ios::ate); // Open file as a binary input file, and iterator at the end of the file

	if(inFile.is_open()) // If opens successfully
	{
		int fsize = inFile.tellg(); // Get number of instructions

		inFile.seekg (0, inFile.beg); // Set iterator at the begining of the file
		if(!inFile.read((char *)memory, fsize)) emitError("Cannot read from input file\n"); // If cannot read data

		bool terminated = false; // Changes to true with SCALL by v0 = 10

		cout << "Program Excusion: \n";

		while(!terminated){ // Forever, till SCALL with $v0 = 10
			instWord = (unsigned char)memory[pc] | (((unsigned char)memory[pc + 1]) << 8) | (((unsigned char)memory[pc + 2]) << 16) | (((unsigned char)memory[pc + 3]) << 24); // Gets instruction word from the lowest four bytes in the memory
			pc += 4; // PC++
			terminated = instDecExec(instWord); // Excute instruction
		}
		pc = 0x0;
		cout << "\n\nProgram RISC-V Code: \n";
		while (pc < fsize){ // Print all instructions in the file in order
			instWord = (unsigned char)memory[pc] | (((unsigned char)memory[pc + 1]) << 8) | (((unsigned char)memory[pc + 2]) << 16) | (((unsigned char)memory[pc + 3]) << 24); // Gets instruction word from the lowest four bytes in the memory
			pc += 4; // PC++
			instDecPrint(instWord); // Print instruction
		}

		// Dump the registers
		cout << "\nAll Registers: \n";
		for(int i=0;i<32;i++)
				cout  <<"x" << dec << i << ": \t"<< "0x" << hex << std::setfill('0') << std::setw(8) << regs[i] << "\n";
	} else emitError("Cannot access input file\n"); // If file did not open
	system("pause");
}