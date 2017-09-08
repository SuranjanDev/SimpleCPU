/*
	SIMPLE CPU
	version 1.0
*/
#include<iostream>
#include<fstream>
#include<string>
#include<iomanip>
using namespace std;
/*
Structure of Main Memory
*/
struct MM {
	string addressMode;
	string opCode;
	string value;
};
/*
Structure of Memory Buffer register
*/
struct MemBuffReg {
	string addressMode;
	string op;
	int regS;
	int regD;
	int address;
}static MBR;
static int ptrSize = 7;					// variable for size of Page Table Register
static int PTR[] = { 3,5,2,0,1,4,9 };		// Page Table Register
//{ 3,5,2,0,1,4};//{0,1,2,3,4,5};
string dis;								// variable to display instructions
/*
Variables for Memory Address Register, Page No., Offset,Program Counter and Page size respectively
*/
static int MAR,PageNo,Offset,PC = 0,pgSize=4;
/*
Control Signal for Fetching, Execution and Interrupt
*/
static bool F = false,R=false;
/*
Variable to manage clock pulses
*/
static bool T[4] = { false,false,false,false };

MM* readFile();				//Function to read variables from the instruction text file
void fetchInstCycle(MM*);	//Function to fetch instruction from Memory
void fetchOprndCycle(MM*);  //Function to fetch address of operand from memory(indirect addressing)
void executeJMP(MM*);		//Function to execute Jump
void executeMOV(MM*);		//Function to execute Move
void executeADD(MM*);		//Function to execute Add
void executeJMS(MM*);		//Function to execute Jump Subroutine
void executeSTO(MM*);		//Function to execute Store
void executeLOD(MM*);		//Function to execute Load
void executeISZ(MM*);		//Function to execute Increment Skip Zero
void display();				//Function to display execution of instructions in form of table.

int main() 
{
	display();
	cin.get();
	cout << '|' << setw(15) << left << "INSTRUCTION" << '|' << setw(5) << "PC" << '|' << setw(5)<<left << "MAR" <<  '|' <<setw(10)<<left << "Page No"  << '|' << setw(10) << left << "Frame No" << '|'<< setw(10) << left << "Offset" << '|' << setw(15) << left << "Result" << '|' << endl;
	cout << '|' << setw(15) << "_______________" << '|' << setw(5) << "_____" << '|' << setw(5) << "_____" << '|' << setw(10) << "__________" << '|' << setw(10) << "__________" << '|' << setw(10) << "__________" << '|' << setw(15) << left << "_______________" << '|' << endl;
	MM* mem = readFile();							// Initialize main memory from file	

	while (MBR.op!="HLT") {							//Loop till Halt instruction
		PageNo = PC / pgSize;						// Get page number
		Offset = PC%pgSize;							// Get Offset
		if (!F && !R) {								//Go to fetch cycle if F=0 and R=0.
			fetchInstCycle(mem);
		}
		if (!F && R) {								//Go to indirect cycle if F=0 and R=1.
			fetchOprndCycle(mem);
		}
		if (F&&!R) {								//Go to execute cycle if F=1 and R=0.
			if (MBR.op == "JMP") {					
				executeJMP(mem);					// Execute Jump
			}
			else if (MBR.op == "MOV") {
				executeMOV(mem);					// Execute Move
			}
			else if (MBR.op == "ADD") {
				executeADD(mem);					// Execute Add
			}
			else if (MBR.op == "JMS") {
				executeJMS(mem);					// Execute Jump Subroutine
			}
			else if (MBR.op == "STO") {
				executeSTO(mem);					// Execute Store
			}
			else if (MBR.op == "LOD") {
				executeLOD(mem);					// Execute Load
			}
			else if (MBR.op == "ISZ") {
				executeISZ(mem);
			}
			else if(MBR.op == "HLT") {				// Stop Executing
				cout << setw(15) << "xx-END-xx" <<"|" << endl;
			}
			cout << '|' << setw(15) << "---------------" << '|' << setw(5) << "-----" << '|' << setw(5) << "-----" << '|' << setw(10) << "----------" << '|' << setw(10) << "----------" << '|' << setw(10) << "----------" << '|' << setw(15) << left << "---------------" << '|' << endl;
		}
	}
	delete [] mem;			// Delete Main Memory
	return 0;
}

void fetchInstCycle(MM * memory) {
	T[0] = true;			// set clock pulse T0 to true
	while (T[3]||T[2]||T[1]||T[0]) {
		//MAR gets the address from Program Counter
		if (T[0]){
			MAR = PTR[PageNo]*pgSize+Offset;
			T[1] = true;
			T[0] = false;
		}
		//MBR gets the address selected by MAR.	
		else if (T[1]) {
			MBR.address = stoi(memory[MAR].value);
			MBR.addressMode = memory[MAR].addressMode;
			MBR.op = memory[MAR].opCode;
			
			//Displaying the obtained Instruction						
			dis = "";
			dis += MBR.addressMode + " ";
			dis += MBR.op+" ";
			dis += memory[MAR].value;
			cout << '|'  << setw(15)<<left << dis << '|' << setw(5) <<PC << '|' << setw(5) << left << MAR << '|' << setw(10) << left << PageNo << '|' << setw(10) << left << PTR[PageNo] << '|' << setw(10) << left << Offset << '|' ;
			
			T[2] = true;
			T[1] = false;
		}
		//Program Counter gets the next instruction
		else if (T[2]) {
			PC = PC + 1;
			T[3] = true;
			T[2] = false;
		}
		//Go to fetch operand cycle if it is indirect addressing else directly jump to execution mode
		else if (T[3]) {	
			if (MBR.addressMode == "IND")
				R = true;
			else
				F = true;
			T[3] = false;
		}
	}
}

void fetchOprndCycle(MM * memory) {
	T[0] = true;		// set clock pulse T0 to true
	while (T[3] || T[2] || T[1] || T[0]) {
		//MAR gets the address of the address of the operand from MBR
		if (T[0]) {
			MAR = PTR[MBR.address / pgSize] * pgSize + MBR.address % pgSize;
			T[0] = false;
			T[1] = true;
		}		
		//MBR gets the address of the operand from the memory selected by MAR
			else if (T[1]) {
			MBR.address = stoi(memory[MAR].value);
			T[1] = false;
			T[3] = true;
		}
		//  Move to the execution cycle		
		else if (T[3]) {
			F = true;
			R = false;
			T[3] = false;
		}
	}
}

void executeJMP(MM * memory) {
	T[0] = true;		// set clock pulse T0 to true
		while (T[0] || T[1] || T[2] || T[3]) {
			 // PC gets the address from Memory Buffer Register			
			if (T[0]) {
				PC = MBR.address;				
				cout << setw(11) << left << "PC goes to " << setw(4) << PC <<"|"<< endl;
				T[0] = false;
				T[3] = true;
			}
			//	Back to fetch instruction cycle
			else if (T[3]) {
				F = false;
				T[3] = false;
			}
		}
}
void executeMOV(MM * memory) {
	T[0] = true;		//set clock pulse T0 to true
	while (T[0] || T[1] || T[2] || T[3]) {
		// Source resister gets the required value. 
		if (T[0]) {
			MBR.regS = MBR.address;
			cout << setw(13) << left << "Reg value is " << setw(2) << MBR.regS << "|" << endl;
			T[0] = false;
			T[3] = true;
		}
		// Back to fetch instruction cycle
		else if (T[3]) {
			F = false;
			T[3] = false;
		}
	}
}
void executeADD(MM * memory) {
	T[0] = true;		//set the clock pulse T0 to true
		// If addressing mode is Direct or Indirect
		if ((MBR.addressMode == "DIR")||(MBR.addressMode=="IND" )) {
		while (T[0] || T[1] || T[2] || T[3]) {
			// MAR gets the physical address of the operand
			if (T[0]) {
				MAR = PTR[MBR.address/pgSize]*pgSize+ MBR.address % pgSize;			
				T[0] = false;
				T[1] = true;
			}
			// MBR gets the value of the operand
			else if (T[1]) {
				MBR.address = stoi(memory[MAR].value);
				T[2] = true;
				T[1] = false;
			}
			// The addition takes place and the result is stored in MBR
			else if (T[2]) {
				MBR.regD = MBR.regD + MBR.address;
				cout << setw(11) << left << "Sum is " << setw(4) << MBR.regD << "|" << endl;
				T[3] = true;
				T[2] = false;
			}
			// Back to fetch instruction cycle
			else if (T[3]) {
				F = false;
				R = false;
				T[3] = false;
			}
		}
	}
		// If addressing mode is immediate
	else if (MBR.addressMode == "IMM") {
		while (T[0] || T[1] || T[2] || T[3]) {
			// Directly add the value in address part of MBR to destination register.
			if (T[0]) {
				MBR.regD = MBR.regD + MBR.address;
				cout << setw(11) << left << "Sum is " << setw(4) << MBR.regD << "|" << endl;
				T[0] = false;
				T[3] = true;
			}
			// Back to fetch instruction cycle
			else if (T[3]) {
				F = false;
				R = false;
				T[3] = false;
			}
		}
	}
}
void executeJMS(MM * memory) {
	T[0] = true;		// set clock pulse T0 to true
	while (T[0] || T[1] || T[2] || T[3]) {
		// MAR gets the address stored in MBR
		if (T[0]) {
			MAR = MBR.address;
			T[0] = false;
			T[1] = true;
		}
		// The return address is stored in the first line of subroutine and PC is incremented
		else if (T[1]) {
			memory[PTR[MAR / pgSize] * pgSize + MAR % pgSize].value =to_string(PC) ;
			PC = MBR.address + 1;
			cout << setw(11) << left << "PC goes to " << setw(4) << PC << "|" << endl;
			T[1] = false;
			T[3] = true;
		}
		// Back to fetch instruction cycle
		else if (T[3]) {
			T[3] = false;
			F = false;
		}
	}
}

void executeSTO(MM*memory) {
	T[0] = true;		// set clock pulse T0 to true
	while (T[0] || T[1] || T[2] || T[3]) {
		// MAR gets the physical address
		if (T[0]) {
			MAR = PTR[MBR.address / pgSize] * pgSize + MBR.address % pgSize;
			T[0] = false;
			T[1] = true;
		}
		// The address field of MBR gets the value of destination register
		else if (T[1]) {
			MBR.address = MBR.regD;
			T[2] = true;
			T[1] = false;
		}
		//	The value of address field of MBR is strored at memory selected by MAR
		else if (T[2]) {
			memory[MAR].value = to_string(MBR.address);			
			cout << setw(7) << left << "Stores " << setw(2)<<MBR.address <<setw(4) <<" to "<< setw(2)<<MAR <<"|" << endl;
			T[2] = false;
			T[3] = true;
		}
		// Back to fetch instruction cycle
		else if (T[3]) {
			T[3] = false;
			F = false;
		}
	}
}
void executeLOD(MM*memory) {
	T[0] = true;		//set clock pulse T0 to true
	// If addressing mode is Direct or Indirect
	if ((MBR.addressMode == "DIR") || (MBR.addressMode == "IND")) {
		while (T[0] || T[1] || T[2] || T[3]) {
			// MAR gets the physical address
			if (T[0]) {
				MAR = PTR[MBR.address / pgSize] * pgSize + MBR.address % pgSize;
				T[0] = false;
				T[1] = true;
			}
			// Destination register gets the value stored in the address
			else if (T[1]) {
				MBR.regD = stoi(memory[MAR].value);
				cout << setw(6) << left << "Loads" << setw(2) << MBR.regD << setw(3) << "to " << "Dest" << "|" << endl;
				T[1] = false;
				T[3] = true;
			}
			// Back to fetch instruction cycle
			else if (T[3]) {
				T[3] = false;
				F = false;
			}
		}
	}
	// If the addressing mode is immediate
	else if (MBR.addressMode == "IMM") {
		while (T[0] || T[1] || T[2] || T[3]) {
			// The destination register gets the value stored in the address
			if (T[0]) {
				MBR.regD = MBR.address;
				cout << setw(6) << left << "Loads" <<setw(2) <<MBR.regD << setw(3) << "to " <<"Dest" << "|" << endl;
				T[0] = false;
				T[3] = true;
			}
			// Back to fetch instruction cycle
			else if (T[3]) {
				T[3] = false;
				F = false;
			}
		}
	}
}
void executeISZ(MM*memory) {
	T[0] = true;		// set clock pulse T0 to true
	while (T[0] || T[1] || T[2] || T[3]){
		// Increment PC to next instruction if the value at source register is 0
		if (T[0]) {
			MBR.regS = MBR.regS + 1;
			cout << setw(10) << "ISZ value " << setw(5)<<MBR.regS << "|" << endl;
			if (MBR.regS == 0)
				PC = PC + 1;
			T[3] = true;
			T[0] = false;
		}
		// Back to fetch instruction cycle
		else if (T[3]) {
			T[3] = false;
			F = false;
		}
	}
}

MM* readFile() {
	ifstream infile;
	infile.open("Test1.txt");				// Open File
	if (!infile.is_open()) {
		cout << "Couldn't open file." << endl;
		return nullptr;
	}
	MM *memory = new MM[128];					// Declaring pointer of type Main Memory
	int c = 1, index = 0;
	string s;
	while (infile) {						//loop till end of instruction			
		infile >> s;		
		if (c % 3 == 1) 
			memory[PTR[index / pgSize] * pgSize + (index%pgSize)].addressMode = s;// initializing address mode
		else if (c % 3 == 2)
			memory[PTR[index/pgSize]*pgSize+(index%pgSize)].opCode = s;			// initializing operation code
		else if (c % 3 == 0) {
			memory[PTR[index/pgSize]*pgSize+(index%pgSize)].value = s;		// initializing value
			index++;			
		}
		c++;
	}
	return memory;							// return the pointer to main memory
}

void display() {
	cout << '|' << setw(20) << left << "____________________" << '|' << setw(20) << left << "____________________" << '|' << endl;
	cout << '|' << setw(20) << "OP CODES" << '|' << setw(20) << left << "ACTION" << '|' << endl;
	cout << '|' << setw(20) << "____________________" << '|' << setw(20) << "____________________" << '|' << endl;
	cout << '|' << setw(20) << "ADD" << '|' << setw(20) << left << "ADDITION" << '|' << endl;
	cout << '|' << setw(20) << "---------------" << '|' << setw(20) << "---------------" << '|' << endl;
	cout << '|' << setw(20) << "HLT" << '|' << setw(20) << left << "HALT(STOP)" << '|' << endl;
	cout << '|' << setw(20) << "---------------" << '|' << setw(20) << "---------------" << '|' << endl;
	cout << '|' << setw(20) << "ISZ" << '|' << setw(20) << left << "INCREMENT SKIP ZERO" << '|' << endl;
	cout << '|' << setw(20) << "---------------" << '|' << setw(20) << "---------------" << '|' << endl;
	cout << '|' << setw(20) << "JMP" << '|' << setw(20) << left << "JUMP" << '|' << endl;
	cout << '|' << setw(20) << "---------------" << '|' << setw(20) << "---------------" << '|' << endl;
	cout << '|' << setw(20) << "JMS" << '|' << setw(20) << left << "JUMP SUBROUTINE" << '|' << endl;
	cout << '|' << setw(20) << "---------------" << '|' << setw(20) << "---------------" << '|' << endl;
	cout << '|' << setw(20) << "MOV" << '|' << setw(20) << left << "MOVE" << '|' << endl;
	cout << '|' << setw(20) << "---------------" << '|' << setw(20) << "---------------" << '|' << endl;
	cout << '|' << setw(20) << "LOD" << '|' << setw(20) << left << "LOAD" << '|' << endl;
	cout << '|' << setw(20) << "---------------" << '|' << setw(20) << "---------------" << '|' << endl;
	cout << '|' << setw(20) << "STO" << '|' << setw(20) << left << "STORE" << '|' << endl;
	cout << '|' << setw(20) << "---------------" << '|' << setw(20) << "---------------" << '|' << endl << endl;
	cout << '|' << setw(20) << "ADDRESSING MODE" << '|' << setw(20) << left << "ACTION" << '|' << endl;
	cout << '|' << setw(20) << "____________________" << '|' << setw(20) << "____________________" << '|' << endl;
	cout << '|' << setw(20) << "DIR" << '|' << setw(20) << left << "DIRECT" << '|' << endl;
	cout << '|' << setw(20) << "---------------" << '|' << setw(20) << "---------------" << '|' << endl;
	cout << '|' << setw(20) << "IMM" << '|' << setw(20) << left << "IMMEDIATE" << '|' << endl;
	cout << '|' << setw(20) << "---------------" << '|' << setw(20) << "---------------" << '|' << endl;
	cout << '|' << setw(20) << "IND" << '|' << setw(20) << left << "INDIRECT" << '|' << endl;
	cout << '|' << setw(20) << "---------------" << '|' << setw(20) << "---------------" << '|' << endl;
	cout << '|' << setw(20) << "RRI" << '|' << setw(20) << left << "REGISTER REFERENCE" << '|' << endl;
	cout << '|' << setw(20) << "---------------" << '|' << setw(20) << "---------------" << '|' << endl << endl;
	cout << "PAGE TABLE REGISTER" << endl;
	cout << '|' << setw(10) << "PAGE NO" << '|' << setw(10) << left << "FRAME NO" << '|' << endl;
	cout << '|' << setw(10) << "__________" << '|' << setw(10) << "__________" << '|' << endl;
	for (int i = 0; i < ptrSize; i++) {

		cout << '|' << setw(10) << i << '|' << setw(10) << left << PTR[i] << '|' << endl;
		cout << '|' << setw(10) << "---------" << '|' << setw(10) << "----------" << '|' << endl;
	}
	cout <<endl << " PAGE SIZE IS " << pgSize << endl << endl;
	cout << endl << "PRESS ENTER TO BEGIN" << endl;
}