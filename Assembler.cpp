#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <ctype.h>
#include <cstdlib>
using namespace std;
char ASM_FILE_NAME[] = "assembly.asm";
const int RUN = 0;
const int DEBUG = 1;
const int MODE = RUN;

//user prompts for the GET and PUT commands
string getPrompt = "Input Number: ";
string putPrompt = "AX is ";

//parameter pointer
int param = 0;

const int MAX = 500;  //size of simulators memory
const int COL = 7;	 //number of columns for output

					 //REGISTERS
const int AXREG = 0;
const int BXREG = 1;
const int CXREG = 2;
const int DXREG = 3;

//COMMANDS
const int HALT = 5;
const int GET = 6;
const int PUT = 7;
const int MOVREG = 192;
const int ADD = 160;
const int SUB = 128;
const int CMP = 96;
const int MOVMEM = 224;

//extra
const int GETC = 1;
const int PUTC = 2;

//jump instructions
const int JE = 8;
const int JNE = 9;
const int JB = 10;
const int JBE = 11;
const int JA = 12;
const int JAE = 13;
const int JMP = 14;

//function commands
const int FUN = 3;
const int RET = 4;

//oper1 & oper2
const int CONSTANT = 7;
const int MEMORY_ADDRESS = 6;
const int BXMEM = 4;
const int BXPLUS = 5;

//Types for the opperands
enum paramType { reg, mem, constant, arrayBx, arrayBxPlus, none };

typedef short int Memory;

class Registers
{
public:
	int AX;
	int BX;
	int CX;
	int DX;
	int instrAddr;
	int flag;
} regis;

Memory memory[MAX] = { 0 }; //global
int address;
int stack;

void printMemoryDump(); //prints memeory with integers commands
void convertToMachineCode(ifstream &fin); //Converts each asm line into machine code
void convertToNumber(string line, int &start, int &value); //Converts (sub)string to number
int whichReg(char regLetter);  //returns the number of the letter registar
void fillMemory(); //Reads in asm file and puts it into memory
void runCode(); //Takes in machine code and runs the corresponding command
void changeToLowerCase(string &line); //Change string to lowercase
void splitCommand(string line, string &command, string &oper1, string &oper2); //splits the assembly line into 3 parts: command, oper1, oper2
int processOper(string oper); //returns the machine code based on the x86 instructions
void setFlag(int i); // sets the flag
int whichJump(string command); // determines which jump to use
int& getReg(int reg); //Gets the register's value and returns by reference

					  //other memory functions
void putMemory(int i);
void putMemoryLocation(string oper2);
void putMemoryConstant(string oper2);
void putBXPLUS(string oper2);

void setStack();
void getStack();


int main() //starts the program
{
	fillMemory();
	printMemoryDump();
	runCode();
	printMemoryDump();
	if (MODE == DEBUG) {
		cout << "\nAll done\n" << endl;
	}
	system("pause");
	return 0;
}

void convertToMachineCode(ifstream &fin)
{

	string line, command, oper1, oper2;
	int decimal = 0; //Final machine code for the entire command (in decimal format)
	getline(fin, line, '\n');
	changeToLowerCase(line);
	splitCommand(line, command, oper1, oper2); //splits command into different parts

	const int oper1Code = processOper(oper1); //Machine code for Operand 1
	const int oper2Code = processOper(oper2); //Machine code for Operand 2
	int memoryLocation; //which memory location if oper2 is a memory location ([xxxx])

	if (param != 0) {
		param -= 1;
		if (isdigit(command[0])) {
			putMemory(CONSTANT);
			putMemoryConstant(command);
		}
		else if (command[0] == '[') {
			putMemory(MEMORY_ADDRESS);
			putMemoryLocation(command);
		}
		else if (whichReg(command[0]) != -1) {
			putMemory(whichReg(command[0]));
		}
		else {
			cout << command << endl;
		}
	}
	else {

		if (command[0] == 'h') { // HALT
			putMemory(HALT);
		}
		else if (command[0] == 'm') {
			if (oper1Code == MEMORY_ADDRESS || oper1Code == BXMEM || oper1Code == BXPLUS) {
				decimal += MOVMEM;
				decimal += oper1Code;
				decimal += (oper2Code << 3); //this 'switches' oper1 and oper2 around
				putMemory(decimal);

				if (oper1Code == MEMORY_ADDRESS) {
					putMemoryLocation(oper1);
				}
				else if (oper1Code == BXPLUS) {
					putBXPLUS(oper1);
				}
			}
			else {
				decimal += MOVREG;
				decimal += (oper1Code << 3);
				decimal += oper2Code;
				putMemory(decimal);
				if (oper2Code == CONSTANT) { //oper2 is a constant
					putMemoryConstant(oper2);
				}
				else if (oper2Code == MEMORY_ADDRESS) { //oper2 contains brackets
					putMemoryLocation(oper2);
				}
				else if (oper2Code == BXPLUS) {
					putBXPLUS(oper2);
				}
			}
		}
		else if (command[0] == 'a') { // ADD
			decimal += ADD;
			decimal += (oper1Code << 3);
			decimal += oper2Code;

			putMemory(decimal);

			if (oper2Code == CONSTANT) { // oper2 is a constant
				putMemoryConstant(oper2);
			}
			else if (oper2Code == MEMORY_ADDRESS) { //oper2 contains brackets
				putMemoryLocation(oper2);
			}
			else if (oper2Code == BXPLUS) {
				putBXPLUS(oper2);
			}
		}
		else if (command[0] == 's') { // SUB
			decimal += SUB;
			decimal += (oper1Code << 3);
			decimal += oper2Code;
			putMemory(decimal);
			if (oper2Code == CONSTANT) { // oper2 is a constant
				putMemoryConstant(oper2);
			}
			else if (oper2Code == MEMORY_ADDRESS) { //oper2 contains brackets
				putMemoryLocation(oper2);
			}
			else if (oper2Code == BXPLUS) {
				putBXPLUS(oper2);
			}
		}
		else if (command[0] == 'c') {
			decimal += CMP;
			decimal += (oper1Code << 3);
			decimal += oper2Code;
			putMemory(decimal);
			if (oper2Code == CONSTANT) { // oper2 is a constant
				putMemoryConstant(oper2);
			}
			else if (oper2Code == MEMORY_ADDRESS) {
				putMemoryLocation(oper2);
			}
			else if (oper2Code == BXPLUS) {
				putBXPLUS(oper2);
			}
		}
		else if (command[0] == 'p' && command[3] == 'c') {	// PUTC
			putMemory(PUTC);
		}
		else if (command[0] == 'p') {						// PUT
			putMemory(PUT);
		}
		else if (command[0] == 'g' && command[3] == 'c') { // GETC
			putMemory(GETC);
		}
		else if (command[0] == 'g') {					   // GET
			putMemory(GET);
		}
		else if (command[0] == 'j') {					   // J??
			putMemory(whichJump(command));
			putMemoryLocation(oper1);
		}
		else if (command[0] == 'f') {					   // FUN
			putMemory(FUN);
			putMemoryLocation(oper1);
			putMemoryConstant(oper2);
			param = atoi(oper2.c_str());
		}
		else if (command[0] == 'r') {					  // RET
			putMemory(RET);
			putMemoryLocation(oper1);
		}
		else if (isdigit(command[0])) {					 // LONE CONSTSNT
			putMemoryConstant(command);
		}
		else {
			address++;
		}
	}
}

/************************************************************/
/*runCode														*/
/*Takes in machine code and executes it based on the x86 Instruction Encoding*/
void runCode()
{
	Memory command;
	Memory topBits, midBits, bottomBits, reg; //seperates machine code into command, oper1, oper2
	address = 0;
	stack = MAX - 1;
	while (memory[address] != 5) { //while line is not halt
		if (MODE == DEBUG) {
			printMemoryDump();
		}
		command = memory[address];
		address++;
		topBits = command & 224;
		midBits = ((command & 24) >> 3);
		bottomBits = command & 7;
		switch (topBits) {
		case MOVREG:
			reg = midBits;
			if (bottomBits < 4) //it is a register
			{
				getReg(reg) = getReg(bottomBits);
			}
			else if (bottomBits == CONSTANT) {
				getReg(reg) = memory[address];
				address++;
			}
			else if (bottomBits == MEMORY_ADDRESS) //memory location
			{
				getReg(reg) = memory[memory[address]];
				address++;
			}
			else if (bottomBits == BXMEM) {
				getReg(reg) = memory[regis.BX];
			}
			else if (bottomBits == BXPLUS) {
				getReg(reg) = memory[regis.BX + memory[address]];
			}
			break;
		case MOVMEM:
			if (bottomBits == MEMORY_ADDRESS) {
				memory[memory[address]] = getReg(midBits);
				address++;
			}
			else if (bottomBits == BXMEM) {
				memory[regis.BX] = getReg(midBits);
			}
			else if (bottomBits == BXPLUS) {
				memory[regis.BX + memory[address]] = getReg(midBits);
			}
			break;
		case ADD:
			reg = midBits;
			if (bottomBits == CONSTANT) { // constant
				getReg(reg) += memory[address];
				address++;
			}
			else if (bottomBits < 4) { //register
				getReg(reg) += getReg(bottomBits);
			}
			else if (bottomBits == MEMORY_ADDRESS) //memory location
			{
				getReg(reg) += memory[memory[address]];
				address++;
			}
			else if (bottomBits == BXMEM) {
				getReg(reg) += memory[regis.BX];
			}
			else if (bottomBits == BXPLUS) {
				getReg(reg) += memory[regis.BX + memory[address]];
			}
			break;
		case SUB:
			reg = midBits;
			if (bottomBits == CONSTANT) { // constant
				getReg(reg) -= memory[address];
				address++;
			}
			else if (bottomBits < 4) { //register
				getReg(reg) -= getReg(bottomBits);
			}
			else if (bottomBits == MEMORY_ADDRESS) //memory location
			{
				getReg(reg) -= memory[memory[address]];
				address++;
			}
			else if (bottomBits == BXMEM) {
				getReg(reg) -= memory[regis.BX];
			}
			else if (bottomBits == BXPLUS) {
				getReg(reg) -= memory[regis.BX + memory[address]];
			}
			break;
		case CMP:
			int cmp1 = 0;
			int cmp2 = 0;

			//find cmp1
			cmp1 = getReg(midBits);

			//find cmp2
			if (bottomBits == CONSTANT) {
				cmp2 = memory[address];
				address++;
			}
			else if (bottomBits == MEMORY_ADDRESS) {
				cmp2 = memory[memory[address]];
				address++;
			}
			else if (bottomBits == BXMEM) {
				cmp2 = memory[regis.BX];
				address++;
			}
			else if (bottomBits == BXPLUS) {
				cmp2 = memory[regis.BX + memory[address]];
				address++;
			}
			else if (bottomBits < 4) { //register
				cmp2 = getReg(bottomBits);
			}

			//do the comparing
			if (cmp1 > cmp2) {
				setFlag(1);
			}
			else if (cmp1 == cmp2) {
				setFlag(0);
			}
			else {
				setFlag(-1);
			}
			break;
		}

		if (command == FUN) {
			int location = memory[address];
			int params = memory[address + 1];
			address += 2;
			//place parameters
			int paramCounter = location - 1;
			for (int i = 0; i < params; i++) {
				if (memory[address] < 4) //register
				{
					memory[paramCounter] = getReg(memory[address]);
					paramCounter--;
					address++;
				}
				else if (memory[address] == CONSTANT) {
					address++;
					memory[paramCounter] = memory[address];
					paramCounter--;
					address++;
				}
				else if (memory[address] == MEMORY_ADDRESS) {
					address++;
					memory[paramCounter] = memory[memory[address]];
					paramCounter--;
					address++;
				}
			}

			setStack();
			address = location;

		}
		else if (command == RET) {
			memory[memory[address]] = regis.AX;
			address++;
			getStack();
		}
		else if (command >= 8 && command <= 14) //jump instructions
		{
			switch (command) {
			case JE:
				if (regis.flag == 0) {
					address = memory[address];
				}
				else {
					address++;
				}
				break;
			case JNE:
				if (regis.flag != 0) {
					address = memory[address];
				}
				else {
					address++;
				}
				break;
			case JB:
				if (regis.flag < 0) {
					address = memory[address];
				}
				else {
					address++;
				}
				break;
			case JBE:
				if (regis.flag <= 0) {
					address = memory[address];
				}
				else {
					address++;
				}
				break;
			case JA:
				if (regis.flag > 0) {
					address = memory[address];
				}
				else {
					address++;
				}
				break;
			case JAE:
				if (regis.flag >= 0) {
					address = memory[address];
				}
				else {
					address++;
				}
				break;
			case JMP:
				address = memory[address];
				break;
			}
		}
		else if (command == PUT) {
			cout << putPrompt << regis.AX << endl;
		}
		else if (command == GET) {
			cout << getPrompt;
			cin >> regis.AX;
		}
		else if (command == PUTC) {
			cout << (char)regis.AX;
		}

		else if (command == GETC) {
			char input;
			cin >> input;
			regis.AX = (int)input;
		}
		regis.instrAddr = address;
	}

}
/************************************************************/
/*processOper														*/
/*	returns Machinge code for oper1 and oper2, which will later be bitshifted*/
int processOper(string oper) {
	if (whichReg(oper[0]) != -1) {
		return whichReg(oper[0]);
	}
	else if (oper[0] == '[') {
		if (oper[1] == 'b') {
			if (oper[3] == '+') {
				return BXPLUS;
			}
			return BXMEM;
		}
		return MEMORY_ADDRESS;
	}
	return CONSTANT;
}


/************************************************************/
/*fillMemory												*/
/*		changes the code to machine code and places the     */
/*commands into the memory. */
void fillMemory()
{
	int linesOfCode = 0; //# of lines in the asm file
	address = 0;
	ifstream fin;
	fin.open( ASM_FILE_NAME );
	if (fin.fail( ))
	{
	std::cout << "Error, file didn't open\n";
	//system( "pause" );
	//exit( 1 );
	}
	else {
		string line;
		//Get lines of code
		ifstream lineGetter;
		lineGetter.open(ASM_FILE_NAME);
		while (getline(lineGetter, line))
			++linesOfCode;
		lineGetter.close();
	}

	for (int i = 0; i< linesOfCode; i++)
	{
	convertToMachineCode(fin);
	}
}

void splitCommand(string line, string &command, string &oper1, string &oper2) {
	int space = line.find(' ');
	command.assign(line, 0, space);
	line.erase(0, space + 1);
	space = line.find(' ');
	oper1.assign(line, 0, space);
	line.erase(0, space + 1);
	space = line.find(' ');
	oper2.assign(line, 0, space);
}


/************************************************************/
/*whichReg																	*/
/*		changes the letter of the registar to a number.			*/

int whichReg(char regLetter)
{
	if (regLetter == 'a')
	{
		return AXREG;
	}
	else if (regLetter == 'b')
	{
		return BXREG;
	}
	else if (regLetter == 'c')
	{
		return CXREG;
	}
	else if (regLetter == 'd')
	{
		return DXREG;
	}
	return -1;  //something went wrong if -1 is returned
}


/************************************************************/
/*ConvertToNumber															*/
/*  takes in a line and converts digits to a integer			*/
/*  line - is the string of assembly code to convert			*/
/*  start - is the location where the line is being coverted,
		it starts at the beginning of number and it passed
		back at the next location */
/*  value - is the integer value of the digits in the code	*/
void convertToNumber(string line, int &start, int &value)
{
	char number[16];
	bool negative = false;
	//	cout<< "in convertToNumber before function 1  start is "<<start<<endl;
	int i = 0;
	if (line[start] == '-')
	{
		start++;
		negative = true;
	}
	while (i<16 && line.size() > (unsigned int) start&& isdigit(line[start]))
	{
		number[i] = line[start];
		//	cout<<line[start];
		i++;
		start++;
		//		cout<<i<<start;
	}
	number[i] = '\0';
	value = atoi(number);
	if (negative == true)
	{
		value = -value;
	}
	//	cout<< "in convertToNumber after function 1  start is "<<start<<endl;
}


/************************************************************/
/*	printMemoryCommands													*/
/*	prints memory with letter commands								*/
void printMemoryCommands()
{
	int i = 0;
	int count;  // which column is currently outputting
	int moveColumn = MAX / COL;
	int index = 0;
	while (i< MAX)
	{
		count = 0;
		while (count < COL && index < MAX)
		{
			index = i + count * moveColumn;
			//As each type of instruction is added you will need to be able to deal with type of command
			//printcommand(index)
		}
		std::cout << endl;

	}
	std::cout << "RA " << regis.AX << "  BX " << regis.BX << "  CX " << regis.CX << "  DX " << regis.DX << endl;
	std::cout << "\tinstruction " << regis.instrAddr << "\tflag " << regis.flag << "\n\n";
}


/************************************************************/
/*printMemoryDump															*/
/*prints memory by number												*/
/*MAX is the amount of elements in the memory array (Vicki used 100) */
/*COL is the number of columns that are to be displayed (Vicki used 7; was originally called COLUMNS) */
void printMemoryDump()
{
	int numRows = MAX / COL + 1;   //number of rows that will print
	int carryOver = MAX%COL;   //number of columns on the bottom row
	int location;   //the current location being called
	cout << endl;
	for (int row = 0; row<numRows; row++)
	{
		location = row;
		//cout << setw( 3 ) << location << "." << setw( 5 ) << memory[location];
		for (int column = 0; location<MAX&&column<COL; column++)
		{
			if (!(numRows - 1 == row&&carryOver - 1<column))
			{
				std::cout << setw(5) << location << "." << setw(5) << memory[location];
				location += (numRows - (carryOver - 1<column));
			}
		}
		std::cout << endl;
		//cout<<setw(3);
	}

	std::cout << endl;
	std::cout << "   AX: " << regis.AX << '\t';
	std::cout << "   BX: " << regis.BX << '\t';
	std::cout << "   CX: " << regis.CX << '\t';
	std::cout << "   DX: " << regis.DX << '\t';
	std::cout << endl << endl;
	std::cout << "   Instruction: " << regis.instrAddr << endl;
	std::cout << "   Flag: " << regis.flag;

	std::cout << endl << endl;
}

/**********************/
/*changeToLowerCase*/
/*	Makes a string lowercase*/
void changeToLowerCase(string &line)
{

	unsigned int index = 0;
	while (index < line.size())
	{
		line[index] = tolower(line[index]);
		index++;
	}
	//	cout<<"the line in change"<<line;

}
/****************************************/
//setFlag
//Sets the register's flag
void setFlag(int i) {
	regis.flag = i;
}

/********************************************
*	getReg
*	determines the value of the register you
	are referring to and returns by reference

********************************************/
int& getReg(int reg) {
	switch (reg) {
	case AXREG:
		return regis.AX;
		break;
	case BXREG:
		return regis.BX;
		break;
	case CXREG:
		return regis.CX;
		break;
	case DXREG:
		return regis.DX;
		break;
	}
	return regis.AX;
}

/****************************************

*whichJump
*Determines which jump to use

*****************************************/
int whichJump(string command)
{
	if (command == "je") {
		return JE;
	}
	else if (command == "jne") {
		return JNE;
	}
	else if (command == "jb") {
		return JB;
	}
	else if (command == "jbe") {
		return JBE;
	}
	else if (command == "ja") {
		return JA;
	}
	else if (command == "jae") {
		return JAE;
	}
	else if (command == "jmp") {
		return JMP;
	}
	return 0;
}

void putMemoryLocation(string oper2) {
	memory[address] = atoi(oper2.substr(1, oper2.length() - 2).c_str());
	address++;
}

void putMemory(int i) {
	memory[address] = i;
	address++;
}

void putMemoryConstant(string oper2) {
	memory[address] = std::atoi(oper2.c_str());
	address++;
}

void putBXPLUS(string oper2) {
	memory[address] = atoi(oper2.substr(4, oper2.length() - 2).c_str());
	address++;
}
//name,  title, place of employment, address, phone, email
void setStack() {
	//stack = MAX - 1;
	memory[stack] = regis.AX;
	stack --;
	memory[stack] = regis.BX;
	stack --;
	memory[stack] = regis.CX;
	stack --;
	memory[stack] = regis.DX;
	stack --;
	memory[stack] = regis.flag;
	stack --;
	memory[stack] = address;
	stack --;
}

void getStack() {
	stack++;
	address = memory[stack];
	stack++;
	regis.flag = memory[stack];
	stack++;
	regis.DX = memory[stack];
	stack++;
	regis.CX = memory[stack];
	stack++;
	regis.BX = memory[stack];
	stack++;
	regis.AX = memory[stack];

	/*regis.AX = memory[MAX - 1];
	regis.BX = memory[MAX - 2];
	regis.CX = memory[MAX - 3];
	regis.DX = memory[MAX - 4];
	regis.flag = memory[MAX - 5];
	address = memory[MAX - 6];*/

}
