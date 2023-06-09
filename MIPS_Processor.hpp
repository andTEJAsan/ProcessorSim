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
	};
	
	struct MEMctr{
		bool branch;
		bool MemRead;
		bool MemWrite;
		bool jump;
	};
	struct EXctr{
		bool RegDst;
		int OP; // 0 denotes add , 1 denotes subtraction
		bool ALUSrc ; // true if alu src has to be immediate field
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
	bool zero = ALURESULT == 0;
	int RegReadData1;
	int RegReadData2;

	int RS,RT,RD;
};
typedef struct PipelineReg pr;
enum PipelineStage
{
	IF = 0,
	ID,
	ALU,
	MEM,
	WB
};int registers[32] = {0}, PCcurr = 0, PCnext;
	std::unordered_map<std::string, std::function<int(MIPS_Architecture &, std::string, std::string, std::string)>> instructions;
	std::unordered_map<std::string, int> registerMap, address;

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
	}

	// execute the commands sequentially (no pipelining)
	void executeCommandsUnpipelined()
	{
		if (commands.size() >= MAX / 4)
		{
			handleExit(MEMORY_ERROR, 0);
			return;
		}

		int clockCycles = 0;
		while (PCcurr < commands.size())
		{
			++clockCycles;
			std::vector<std::string> &command = commands[PCcurr]; // Fetch the instruction
			if (instructions.find(command[0]) == instructions.end())
			{
				handleExit(SYNTAX_ERROR, clockCycles);
				return;
			}
			exit_code ret = (exit_code)instructions[command[0]](*this, command[1], command[2], command[3]);
			if (ret != SUCCESS)
			{
				handleExit(ret, clockCycles);
				return;
			}
			++commandCount[PCcurr];
			PCcurr = PCnext;
			printRegisters(clockCycles);
		}
		handleExit(SUCCESS, clockCycles);
	}
	void executePipelined(){
		int clockCycles=0;
		pr IF_ID;
		pr ID_ALU;
		pr ALU_MEM;
		pr MEM_WB;
		while (PCcurr < commands.size())
		{
			++clockCycles;
			// write back (during first half )	
			if(MEM_WB.at_PC >= 0 && (MEM_WB.WB)->RegWrite){
				
				if ((MEM_WB.WB)->MemtoReg)  registers[MEM_WB.RD] = MEM_WB.Read_data;
				else registers[MEM_WB.RD] = MEM_WB.ALURESULT;

			};
		
			// mem stage 
			if (ALU_MEM.at_PC >= 0 ){
				if ((ALU_MEM.MEM)->MemWrite) data[ALU_MEM.ALURESULT] = ALU_MEM.RegReadData2; 
				if ( (ALU_MEM.MEM)->MemRead) MEM_WB.Read_data = data[ALU_MEM.ALURESULT];
				(ALU_MEM.MEM)->branch =((ALU_MEM).zero && (ALU_MEM.MEM)->branch);
				if ((ALU_MEM.MEM)->branch || (ALU_MEM.MEM)->jump ) PCnext = ALU_MEM.PC_new;	
				MEM_WB.WB = ALU_MEM.WB;
				MEM_WB.ALURESULT = ALU_MEM.ALURESULT;
				MEM_WB.RD = ALU_MEM.RD;	
			}

			MEM_WB.at_PC = ALU_MEM.at_PC;



			// EX Stage

			if (ID_ALU.at_PC >= 0){
						
				if ((ID_ALU.EX)->OP == 0){
					if (ID_ALU.EX->ALUSrc) ALU_MEM.ALURESULT = ID_ALU.immediate + ID_ALU.RegReadData1;
					else ALU_MEM.ALURESULT = ID_ALU.RegReadData1 + ID_ALU.RegReadData2;
				}
				else if ((ID_ALU.EX)->OP == 1){	
					if (ID_ALU.EX->ALUSrc) ALU_MEM.ALURESULT = ID_ALU.immediate - ID_ALU.RegReadData1;
					else ALU_MEM.ALURESULT = ID_ALU.RegReadData1 - ID_ALU.RegReadData2;
				}

				else {
					if (ID_ALU.EX->ALUSrc) ALU_MEM.ALURESULT = ID_ALU.immediate * ID_ALU.RegReadData1;
					else ALU_MEM.ALURESULT = ID_ALU.RegReadData1 * ID_ALU.RegReadData2;				
				}
				 
				ALU_MEM.zero = (ALU_MEM.ALURESULT == 0);
				(ALU_MEM.MEM)=(ID_ALU.MEM);
				(ALU_MEM.WB) = ID_ALU.WB;
				
				if((ID_ALU.EX)->RegDst) ALU_MEM.RD = ID_ALU.RD;
				else ALU_MEM.RD = ID_ALU.RT;  
				ALU_MEM.RegReadData2 = ID_ALU.RegReadData2;

			} 
			ALU_MEM.at_PC = ID_ALU.at_PC;
			ALU_MEM.PC_new = ID_ALU.PC_new;
			// Decode phase  
			if(IF_ID.at_PC >= 0) {
				
				string instruction_name = (IF_ID.currcomand)[0];
				string rs = IF_ID.currcomand[1];
				string rt = IF_ID.currcomand[2];
				srting	
				EXctr excontrols = {.RegDst=1, .OP=0 , .ALUSrc = 0};
				IF_ID.EX = &excontrols;
				MEMctr memcontrols = {.branch = 0, .MemRead=0 , .MemWrite =0 ,.jump =0};
				IF_ID.MEM = &memcontrols;
				WBctr wbcontrols = {.RegWrite = 1, .MemtoReg =0};
				IF_ID.WB = &wbcontrols;
				
				if (instruction_name == "lw"){
					(IF_ID.EX)->RegDst =0;
					(IF_ID.EX)->ALUSrc =1;
					(IF_ID.MEM)->MemRead =1;
					(IF_ID.WB)->MemtoReg =1;					
				}
				else if (instruction_name == "sw"){
					(IF_ID.EX)->ALUSrc =1;
					(IF_ID.MEM)->MemWrite=1;
					(IF_ID.WB)->RegWrite=0;					
				}
				else if (instruction_name == "beq"){
					(IF_ID.MEM)->branch=1;
					(IF_ID.EX)->OP =1;
					(IF_ID.WB)->RegWrite=0;					
				}
				else if (instruction_name =="j"){
					(IF_ID.MEM)->jump =1;
					(IF_ID.WB)->RegWrite =0;
				}
				else if (instruction_name == "addi"){
					(IF_ID.EX)->ALUSrc=1;	
				}
			ID_ALU.RegReadData1 = register[registerMap[]]
			}


		}


		
	}
	// print the register data in hexadecimal
	void printRegisters(int clockCycle)
	{
		std::cout << "Cycle number: " << clockCycle << '\n'
				  << std::hex;
		for (int i = 0; i < 32; ++i)
			std::cout << registers[i] << ' ';
		std::cout << std::dec << '\n';
	}
};

#endif