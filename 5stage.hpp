/**
 * @file MIPS_Processor.hpp
 * @author Mallika Prabhakar and Sayam Sethi
 *
 */

#ifndef __MIPS_PROCESSOR_HPP__
#define __MIPS_PROCESSOR_HPP__
using namespace std;
#include <unordered_map>
#include <string>
#include <functional>
#include <vector>
#include <fstream>
#include <exception>
#include <iostream>
#include <boost/tokenizer.hpp>
/*
signature of PipelineReg - 

IF/ID - 1. PC + 4
	2. Fetched unparsed instruction 

ID/ALU - 1. WB.CONTROL
	 2. M.CONTROL
	 3. EX.CONTROL 
	 4. NEW_PC
	 5. RS
	 6. RT 
	 7. IMMEDIATE 
	 8. RD

ALU/MEM	1. WB.CONTROL
	2. MEM.CONTROL
	3. NEW_PC 
	4. ZERO 
	5. ALU_RESULT 
	6. RT
	7. RD 

MEM/WB 	1. WB.CONTROL
	2. ALU_RESULT
	3. MEM_READ
	4. RD
*/

struct MIPS_Architecture
{
	struct WBctr{
		bool RegWrite;
		bool MemtoReg;

		void print(struct WBctr* ptr){
			if (ptr == NULL){return ; }
			cout << "RegWrite" << ptr->RegWrite  << "\n" ; 
			cout << "MemtoReg" << ptr->MemtoReg  << "\n" ; 
		}
	};
	
	struct MEMctr{
		
		bool MemRead;
		bool MemWrite;
		

		void print(struct MEMctr* ptr){
			if (ptr == NULL){return ; }
			
			cout << "MemRead" << ptr->MemRead  << "\n" ; 
			cout << "MemWrite" << ptr->MemWrite  << "\n" ; 
			
		}

	};
	struct EXctr{
		bool branch;
		bool branch_not ; 
		bool RegDst;
		int OP; // 0 denotes add , 1 denotes subtraction, 2 denotes multiplication
		bool ALUSrc ; // true if alu src has to be immediate field

		void print(struct EXctr* ptr){
			if (ptr == NULL){return ; }
			cout << "branch" << ptr->branch  << "\n" ; 
			cout << "branch not" << ptr->branch_not  << "\n" ; 
			cout << "RegDst" << ptr->RegDst  << "\n" ; 
			cout << "OP" << ptr->OP  << "\n" ; 
			cout << "ALUSrc" << ptr->ALUSrc  << "\n" ; 
		}

	};
	typedef struct WBctr WBctr;
	typedef struct MEMctr MEMctr;
	typedef struct EXctr EXctr;
	struct PipelineReg
{
	std::vector<string> currcomand;
	int at_PC = -1;
	int PC_new = -1;
/*control signals */
	WBctr* WB = NULL;
	MEMctr* MEM = NULL;
	EXctr* EX = NULL;
/*saved results */
	int ALURESULT;
	int Read_data;
	int immediate;
	bool zero = (ALURESULT == 0) ;
	int RegReadData1;
	int RegReadData2;

	int RS,RT,RD;

	void print(struct  PipelineReg* ptr)  {
		if (ptr == NULL){return ; }
		cout << "at_PC" << ptr->at_PC << "\n" ; 
		cout << "PC_new" << ptr->PC_new << "\n" ;
		cout << "ALURESULT" << ptr->ALURESULT << "\n" ;
		cout << "Read_data" << ptr->Read_data << "\n" ;
		cout << "immediate" << ptr->immediate << "\n" ;
		cout << "zero" << ptr->zero << "\n" ;
		cout << "RegReadData1" << ptr->RegReadData1 << "\n" ;
		cout << "RegReadData2" << ptr->RegReadData2 << "\n" ;
		cout << "RS" << ptr->RS << "\n" ;
		cout << "RT" << ptr->RT << "\n" ;
		cout << "RD" << ptr->RD << "\n" ;
		cout << "printing control signals" << '\n' ;
		
		if (ptr->EX)  (ptr->EX)->print(ptr->EX) ;
		if (ptr->MEM) (ptr->MEM)->print(ptr->MEM) ; 
		if (ptr->WB) (ptr->WB)->print(ptr->WB) ; 
		
		  

	}
};
	typedef struct PipelineReg pr;
	


	int registers[32] = {0}, PCcurr = 0, PCnext;
	std::unordered_map<std::string, std::function<int(MIPS_Architecture &, std::string, std::string, std::string)>> instructions;
	std::unordered_map<std::string, int> registerMap, address;

	std::unordered_map<int, int> memoryDelta;

	static const int MAX = (1 << 20);
	int data[MAX >> 2] = {0};
	std::vector<std::vector<std::string>> commands;
	std::vector<int> commandCount;
	enum exit_code
	{
		SUCCESS = 0,
		INVALID_REGISTER,
		INVALID_LABEL,
		INVALID_ADDRESS,
		SYNTAX_ERROR,
		MEMORY_ERROR
	};

	// constructor to initialise the instruction set
	MIPS_Architecture(std::ifstream &file)
	{
		instructions = {{"add", &MIPS_Architecture::add}, {"sub", &MIPS_Architecture::sub}, {"mul", &MIPS_Architecture::mul}, {"beq", &MIPS_Architecture::beq}, {"bne", &MIPS_Architecture::bne}, {"slt", &MIPS_Architecture::slt}, {"j", &MIPS_Architecture::j}, {"lw", &MIPS_Architecture::lw}, {"sw", &MIPS_Architecture::sw}, {"addi", &MIPS_Architecture::addi}};

		for (int i = 0; i < 32; ++i)
			registerMap["$" + std::to_string(i)] = i;
		registerMap["$zero"] = 0;
		registerMap["$at"] = 1;
		registerMap["$v0"] = 2;
		registerMap["$v1"] = 3;
		for (int i = 0; i < 4; ++i)
			registerMap["$a" + std::to_string(i)] = i + 4;
		for (int i = 0; i < 8; ++i)
			registerMap["$t" + std::to_string(i)] = i + 8, registerMap["$s" + std::to_string(i)] = i + 16;
		registerMap["$t8"] = 24;
		registerMap["$t9"] = 25;
		registerMap["$k0"] = 26;
		registerMap["$k1"] = 27;
		registerMap["$gp"] = 28;
		registerMap["$sp"] = 29;
		registerMap["$s8"] = 30;
		registerMap["$ra"] = 31;

		constructCommands(file);
		commandCount.assign(commands.size(), 0);
	}

	// perform add operation
	int add(std::string r1, std::string r2, std::string r3)
	{
		return op(r1, r2, r3, [&](int a, int b)
				  { return a + b; });
	}

	// perform subtraction operation
	int sub(std::string r1, std::string r2, std::string r3)
	{
		return op(r1, r2, r3, [&](int a, int b)
				  { return a - b; });
	}

	// perform multiplication operation
	int mul(std::string r1, std::string r2, std::string r3)
	{
		return op(r1, r2, r3, [&](int a, int b)
				  { return a * b; });
	}

	// perform the binary operation
	int op(std::string r1, std::string r2, std::string r3, std::function<int(int, int)> operation)
	{
		if (!checkRegisters({r1, r2, r3}) || registerMap[r1] == 0)
			return 1;
		registers[registerMap[r1]] = operation(registers[registerMap[r2]], registers[registerMap[r3]]);
		PCnext = PCcurr + 1;
		return 0;
	}

	// perform the beq operation
	int beq(std::string r1, std::string r2, std::string label)
	{
		return bOP(r1, r2, label, [](int a, int b)
				   { return a == b; });
	}

	// perform the bne operation
	int bne(std::string r1, std::string r2, std::string label)
	{
		return bOP(r1, r2, label, [](int a, int b)
				   { return a != b; });
	}

	// implements beq and bne by taking the comparator
	int bOP(std::string r1, std::string r2, std::string label, std::function<bool(int, int)> comp)
	{
		if (!checkLabel(label))
			return 4;
		if (address.find(label) == address.end() || address[label] == -1)
			return 2;
		if (!checkRegisters({r1, r2}))
			return 1;
		PCnext = comp(registers[registerMap[r1]], registers[registerMap[r2]]) ? address[label] : PCcurr + 1;
		return 0;
	}

	// implements slt operation
	int slt(std::string r1, std::string r2, std::string r3)
	{
		if (!checkRegisters({r1, r2, r3}) || registerMap[r1] == 0)
			return 1;
		registers[registerMap[r1]] = registers[registerMap[r2]] < registers[registerMap[r3]];
		PCnext = PCcurr + 1;
		return 0;
	}

	// perform the jump operation
	int j(std::string label, std::string unused1 = "", std::string unused2 = "")
	{
		if (!checkLabel(label))
			return 4;
		if (address.find(label) == address.end() || address[label] == -1)
			return 2;
		PCnext = address[label];
		return 0;
	}

	// perform load word operation
	int lw(std::string r, std::string location, std::string unused1 = "")
	{
		if (!checkRegister(r) || registerMap[r] == 0)
			return 1;
		int address = locateAddress(location);
		if (address < 0)
			return abs(address);
		registers[registerMap[r]] = data[address];
		PCnext = PCcurr + 1;
		return 0;
	}

	// perform store word operation
	int sw(std::string r, std::string location, std::string unused1 = "")
	{
		if (!checkRegister(r))
			return 1;
		int address = locateAddress(location);
		if (address < 0)
			return abs(address);
		data[address] = registers[registerMap[r]];
		PCnext = PCcurr + 1;
		return 0;
	}

	// for strings of form 56($s1) ; 
	int locateAddress(std::string location)
	{
		if (location.back() == ')')
		{
			try
			{
				int lparen = location.find('('), offset = stoi(lparen == 0 ? "0" : location.substr(0, lparen));
				std::string reg = location.substr(lparen + 1);
				reg.pop_back();
				if (!checkRegister(reg))
					return -3;
				int address = registers[registerMap[reg]] + offset;
				if (address % 4 || address < int(4 * commands.size()) || address >= MAX)
					return -3;
				return address / 4;
			}
			catch (std::exception &e)
			{
				return -4;
			}
		}
		try
		{
			int address = stoi(location);
			if (address % 4 || address < int(4 * commands.size()) || address >= MAX)
				return -3;
			return address / 4;
		}
		catch (std::exception &e)
		{
			return -4;
		}
	}

	// perform add immediate operation
	int addi(std::string r1, std::string r2, std::string num)
	{
		if (!checkRegisters({r1, r2}) || registerMap[r1] == 0)
			return 1;
		try
		{
			registers[registerMap[r1]] = registers[registerMap[r2]] + stoi(num);
			PCnext = PCcurr + 1;
			return 0;
		}
		catch (std::exception &e)
		{
			return 4;
		}
	}

	// checks if label is valid
	inline bool checkLabel(std::string str)
	{
		return str.size() > 0 && isalpha(str[0]) && all_of(++str.begin(), str.end(), [](char c)
														   { return (bool)isalnum(c); }) &&
			   instructions.find(str) == instructions.end();
	}

	// checks if the register is a valid one
	inline bool checkRegister(std::string r)
	{
		return registerMap.find(r) != registerMap.end();
	}

	// checks if all of the registers are valid or not
	bool checkRegisters(std::vector<std::string> regs)
	{
		return std::all_of(regs.begin(), regs.end(), [&](std::string r)
						   { return checkRegister(r); });
	}

	/*
		handle all exit codes:
		0: correct execution
		1: register provided is incorrect
		2: invalid label
		3: unaligned or invalid address
		4: syntax error
		5: commands exceed memory limit
	*/
	void handleExit(exit_code code, int cycleCount)
	{
		std::cout << '\n';
		switch (code)
		{
		case 1:
			std::cerr << "Invalid register provided or syntax error in providing register\n";
			break;
		case 2:
			std::cerr << "Label used not defined or defined too many times\n";
			break;
		case 3:
			std::cerr << "Unaligned or invalid memory address specified\n";
			break;
		case 4:
			std::cerr << "Syntax error encountered\n";
			break;
		case 5:
			std::cerr << "Memory limit exceeded\n";
			break;
		default:
			break;
		}
		if (code != 0)
		{
			std::cerr << "Error encountered at:\n";
			for (auto &s : commands[PCcurr])
				std::cerr << s << ' ';
			std::cerr << '\n';
		}
		std::cout << "\nFollowing are the non-zero data values:\n";
		for (int i = 0; i < MAX / 4; ++i)
			if (data[i] != 0)
				std::cout << 4 * i << '-' << 4 * i + 3 << std::hex << ": " << data[i] << '\n'
						  << std::dec;
		std::cout << "\nTotal number of cycles: " << cycleCount << '\n';
		std::cout << "Count of instructions executed:\n";
		for (int i = 0; i < (int)commands.size(); ++i)
		{
			std::cout << commandCount[i] << " times:\t";
			for (auto &s : commands[i])
				std::cout << s << ' ';
			std::cout << '\n';
		}
	}
	
	void print_vector(vector<string> vi){
		for (auto i = vi.begin() ; i != vi.end() ; i ++){
			cout << *i << " " ; 
		}
		cout << "\n" ; 
	}

	// parse the command assuming correctly formatted MIPS instruction (or label)
	void parseCommand(std::string line)
	{
		// strip until before the comment begins
		line = line.substr(0, line.find('#'));
		std::vector<std::string> command;
		boost::tokenizer<boost::char_separator<char>> tokens(line, boost::char_separator<char>(", \t"));
		for (auto &s : tokens)
			command.push_back(s);
		// empty line or a comment only line
		if (command.empty())
			return;
		else if (command.size() == 1)
		{
			std::string label = command[0].back() == ':' ? command[0].substr(0, command[0].size() - 1) : "?";
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command.clear();
		}
		else if (command[0].back() == ':')
		{
			std::string label = command[0].substr(0, command[0].size() - 1);
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command = std::vector<std::string>(command.begin() + 1, command.end());
		}
		else if (command[0].find(':') != std::string::npos)
		{
			int idx = command[0].find(':');
			std::string label = command[0].substr(0, idx);
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command[0] = command[0].substr(idx + 1);
		}
		else if (command[1][0] == ':')
		{
			if (address.find(command[0]) == address.end())
				address[command[0]] = commands.size();
			else
				address[command[0]] = -1;
			command[1] = command[1].substr(1);
			if (command[1] == "")
				command.erase(command.begin(), command.begin() + 2);
			else
				command.erase(command.begin(), command.begin() + 1);
		}
		if (command.empty())
			return;
		if (command.size() > 4)
			for (int i = 4; i < (int)command.size(); ++i)
				command[3] += " " + command[i];
		command.resize(4);
		commands.push_back(command);
	}

	// construct the commands vector from the input file
	void constructCommands(std::ifstream &file)
	{
		std::string line;
		while (getline(file, line))
			parseCommand(line);
		file.close();
		// for (auto i = commands.begin() ; i != commands.end() ; i ++){
		// 	print_vector(*i)  ; 
		// }
	}

	// execute the commands sequentially (no pipelining)
	// void executeCommandsUnpipelined()
	// {
	// 	if (commands.size() >= MAX / 4)
	// 	{
	// 		handleExit(MEMORY_ERROR, 0);
	// 		return;
	// 	}

	// 	int clockCycles = 0;
	// 	while (PCcurr < commands.size())
	// 	{
	// 		++clockCycles;
	// 		std::vector<std::string> &command = commands[PCcurr]; // Fetch the instruction
	// 		if (instructions.find(command[0]) == instructions.end())
	// 		{
	// 			handleExit(SYNTAX_ERROR, clockCycles);
	// 			return;
	// 		}
	// 		exit_code ret = (exit_code)instructions[command[0]](*this, command[1], command[2], command[3]);
	// 		if (ret != SUCCESS)
	// 		{
	// 			handleExit(ret, clockCycles);
	// 			return;
	// 		}
	// 		++commandCount[PCcurr];
	// 		PCcurr = PCnext;
	// 		printRegisters(clockCycles);
	// 	}
	// 	handleExit(SUCCESS, clockCycles);
	// }

	void executePipelined(){
		int clockCycles=0;
		WBctr wbctr1,wbctr2, wbctr3, wbctr4 = {.RegWrite = false , .MemtoReg = false} ; 
		EXctr exctr1, exctr2, exctr3, exctr4 = {.branch = false, .branch_not = false,  .RegDst = false , .OP = 0, .ALUSrc = false  } ; 
		MEMctr memctr1, memctr2, memctr3, memctr4 = {  .MemRead = false, .MemWrite = false } ; 
		vector<string> str = {"0"}  ; 
		pr IF_ID = {.currcomand = str, .at_PC = -1, .PC_new = -1, .WB = &wbctr1, .MEM = &memctr1, .EX = &exctr1,
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0} ; 

		pr ID_ALU = {.currcomand = str, .at_PC = -1, .PC_new = -1, .WB = &wbctr2, .MEM = &memctr2, .EX = &exctr2,
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0} ; 

		pr ALU_MEM = {.currcomand = str, .at_PC = -1, .PC_new = -1, .WB = &wbctr3, .MEM = &memctr3, .EX = &exctr3,
					 .ALURESULT = 0, .Read_data =  0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0} ; 

		pr MEM_WB = {.currcomand = str, .at_PC = -1, .PC_new = -1, .WB = &wbctr4, .MEM = &memctr4, .EX = &exctr4,
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0} ; 

		
		bool is_RR_stall = false ; bool is_beq_stall = false ; bool is_j_stall = false ; bool check = false; 
		int no_of_RR_stall = 0 ; int prev_RD = 0 ; bool prev_RegWrite = false ; int prev_PC = -1 ; int check_rd = -1 ;   
		bool check_rs = false ; bool check_rt = false ; 
		string rs, rt, rd ; 
		printRegistersAndMemoryDelta(0) ; 
		while ( !(PCcurr == -1 && IF_ID.at_PC == -1 && ID_ALU.at_PC == -1 && 
						 		ALU_MEM.at_PC == -1 && MEM_WB.at_PC == -1 )) // if pc < 0 then a stall will occur
		{	
			// cout << "current pc is " << PCcurr << '\n' ; 
			// cout << "ended signal is " << ended << "\n" ; 
			
			++clockCycles;
			// cout << "clock cycle - " << clockCycles << "\n" ; 
			if (PCcurr == -1){PCnext = -1 ; }
			else PCnext = PCcurr + 1 ;  

			  

			// cout << "WB" << '\n' ; 
			// write back (during first half )	// rest stages are in second half cycle 
			if(MEM_WB.at_PC >= 0 && (MEM_WB.WB)->RegWrite){
				 
				if ((MEM_WB.WB)->MemtoReg)  registers[MEM_WB.RD] = MEM_WB.Read_data;
				else registers[MEM_WB.RD] = MEM_WB.ALURESULT;

				// cout << "---------- " << '\n'  ; 
				// printRegisters() ;  
				// cout << "-------------" << "\n" ; 
				// if (MEM_WB.at_PC == (commands.size() - 1) ) break ; 
			};

			
			// mem stage 
			// cout << "MEM" << "\n" ;
			// cout << locateAddress("exit") << "\n" ; 
			if (ALU_MEM.at_PC >= 0 ){
				  
				if ((ALU_MEM.MEM)->MemWrite) {
					// sw instruction 
					if (data[ALU_MEM.ALURESULT / 4] != ALU_MEM.RegReadData2){
					memoryDelta[ALU_MEM.ALURESULT / 4] = ALU_MEM.RegReadData2 ;
					}
					
					data[ALU_MEM.ALURESULT / 4] = ALU_MEM.RegReadData2;
					// cout << "wrote " << ALU_MEM.RegReadData2 << " at address " << ALU_MEM.ALURESULT / 4<< "\n" ;
				} 
				if ( (ALU_MEM.MEM)->MemRead) {
					MEM_WB.Read_data = data[ALU_MEM.ALURESULT/4];
					// cout << "read " << MEM_WB.Read_data << " from address " << ALU_MEM.ALURESULT/4 << "\n" ;

				}
				


				(MEM_WB.WB)->MemtoReg = (ALU_MEM.WB)->MemtoReg;
				(MEM_WB.WB)->RegWrite = (ALU_MEM.WB)->RegWrite; 
				

				MEM_WB.ALURESULT = ALU_MEM.ALURESULT;
				MEM_WB.RD = ALU_MEM.RD;	
			}

			MEM_WB.at_PC = ALU_MEM.at_PC;

			prev_RD = ALU_MEM.RD ;// saving the value of alu_mem rd for stall check
			prev_RegWrite = (ALU_MEM.WB)->RegWrite ; // saving the value of alu_mem regwrite for stall check
			prev_PC = ALU_MEM.at_PC ; // saving the value of alu_mem pc for stall check


			// EX Stage
			// cout << "EX" << "\n" ;
			if (ID_ALU.at_PC >= 0){
				// cout << "entered EX" << "\n" ; 

				// cout << "(ALU_MEM.RD) " << (ALU_MEM.RD)<< "\n" ;  
                // cout << "printing EX controls" << "\n" ; 
                // (ID_ALU.EX)->print(ID_ALU.EX) ; 
				if ((ID_ALU.EX)->OP == 0){
					
					if ((ID_ALU.EX)->ALUSrc) {
						ALU_MEM.ALURESULT = ID_ALU.immediate + ID_ALU.RegReadData1;
						// cout << "added " << ID_ALU.immediate << " and " << ID_ALU.RegReadData1 << "to get " << ALU_MEM.ALURESULT<< "\n" ; 
					}
					else {
						ALU_MEM.ALURESULT = ID_ALU.RegReadData1 + ID_ALU.RegReadData2;
						// cout << "added " << ID_ALU.RegReadData2 << " and " << ID_ALU.RegReadData1 << "to get " << ALU_MEM.ALURESULT<< "\n" ; 

					}
				}
				else if ((ID_ALU.EX)->OP == 1){	
					if ((ID_ALU.EX)->ALUSrc) {
						ALU_MEM.ALURESULT = ID_ALU.RegReadData1 - ID_ALU.immediate ;
						// cout << "subtracted " << ID_ALU.immediate << " from " << ID_ALU.RegReadData1 << "to get " << ALU_MEM.ALURESULT<< "\n" ;
					}
					else {
						ALU_MEM.ALURESULT = ID_ALU.RegReadData1 - ID_ALU.RegReadData2;
						// cout << "subtracted " << ID_ALU.RegReadData2 << " from " << ID_ALU.RegReadData1 << "to get " << ALU_MEM.ALURESULT<< "\n" ;
					}
				}

				else if ((ID_ALU.EX)->OP == 2) {
					if ((ID_ALU.EX)->ALUSrc) {
						ALU_MEM.ALURESULT = ID_ALU.immediate * ID_ALU.RegReadData1;
						// cout << "multiplied " << ID_ALU.immediate << " and " << ID_ALU.RegReadData1 << "to get " << ALU_MEM.ALURESULT<< "\n" ; 
					}
					else {
						ALU_MEM.ALURESULT = ID_ALU.RegReadData1 * ID_ALU.RegReadData2;		
						// cout << "multiplied " << ID_ALU.RegReadData2 << " and " << ID_ALU.RegReadData1 << "to get " << ALU_MEM.ALURESULT<< "\n" ;
					}		
				}
				
				else if ( (ID_ALU.EX)->OP == 3){
					if ((ID_ALU.EX)->ALUSrc) {
						ALU_MEM.ALURESULT = ( ID_ALU.RegReadData1 < ID_ALU.immediate) ? 1 : 0 ;  
						// cout << "setting ALU_MEM.ALURESULT " << ALU_MEM.ALURESULT << "\n" ; 
					}
					else {
						ALU_MEM.ALURESULT = ( ID_ALU.RegReadData1 < ID_ALU.RegReadData2) ? 1 : 0 ;  
						// cout << "setting ALU_MEM.ALURESULT " << ALU_MEM.ALURESULT << "\n" ; 
					}		

				}
				
				
				
				// handling beq instr
				if ((ID_ALU.EX)->branch){
					is_beq_stall = true ; 
					// cout << "two stalls inserted for beq instruction" << "\n" ;
					ALU_MEM.zero = (ALU_MEM.ALURESULT == 0);
					check = (ID_ALU.EX)->branch && ALU_MEM.zero ; 
					if (check){
						
						PCnext = ID_ALU.PC_new ; 
					
					}
					else{
						PCnext = ID_ALU.at_PC + 1; 
					}
					// cout << "branching to new PC " << PCnext << "\n" ;
				}
				// bne 
				if ((ID_ALU.EX)->branch_not){
					is_beq_stall = true ; 
					// cout << "two stalls inserted for bne instruction" << "\n" ;
					ALU_MEM.zero = (ALU_MEM.ALURESULT == 0);
					check = (ID_ALU.EX)->branch && (!ALU_MEM.zero) ; 
					if (check){
						
						PCnext = ID_ALU.PC_new ; 
					
					}
					else{
						PCnext = ID_ALU.at_PC + 1; 
					}
					// cout << "branching to new PC " << PCnext << "\n" ;
				}
				 
				(ALU_MEM.EX)->branch = (ID_ALU.EX)->branch ;
				(ALU_MEM.EX)->branch_not = (ID_ALU.EX)->branch_not ; 
				
				(ALU_MEM.MEM)->MemRead = (ID_ALU.MEM)->MemRead ;
				(ALU_MEM.MEM)->MemWrite = (ID_ALU.MEM)->MemWrite ;

				(ALU_MEM.WB)->MemtoReg = (ID_ALU.WB)->MemtoReg ;
				(ALU_MEM.WB)->RegWrite = (ID_ALU.WB)->RegWrite ; 

				ALU_MEM.RT = ID_ALU.RT ; 
				if((ID_ALU.EX)->RegDst) ALU_MEM.RD = ID_ALU.RD;
				else ALU_MEM.RD = ID_ALU.RT;
				check_rd = ALU_MEM.RD ; 
				// cout << "write back register is " <<   ALU_MEM.RD << '\n' ; 
				ALU_MEM.RegReadData2 = ID_ALU.RegReadData2;

			} 
			
			ALU_MEM.at_PC = ID_ALU.at_PC;
			ALU_MEM.PC_new = ID_ALU.PC_new;



			// Decode phase  
			// cout << "ID" <<"\n" ; 
			
			if(IF_ID.at_PC >= 0) {
				string instruction_name = (IF_ID.currcomand)[0];
				// cout << "decoding instruction- "   ; 
				// print_vector(IF_ID.currcomand) ; 
				

			
				
				if (instruction_name == "add" || instruction_name == "sub" || instruction_name == "mul" || instruction_name == "slt"){
					  rs = (IF_ID.currcomand)[2]; check_rs = true ; 
					  rt = (IF_ID.currcomand)[3] ; check_rt = true; 
				}
				else if (instruction_name == "addi"){
					
					  rs = (IF_ID.currcomand)[2]; check_rs= true ; check_rt = false ; 
				}

				else if (instruction_name == "lw" && (IF_ID.currcomand)[2].back() == ')'){
					int lparen = (IF_ID.currcomand)[2].find('(');
					  rs = (IF_ID.currcomand)[2].substr(lparen + 1); rs.pop_back();
					check_rs = true  ; check_rt = false ; 
				}

				else if (instruction_name == "sw" ){
					if ((IF_ID.currcomand)[2].back() == ')'){
					int lparen = (IF_ID.currcomand)[2].find('(');
					  rs = (IF_ID.currcomand)[2].substr(lparen + 1); rs.pop_back();
					//   cout << "decoded rs is " << rs << "\n" ; 
					check_rs = true ; 
					}
					else  check_rs = false ; 
					  rt = (IF_ID.currcomand)[1]; check_rt = true ;

				}
				else if (instruction_name == "beq" || instruction_name == "bne"  ){
					  rs = (IF_ID.currcomand)[1] ; check_rs = true ; 
					  rt = (IF_ID.currcomand)[2] ; check_rt = true ; 
				}

				// CHECKING STALL
				// cout << "prev_PC " << prev_PC << "\n" ; 
				// cout << "prev_RD " << prev_RD << "\n" ; 
				// if (prev_RegWrite) {cout << "prev_RegWrite true" << "\n" ; }
				// else {cout << "prev_RegWrite false" << "\n" ; }
				// cout << "RD is " << ID_ALU.RD << "\n" ; 
				// cout << "rs is " << registerMap[rs] << "\n" ; 
				// cout << " ID_ALU.at_PC "<<  ID_ALU.at_PC  << "\n" ; 
				// cout << "(ID_ALU.WB)->RegWrite " << (ID_ALU.WB)->RegWrite << "\n" ; 
				if ( ID_ALU.at_PC >= 0 && (ID_ALU.WB)->RegWrite && (check_rd != 0) && ( (check_rs && registerMap[rs] == check_rd) || (check_rt && registerMap[rt] == check_rd) ) ){
						is_RR_stall = true  ; no_of_RR_stall = 2 ;
						// cout << "rr stalling" << "\n" ;  
						// cout << "type 1 stall" << "\n" ; 
					}
				else if (prev_PC >= 0 && prev_RegWrite && (prev_RD != 0) && ( (check_rs && registerMap[rs] == prev_RD) || (check_rt && registerMap[rt] == prev_RD) )){
						is_RR_stall = true ; no_of_RR_stall = 1 ; 
						// cout << "rr stalling" << "\n" ;  
						// cout << "type 2 stall" << "\n" ; 
					}
			
				else {

					// setting the control signals
				
					(ID_ALU.EX)->RegDst = true ; (ID_ALU.EX)->OP = 0 ; (ID_ALU.EX)->ALUSrc = false ;  

					if (instruction_name == "add"){(ID_ALU.EX)->OP = 0 ; }
					if (instruction_name == "sub"){(ID_ALU.EX)->OP= 1 ; }
					if (instruction_name == "mul") { (ID_ALU.EX)->OP = 2 ; } 
					if (instruction_name == "slt") { (ID_ALU.EX)->OP = 3 ;}

					(ID_ALU.EX)->branch = (ID_ALU.EX)->branch_not =  false; (ID_ALU.MEM)->MemRead = false; (ID_ALU.MEM)->MemWrite = false;
					
					(ID_ALU.WB)->RegWrite = true ; (ID_ALU.WB)->MemtoReg = false ; 
					
					if (instruction_name == "lw"){ // change1 - instead of adding immediate field in ex stage, 
												//  I add it in id stage and equate it to immediate, setting regread1 = 0 
						(ID_ALU.EX)->RegDst =0;
						(ID_ALU.EX)->ALUSrc =1;
						(ID_ALU.MEM)->MemRead =1;
						(ID_ALU.WB)->MemtoReg =1;					
					}
					else if (instruction_name == "sw"){
						(ID_ALU.EX)->ALUSrc =1;
						(ID_ALU.MEM)->MemWrite=1;
						(ID_ALU.WB)->RegWrite=0;					
					}
					else if (instruction_name == "beq"){ // inserting a stall whenver encountering a beq instruction as per piazza instruction 
						(ID_ALU.EX)->branch=1;
						(ID_ALU.EX)->branch_not=0;
						(ID_ALU.EX)->OP =1;  
						(ID_ALU.WB)->RegWrite=0;					
					}
					else if (instruction_name == "bne"){
						(ID_ALU.EX)->branch=0;
						(ID_ALU.EX)->branch_not=1;
						(ID_ALU.EX)->OP =1;  
						(ID_ALU.WB)->RegWrite=0;		

					}
					else if (instruction_name =="j"){
					
						(ID_ALU.WB)->RegWrite =0;
					}
					else if (instruction_name == "addi"){
						(ID_ALU.EX)->ALUSrc=1;	 
					}
				




					if (instruction_name == "add" || instruction_name == "sub" || instruction_name == "mul" || instruction_name == "slt"){
					
					  rd = (IF_ID.currcomand)[1];
					  rs = (IF_ID.currcomand)[2];
					  rt = (IF_ID.currcomand)[3] ;
					
					ID_ALU.RegReadData1 = registers[registerMap[rs]] ; 
					ID_ALU.RegReadData2 = registers[registerMap[rt]] ; 
					
					ID_ALU.RD = registerMap[rd] ; 
					ID_ALU.RT = registerMap[rt] ; 
					ID_ALU.PC_new = IF_ID.at_PC + 1;
					
					}


					else if (instruction_name == "lw" || instruction_name == "sw"){ 
						int lparen ; int offset ; 
						 if ((IF_ID.currcomand)[2].back() == ')'){
                            lparen = (IF_ID.currcomand)[2].find('('), offset = stoi(lparen == 0 ? "0" : (IF_ID.currcomand)[2].substr(0, lparen));
                            rs = (IF_ID.currcomand)[2].substr(lparen + 1);
                            rs.pop_back();

                            ID_ALU.RegReadData1 = registers[registerMap[rs]] ; 
                            ID_ALU.immediate = offset ; 

                        }
                        else {
                            ID_ALU.immediate = stoi((IF_ID.currcomand)[2]) ; 
                            ID_ALU.RegReadData1 = 0 ; 
                        }
						

						rt = (IF_ID.currcomand)[1];
						ID_ALU.RegReadData2 = registers[registerMap[rt]] ; 
                        ID_ALU.RS = registerMap[rs] ; 
						ID_ALU.RT = registerMap[rt] ;  
						ID_ALU.PC_new = IF_ID.at_PC + 1;
					}

					else if (instruction_name == "addi") { 
						  rs = (IF_ID.currcomand)[2]; 
						ID_ALU.immediate = stoi(IF_ID.currcomand[3]) ;
						  rd = (IF_ID.currcomand)[1];

						ID_ALU.RegReadData1 = registers[registerMap[rs]] ; 
						ID_ALU.RD = registerMap[rd] ; 
						ID_ALU.PC_new = IF_ID.at_PC + 1 ; 
					}

					else if (instruction_name == "j"){
						ID_ALU.PC_new = address[(IF_ID.currcomand)[1]] ;
						is_j_stall = true;
						PCnext = ID_ALU.PC_new ;
						// cout << "jumping to new PC " << ID_ALU.PC_new << "\n" ; 
					}

					else if (instruction_name == "beq" || instruction_name == "bne" ){
						rs = (IF_ID.currcomand)[1]; 
						rt = (IF_ID.currcomand)[2]; 

						ID_ALU.RegReadData1 = registers[registerMap[rs]] ; 
						ID_ALU.RegReadData2 = registers[registerMap[rt]] ; 
						ID_ALU.PC_new = address[IF_ID.currcomand[3]] ; 


					}
			
			// cout << "decoded signals" << "\n" ; 
			// ID_ALU.print(&ID_ALU) ;
			
			}
			}
			ID_ALU.at_PC = IF_ID.at_PC ;
			

			// IF stage 
			// cout << "IF" << "\n" ;
			
			if ( (PCcurr >= 0 && PCcurr < commands.size()) || is_RR_stall){
				if (!(is_RR_stall)){	 
			IF_ID.at_PC = PCcurr ;
			IF_ID.currcomand = commands[PCcurr] ; // fetching the instruction
			// cout << "fetched instruction is " << "\n" ; 
			// print_vector(IF_ID.currcomand) ;
			}
				else {
					// cout << "stalled cycle" << "\n" ; 
				} 
			} 
			else {
				// cout << "no fetched instruction" << "\n" ; 
				IF_ID.at_PC = -1 ;
			}
			

			// doing rr stalls 
			if (is_RR_stall && !is_beq_stall){
				ID_ALU.at_PC = -1 ;
				PCnext = PCcurr ;  
				// no_of_RR_stall -= 1 ; 
			}
			
			// doing j stalls
			if (is_j_stall && !is_beq_stall){
				IF_ID.at_PC = -1 ; 
			}

			// doing beq stalls
			if (is_beq_stall){
				IF_ID.at_PC = -1 ; 
				ID_ALU.at_PC = -1 ; 
				// ALU_MEM.at_PC = -1 ; 
			}
			// updating program counter
			if (PCnext >= commands.size()) { PCnext = -1 ; }  
			PCcurr = PCnext ;  
			is_RR_stall = false ; is_beq_stall = false ; is_j_stall = false ; check = false ; check_rs = check_rt = false ; 

			printRegistersAndMemoryDelta(clockCycles);
		}
		// printRegisters() ; 
		// cout << "printing memory " << "\n" ; 
		// cout << data[83] << "\n" ; 
		

		
	}

	void printRegistersAndMemoryDelta(int clockCycle)
	{
		for (int i = 0; i < 32; ++i)
			std::cout << registers[i] << ' ';
		std::cout << '\n';
		std::cout << memoryDelta.size() << ' ';
		if ( memoryDelta.size() == 0){
			cout << "\n" ; 
		}
		for (auto &p : memoryDelta)
			std::cout << p.first << ' ' << p.second << '\n';
		memoryDelta.clear();
	}

	// print the register data in decimal
	void printRegisters()
	{
		// std::cout << "Cycle number: " << clockCycle << '\n'
		// 		  << std::dec; 
		for (int i = 0; i < 32; ++i)
			std::cout << registers[i] << ' ';
		std::cout << std::dec << '\n';
	}
};

#endif