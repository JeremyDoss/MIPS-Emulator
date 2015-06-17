/*
Jeremy Doss
Main Project - MIPS emulator
CSCE 2610 - Assembly Language and Comp. Organization
8/1/14
*/

#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

//Declare global instruction memory and registries.
vector<unsigned int> InstrMem;
int reg[33] = {};
int globalpc;

//Declare function prototypes (Description at implementation).
void fetch_mips();
void decode_mips(unsigned int instr);
void store_input(string inFile);
void r_type(unsigned int instr);
void i_type(unsigned int instr);
void j_type(unsigned int instr);
void print_reg();

//Main program code.
int main()
{
	string input;

	cout << "\n---------------------- M I P S - E M U L A T O R ----------------------\n" << endl;
	           
	cout << "Options:\n1: 'run'\n2: 'print'\n3: 'exit'\n" << endl;

	//User input prompt (main loop).
	while (1) {
		cout << "User Input> ";
		cin >> input;

		if (input == "run") {
			//Specify FILENAME.txt
			InstrMem.clear();
			cout << "\nEnter filename: ";
			cin >> input;
			//Load instruction file to instruction memory.
			store_input(input);
			fetch_mips();
		}
		else if (input == "print") {
			print_reg();
		}
		else if (input == "exit") {
			//Clean up memory and break from prompt loop.
			cout << "\n------------------THANK YOU! Exiting MIPS emulator...------------------\n" << endl;
			break;
		}
		else
			cout << "Error: Invalid command." << endl;
	}
	return 0;
}


//Read file input and store instructions and starting PC.
void store_input(string inFile)
{
	ifstream input(inFile.c_str());
	string line;
	unsigned int instr;

	if(input) {
		//Retrieve the location of the P.C. (PC).
		getline(input,line);
		line = line.substr(1);
		reg[32] = strtol(line.c_str(), NULL, 16);
		globalpc = reg[32];

		//Store instruction lines into vector until no more lines exist.
		while (getline(input,line)) {
			instr = strtoul(line.c_str(), NULL, 16);
			InstrMem.push_back(instr);
		}

		input.close();
	}
	else {
		cout << "Error opening the input file!" << endl;
		cout << "Terminating program...\n" << endl;
		exit(0);
	}
}


//Execute the MIPS code from instruction memory.
void fetch_mips()
{
	unsigned int instr;
	unsigned int memloc = reg[32];
	int i = 0;

	while (1)
	{
		//Fetch instruction from memory.
		instr = InstrMem[i];

		//increment PC.
		reg[32] += 4;

		//Exit instruction is read.
		if (instr == 0xffffffff) {
			cout << "-- Program terminated normally (0xFFFFFFFF) --\n" << endl;
			break;
		}
		//End of instruction memory is reached. Terminate program.
		if (i+1 == InstrMem.size()) {
			cout << "No exit command read. Program terminated automatically." << endl;
			break;
		}

		//Decode MIPS code.
		decode_mips(instr);

		i = (reg[32] - memloc)/4;
	}
}


//Decode MIPS instruction.
void decode_mips(unsigned int instr)
{
	unsigned int op, funct;
	
	op = instr >> 26;
	funct = instr & 0x0000003f;
	printf("DEBUG: exec: pc=%0.4X\top=%d\tcode=%0.8X\n", reg[32]-4, op, instr);

	switch (op) {
		case 0:
			//R-type instruction.
			r_type(instr);
			break;
		case 4:
			//I-type instruction beq.
			i_type(instr);
			break;
		case 5:
			//I-type instruction bne.
			i_type(instr);
			break;
		case 8:
			//I-type instruction addi.
			i_type(instr);
			break;
		case 10:
			//I-type instruction slti.
			i_type(instr);
			break;
		case 2:
			//J-type instruction.
			j_type(instr);
			break;
		case 63:
			//Print instruction.
			print_reg();
			break;
		default:
			cout << "ERROR: Function code not found" << endl;
			break;
	}

}

//R-type handler.
void r_type(unsigned int instr)
{
	unsigned int funct = instr & 0x0000003f;
	unsigned int rs, rt, rd, shamt;

	rs = (instr & 0x03e00000) >> 21;
	//printf("DEBUG: rs= %d\n", rs);
	rt = (instr & 0x001f0000) >> 16;
	//printf("DEBUG: rt= %d\n", rt);
	rd = (instr & 0x0000f800) >> 11;
	//printf("DEBUG: rd= %d\n", rd);
	shamt = (instr & 0x000007c0) >> 6;
	//printf("DEBUG: shamt= %d\n", shamt);

	//Addition.
	if (funct == 32)
		reg[rd] = reg[rs] + reg[rt];

	//Subtration.
	if (funct == 34)
		reg[rd] = reg[rs] - reg[rt];

	//Shift left logical.
	if (funct == 0)
		reg[rd] = reg[rt] << shamt;

	//Shift right logical.
	if (funct == 2)
		reg[rd] = reg[rt] >> shamt;

	//Set less than.
	if (funct == 42) 
		reg[rd] = (reg[rs] < reg[rt]) ? 1 : 0;
}


//I-type handler.
void i_type(unsigned int instr)
{
	unsigned int opcode = instr >> 26;
	unsigned int rs, rt;
	int imm;

	rs = (instr & 0x03e00000) >> 21;
	//printf("DEBUG: rs= %d\n", rs);
	rt = (instr & 0x001f0000) >> 16;
	//printf("DEBUG: rt= %d\n", rt);

	if ((instr & 0x0000ffff) > 0x7fff) {
		imm = (instr & 0x0000ffff);
		imm = imm | 0xffff0000;
		//printf("DEBUG: immediate= %d\n", imm);
	}
	else {
		imm = (instr & 0x0000ffff);
		//printf("DEBUG: immediate= %d\n", imm);
	}

	//addi 8
	if (opcode == 8)
		reg[rt] = reg[rs] + imm;

	//slti 10
	if (opcode == 10)
		reg[rt] = (reg[rs] < imm) ? 1 : 0;

	//bne 5
	if (opcode == 5) {
		if (reg[rs] != reg[rt])
		reg[32] = reg[32] + (4*imm);
	}

	//beq 4
	if (opcode == 4) {
		if (reg[rs] == reg[rt])
		reg[32] = reg[32] + (4*imm);
	}
}


//J-type handler.
void j_type(unsigned int instr)
{
	unsigned int opcode = instr >> 26;
	unsigned int addr = instr & 0x03ffffff;

	//jump
	reg[32] = (addr*4);
}


//Print out the registry.
void print_reg()
{
	cout << "------------------------R E G I S T E R S------------------------------" << endl;

	for(int i=0; i<32; i++) {
		printf("reg[%0.2d] = 0x%0.8X\t", i, reg[i]);
		if ((i+1)%3 == 0)
			printf("\n");
	}
	printf("P.C.\t= 0x%0.8X", reg[32]-4);

	cout << "\n-----------------------------------------------------------------------" << endl;
}