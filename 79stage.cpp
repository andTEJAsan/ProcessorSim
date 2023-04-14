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
using namespace std ; 
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
	
	struct MEMctr2{
		bool MemWrite;
		void print(struct MEMctr2* ptr){
			if (ptr == NULL){return ; }
			cout << "MemWrite" << ptr->MemWrite  << "\n" ; 
		}

	};

    struct MEMctr1{
		bool MemRead;
		void print(struct MEMctr1* ptr){
			if (ptr == NULL){return ; }
			cout << "MemRead" << ptr->MemRead  << "\n" ; 
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
			cout << "branch_not" << ptr->branch_not  << "\n" ; 
			cout << "RegDst" << ptr->RegDst  << "\n" ; 
			cout << "OP" << ptr->OP  << "\n" ; 
			cout << "ALUSrc" << ptr->ALUSrc  << "\n" ; 
		}

	};
	typedef struct WBctr WBctr;
	typedef struct MEMctr1 MEMctr1;
    typedef struct MEMctr2 MEMctr2 ; 
	typedef struct EXctr EXctr;
	struct PipelineReg
    {
        std::vector<string> currcomand;
        int at_PC = -1;
        int PC_new = -1;
    /*control signals */
        WBctr* WB = NULL;
        MEMctr1* MEM1 = NULL;
        MEMctr2* MEM2 = NULL ; 
        EXctr* EX = NULL;
    /*saved results */
        int ALURESULT;
        int Read_data;
        int immediate;
        bool zero = (ALURESULT == 0) ;
        int RegReadData1;
        int RegReadData2;

        int RS,RT,RD;
         bool is_9_stage ; 

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
            cout << "is_9_stage " << ptr->is_9_stage << "\n" ;
            cout << "printing control signals" << '\n' ;
            
            if (ptr->EX)  (ptr->EX)->print(ptr->EX) ;
            if (ptr->MEM1) (ptr->MEM1)->print(ptr->MEM1) ; 
            if (ptr->MEM2) (ptr->MEM2)->print(ptr->MEM2) ; 
            if (ptr->WB) (ptr->WB)->print(ptr->WB) ; 
            
        }
    };
	typedef struct PipelineReg pr;

    struct  seven_nine_stage_pr : public pr {
        bool is_9_stage ; 
        void print(struct   seven_nine_stage_pr* ptr)  {
            if (ptr == NULL){return ; }
            cout << "at_PC " << ptr->at_PC << "\n" ; 
            cout << "PC_new " << ptr->PC_new << "\n" ;
            cout << "ALURESULT " << ptr->ALURESULT << "\n" ;
            cout << "Read_data " << ptr->Read_data << "\n" ;
            cout << "immediate " << ptr->immediate << "\n" ;
            cout << "zero " << ptr->zero << "\n" ;
            cout << "RegReadData1 " << ptr->RegReadData1 << "\n" ;
            cout << "RegReadData2 " << ptr->RegReadData2 << "\n" ;
            cout << "RS " << ptr->RS << "\n" ;
            cout << "RT " << ptr->RT << "\n" ;
            cout << "RD " << ptr->RD << "\n" ;
            cout << "is_9_stage " << ptr->is_9_stage << "\n" ; 
            cout << "printing control signals " << '\n' ;
            
            if (ptr->EX)  (ptr->EX)->print(ptr->EX) ;
            if (ptr->MEM1) (ptr->MEM1)->print(ptr->MEM1) ; 
            if (ptr->MEM2) (ptr->MEM2)->print(ptr->MEM2) ; 
            if (ptr->WB) (ptr->WB)->print(ptr->WB) ; 
            
        }
    } ; 

	struct check {
		int rd ; 
		int at_pc ; 
		bool regwrite ; 
	} ;
	typedef struct check check ; 

    typedef struct  seven_nine_stage_pr   seven_nine_stage_pr ; 

	int registers[32] = {0}, PCcurr = 0, PCnext;
	std::unordered_map<std::string, std::function<int(MIPS_Architecture &, std::string, std::string, std::string)>> instructions;
	std::unordered_map<std::string, int> registerMap, address;
	std :: unordered_map<int, int> memoryDelta ; 
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
		int clockCycles=0; WBctr wbarr[8] ; EXctr exarr[8] ; MEMctr1 mem1arr[8] ; MEMctr2 mem2arr[8] ; 
		
        WBctr wbctr1 = {.RegWrite = false , .MemtoReg = false} ; 
        EXctr exctr1 = {.branch = false, .branch_not = false,  .RegDst = false , .OP = 0, .ALUSrc = false  } ; 
		MEMctr2 memctr2 = {   .MemWrite = false } ; 
        MEMctr1 memctr1 = {  .MemRead = false } ; 
        for (int i = 0 ; i < 8 ; i ++){
            wbarr[i] = wbctr1 ; exarr[i] = exctr1 ; mem1arr[i] = memctr1 ; mem2arr[i] = memctr2 ; 
        }
		vector<string> str = {"0"}  ; 
		pr IF1_IF2 = {.currcomand = str, .at_PC = -1, .PC_new = -1, .WB = &(wbarr[0]), .MEM1 = &(mem1arr[0]), .MEM2 = &(mem2arr[0]) , .EX = &(exarr[0]) , 
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0, .is_9_stage = false } ; 

		pr IF2_ID1 = {.currcomand = str, .at_PC = -1, .PC_new = -1,  .WB = &(wbarr[1]), .MEM1 = &(mem1arr[1]), .MEM2 = &(mem2arr[1]) , .EX = &(exarr[1]) , 
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0, .is_9_stage = false} ; 

		pr ID1_ID2 = {.currcomand = str, .at_PC = -1, .PC_new = -1,  .WB = &(wbarr[2]), .MEM1 = &(mem1arr[2]), .MEM2 = &(mem2arr[2]) , .EX = &(exarr[2]) , 
					 .ALURESULT = 0, .Read_data =  0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0, .is_9_stage = false} ; 

		pr ID2_RR = {.currcomand = str, .at_PC = -1, .PC_new = -1,  .WB = &(wbarr[3]), .MEM1 = &(mem1arr[3]), .MEM2 = &(mem2arr[3]) , .EX = &(exarr[3]) , 
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0, .is_9_stage = false} ; 
        pr RR_EX = {.currcomand = str, .at_PC = -1, .PC_new = -1,  .WB = &(wbarr[4]), .MEM1 = &(mem1arr[4]), .MEM2 = &(mem2arr[4]) , .EX = &(exarr[4]) , 
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0, .is_9_stage = false} ; 

		pr EX_DM1 = {.currcomand = str, .at_PC = -1, .PC_new = -1,  .WB = &(wbarr[5]), .MEM1 = &(mem1arr[5]), .MEM2 = &(mem2arr[5]) , .EX = &(exarr[5]) , 
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0, .is_9_stage = false} ; 

		pr DM1_DM2 = {.currcomand = str, .at_PC = -1, .PC_new = -1, .WB = &(wbarr[6]), .MEM1 = &(mem1arr[6]), .MEM2 = &(mem2arr[6]) , .EX = &(exarr[6]) , 
					 .ALURESULT = 0, .Read_data =  0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0, .is_9_stage = false} ; 

		pr DM2_WB = {.currcomand = str, .at_PC = -1, .PC_new = -1,  .WB = &(wbarr[7]), .MEM1 = &(mem1arr[7]), .MEM2 = &(mem2arr[7]) , .EX = &(exarr[7]) , 
					 .ALURESULT = 0, .Read_data = 0, .immediate = 0 , .zero = false, .RegReadData1 = 0, .RegReadData2 = 0, 
					.RS = 0, .RT = 0, .RD = 0, .is_9_stage = false} ; 
     pr* dummy ; 
		

		bool is_RR_stall = false ; bool is_beq_stall = false ; bool is_j_stall = false ; bool is_fifo_stall = false ; 
		int no_of_RR_stall = 0 ; int prev_RD = 0 ; bool prev_RegWrite = false ; int prev_PC = -1 ; int check_rd = -1 ;   
		bool check_rs = false ; bool check_rt = false ; 

		bool if1_if2_write_enable = true ; bool if2_id1_write_enable = true ;
		bool id1_id2_write_enable = true ; bool id2_rr_write_enable = true ;
		string rs, rt, rd ; bool prev_is_9_stage = false ; 

		
		printRegistersAndMemoryDelta(0) ; 

		vector<check> vi ; // vector to check which stages are active 
		while ( !(PCcurr == -1 && IF1_IF2.at_PC == -1 && IF2_ID1.at_PC == -1 && ID1_ID2.at_PC == -1 && 
						 		ID2_RR.at_PC == -1 && RR_EX.at_PC == -1 && EX_DM1.at_PC == -1 && 
                                DM1_DM2.at_PC == -1 && DM2_WB.at_PC == -1)) // if pc < 0 then a stall will occur
		{		
			// cout << "current pc is " << PCcurr << '\n' ; 
			// cout << "ended signal is " << ended << "\n" ; 
			
			++clockCycles;
			// cout << "clock cycle - " << clockCycles << "\n" ; 
			if (PCcurr == -1){PCnext = -1 ; }
			else PCnext = PCcurr + 1 ;  

			  

			// cout << "WB" << '\n' ; 
			// write back (during first half )	// rest stages are in second half cycle 
            // cout << DM2_WB.at_PC  << " " << (DM2_WB.WB)->RegWrite << "\n" ; 
			if(DM2_WB.at_PC >= 0 && (DM2_WB.WB)->RegWrite){
				
				
				check temp = {.rd = -1  , .at_pc = -1, .regwrite = false } ; 
				temp.rd = DM2_WB.RD ; temp.regwrite = (DM2_WB.WB)->RegWrite ; temp.at_pc = DM2_WB.at_PC ; 
				vi.push_back(temp) ;  // written as RW and RR take an entire cycle 

				if ((DM2_WB.WB)->MemtoReg)  registers[DM2_WB.RD] = DM2_WB.Read_data;
				else registers[DM2_WB.RD] = DM2_WB.ALURESULT;

				// cout << "---------- " << '\n'  ; 
				// printRegisters() ;  
				// cout << "-------------" << "\n" ; 
				// if (MEM_WB.at_PC == (commands.size() - 1) ) break ; 
			};

			
			// DM2 stage (memwrite)
			// cout << "DM2" << "\n" ;
			
			if (DM1_DM2.at_PC >= 0 && DM1_DM2.is_9_stage){

				check temp = {.rd = -1  , .at_pc = -1, .regwrite = false } ; 
				temp.rd = DM1_DM2.RD ; temp.regwrite = (DM1_DM2.WB)->RegWrite ; temp.at_pc = DM1_DM2.at_PC ; 
				vi.push_back(temp) ;				
				  
				if ((DM1_DM2.MEM2)->MemWrite) {
					if ( data[(DM1_DM2.ALURESULT)/4] != DM1_DM2.RegReadData2){
						memoryDelta[(DM1_DM2.ALURESULT)/4] = DM1_DM2.RegReadData2 ;
					   }
					data[DM1_DM2.ALURESULT/4] = DM1_DM2.RegReadData2;
					// cout << "wrote " << DM1_DM2.RegReadData2 << " at address " << DM1_DM2.ALURESULT/4 << "\n" ;
				} 
				
				(DM2_WB.WB)->MemtoReg = (DM1_DM2.WB)->MemtoReg;
				(DM2_WB.WB)->RegWrite = (DM1_DM2.WB)->RegWrite; 
				
                DM2_WB.ALURESULT = DM1_DM2.ALURESULT;
				DM2_WB.RD = DM1_DM2.RD;	
				DM2_WB.Read_data = DM1_DM2.Read_data ; 
			}

			DM2_WB.at_PC = DM1_DM2.at_PC;

			// prev_RD = ALU_MEM.RD ;// saving the value of alu_mem rd for stall check
			// prev_RegWrite = (ALU_MEM.WB)->RegWrite ; // saving the value of alu_mem regwrite for stall check
			// prev_PC = ALU_MEM.at_PC ; // saving the value of alu_mem pc for stall check



            // DM1 stage (mem read stage) 
            // cout << "DM1" << "\n" ;
            if (EX_DM1.at_PC >= 0 && EX_DM1.is_9_stage){
				check temp = {.rd = -1  , .at_pc = -1, .regwrite = false } ; 
				temp.rd = EX_DM1.RD ; temp.regwrite = (EX_DM1.WB)->RegWrite ; temp.at_pc = EX_DM1.at_PC ; 
				vi.push_back(temp) ;


                if ( (EX_DM1.MEM1)->MemRead) {
                        DM1_DM2.Read_data = data[EX_DM1.ALURESULT/4];
                        // cout << "read " <<DM1_DM2.Read_data<< " from address " << EX_DM1.ALURESULT/4 << "\n" ;
                    }

                (DM1_DM2.WB)->MemtoReg = (EX_DM1.WB)->MemtoReg;
				(DM1_DM2.WB)->RegWrite = (EX_DM1.WB)->RegWrite; 

                (DM1_DM2.MEM2)->MemWrite = (EX_DM1.MEM2)->MemWrite ; 

                DM1_DM2.ALURESULT = EX_DM1.ALURESULT;
				DM1_DM2.RD = EX_DM1.RD;	
                DM1_DM2.RegReadData2 = EX_DM1.RegReadData2 ;    
                DM1_DM2.is_9_stage = EX_DM1.is_9_stage ; 
                
            }   

            DM1_DM2.at_PC = EX_DM1.at_PC  ; 

			// EX Stage
			// cout << "EX" << "\n" ;
			if (RR_EX.at_PC >= 0){
				check temp = {.rd = -1  , .at_pc = -1, .regwrite = false } ; 
				temp.regwrite = (RR_EX.WB)->RegWrite ; temp.at_pc = RR_EX.at_PC ; 
				
				

                dummy = RR_EX.is_9_stage ? &EX_DM1 : &DM2_WB ; 
                  
                if (!RR_EX.is_9_stage){
					// cout << "jumping to WB in next cycle" << "\n" ; 
					}
				if ((RR_EX.EX)->OP == 0){
					
					if ((RR_EX.EX)->ALUSrc) {
						dummy->ALURESULT = RR_EX.immediate + RR_EX.RegReadData1;
						// cout << "added " << RR_EX.immediate << " and " << RR_EX.RegReadData1 << "to get " << dummy->ALURESULT<< "\n" ; 
					}
					else {
						dummy->ALURESULT = RR_EX.RegReadData1 + RR_EX.RegReadData2;
						// cout << "added " << RR_EX.RegReadData2 << " and " << RR_EX.RegReadData1 << "to get " << dummy->ALURESULT<< "\n" ; 

					}
				}
				else if ((RR_EX.EX)->OP == 1){	
					if ((RR_EX.EX)->ALUSrc) {
						dummy->ALURESULT = RR_EX.RegReadData1 - RR_EX.immediate ;
						// cout << "subtracted " << RR_EX.immediate << " from " << RR_EX.RegReadData1 << "to get " << dummy->ALURESULT<< "\n" ;
					}
					else {
						dummy->ALURESULT = RR_EX.RegReadData1 - RR_EX.RegReadData2;
						// cout << "subtracted " << RR_EX.RegReadData2 << " from " << RR_EX.RegReadData1 << "to get " << dummy->ALURESULT<< "\n" ;
					}
				}

				else if ((RR_EX.EX)->OP == 2) {
					if ((RR_EX.EX)->ALUSrc) {
						dummy->ALURESULT = RR_EX.immediate * RR_EX.RegReadData1;
						// cout << "multiplied " << RR_EX.immediate << " and " << RR_EX.RegReadData1 << "to get " << dummy->ALURESULT<< "\n" ; 
					}
					else {
						dummy->ALURESULT = RR_EX.RegReadData1 * RR_EX.RegReadData2;		
						// cout << "multiplied " << RR_EX.RegReadData2 << " and " << RR_EX.RegReadData1 << "to get " << dummy->ALURESULT<< "\n" ;
					}		
				}
				else if ((RR_EX.EX)->OP == 3) {
					if ((RR_EX.EX)->ALUSrc) {
						dummy->ALURESULT =  (RR_EX.RegReadData1 < RR_EX.immediate) ? 1 : 0  ;
					}
					else {
						dummy->ALURESULT =  (RR_EX.RegReadData1 < RR_EX.RegReadData2) ? 1 : 0  ;
					}		
				}
				 
				
				
				
				// handling beq instr
				if ((RR_EX.EX)->branch){
					is_beq_stall = true ; 
					// cout << "five stalls inserted for beq instruction" << "\n" ;
					dummy->zero = (dummy->ALURESULT == 0);
					
					if ((RR_EX.EX)->branch && dummy->zero){
						PCnext = RR_EX.PC_new ;
					}
					else{
						PCnext = RR_EX.at_PC + 1; 
					}
					// cout << "branching to new PC " << PCnext << "\n" ;
				}

				// handling bne instr
				if ((RR_EX.EX)->branch_not){
					is_beq_stall = true ; 
					// cout << "five stalls inserted for beq instruction" << "\n" ;
					dummy->zero = (dummy->ALURESULT == 0);
					
					if ((RR_EX.EX)->branch && (!dummy->zero) ){
						PCnext = RR_EX.PC_new ;
					}
					else{
						PCnext = RR_EX.at_PC + 1; 
					}
					// cout << "branching to new PC " << PCnext << "\n" ;
				}
				 
				
				(dummy->MEM1)->MemRead = (RR_EX.MEM1)->MemRead ;
				
                (dummy->MEM2)->MemWrite = (RR_EX.MEM2)->MemWrite ;

				(dummy->WB)->MemtoReg = (RR_EX.WB)->MemtoReg ;
				(dummy->WB)->RegWrite = (RR_EX.WB)->RegWrite ; 

				 
				if((RR_EX.EX)->RegDst) dummy->RD = RR_EX.RD;
				else dummy->RD = RR_EX.RT;
				temp.rd = dummy->RD ; 

				vi.push_back(temp) ; 

				// cout << "write back register is " <<   dummy->RD << '\n' ; 
				dummy->RegReadData2 = RR_EX.RegReadData2;
                dummy->is_9_stage = RR_EX.is_9_stage ; 
                dummy->at_PC = RR_EX.at_PC ; 
				if (!RR_EX.is_9_stage) {
					EX_DM1.at_PC = -1 ; 
				}
                // if (clockCycles == 6){
                //     cout << "DM2_WB.at_PC " << DM2_WB.at_PC << "\n" ; 
                // }
                dummy->PC_new = RR_EX.PC_new ; 
			} 
			
			else {
                EX_DM1.at_PC = RR_EX.at_PC;
			    EX_DM1.PC_new = RR_EX.PC_new;
            }


            // RR stage 
            // cout << "RR stage" << "\n" ; 
            if (ID2_RR.at_PC >= 0 ){

				// checking for data hazards
				string instruction_name = (ID2_RR.currcomand)[0] ; 
				if (instruction_name == "add" || instruction_name == "sub" || instruction_name == "mul" || instruction_name == "slt"){
					  rs = (ID2_RR.currcomand)[2]; check_rs = true ; 
					  rt = (ID2_RR.currcomand)[3] ; check_rt = true; 
				}
				else if (instruction_name == "addi"){
					
					  rs = (ID2_RR.currcomand)[2]; check_rs= true ; check_rt = false ; 
				}

				else if (instruction_name == "lw" && (ID2_RR.currcomand)[2].back() == ')'){
					int lparen = (ID2_RR.currcomand)[2].find('(');
					  rs = (ID2_RR.currcomand)[2].substr(lparen + 1); rs.pop_back();
					check_rs = true  ; check_rt = false ; 
				}

				else if (instruction_name == "sw" ){
					if ((ID2_RR.currcomand)[2].back() == ')'){
					int lparen = (ID2_RR.currcomand)[2].find('(');
					  rs = (ID2_RR.currcomand)[2].substr(lparen + 1); rs.pop_back();
					//   cout << "decoded rs is " << rs << "\n" ; 
					check_rs = true ; 
					}
					else  check_rs = false ; 
					  rt = (ID2_RR.currcomand)[1]; check_rt = true ;

				}
				else if (instruction_name == "beq" || instruction_name == "bne" ){
					  rs = (ID2_RR.currcomand)[1] ; check_rs = true ; 
					  rt = (ID2_RR.currcomand)[2] ; check_rt = true ; 
				}

				while (!vi.empty()){
					check to_check = (vi.back())  ; // struct of stage which is active and most recent
					vi.pop_back() ; 
					if ( to_check.at_pc >= 0 && to_check.regwrite && (to_check.rd != 0) && ( (check_rs && registerMap[rs] == to_check.rd ) || (check_rt && registerMap[rt] == to_check.rd ) ) ){
					is_RR_stall = true  ;
					if1_if2_write_enable = if2_id1_write_enable = id1_id2_write_enable = id2_rr_write_enable = false ; 
					// cout << "stalling" << "\n" ;  
					break ; 
					
					}

				}

				if (!is_RR_stall){

					string instruction_name = ID2_RR.currcomand[0] ; 
					if (instruction_name == "add" || instruction_name == "sub" || instruction_name == "mul" || instruction_name == "slt"){
						
						rd = (ID2_RR.currcomand)[1];
						rs = (ID2_RR.currcomand)[2];
						rt = (ID2_RR.currcomand)[3] ;
						
						RR_EX.RegReadData1 = registers[registerMap[rs]] ; 
						RR_EX.RegReadData2 = registers[registerMap[rt]] ; 
						
						// cout << "registers read are " << rs << " " << RR_EX.RegReadData1 << rt << " " << RR_EX.RegReadData2 << '\n' ;  

						RR_EX.RD = registerMap[rd] ; 
						RR_EX.RT = registerMap[rt] ; 
						RR_EX.PC_new = ID2_RR.at_PC + 1;
						
						}


					else if (instruction_name == "lw" || instruction_name == "sw"){ 
						
						int lparen ; int offset ; 
						if ((ID2_RR.currcomand)[2].back() == ')'){
							lparen = (ID2_RR.currcomand)[2].find('('), offset = stoi(lparen == 0 ? "0" : (ID2_RR.currcomand)[2].substr(0, lparen));
							rs = (ID2_RR.currcomand)[2].substr(lparen + 1);
							rs.pop_back();

							RR_EX.RegReadData1 = registers[registerMap[rs]] ; 
							RR_EX.immediate = offset ; 

						}
						else {
							RR_EX.immediate = stoi((ID2_RR.currcomand)[2]) ; 
							RR_EX.RegReadData1 = 0 ; 
						}
						

						rt = (ID2_RR.currcomand)[1];
						RR_EX.RegReadData2 = registers[registerMap[rt]] ; 
						RR_EX.RS = registerMap[rs] ; 
						RR_EX.RT = registerMap[rt] ;  
						RR_EX.PC_new = ID2_RR.at_PC + 1;
						
					}

					else if (instruction_name == "addi") { 
						rs = (ID2_RR.currcomand)[2]; 
						RR_EX.immediate = stoi(ID2_RR.currcomand[3]) ;
						rd = (ID2_RR.currcomand)[1];

						RR_EX.RegReadData1 = registers[registerMap[rs]] ; 
						RR_EX.RD = registerMap[rd] ; 
						RR_EX.PC_new = ID2_RR.at_PC + 1 ; 
					}

					else if (instruction_name == "beq" || instruction_name == "bne"){
							rs = (ID2_RR.currcomand)[1]; 
							rt = (ID2_RR.currcomand)[2]; 

						RR_EX.RegReadData1 = registers[registerMap[rs]] ; 
						RR_EX.RegReadData2 = registers[registerMap[rt]] ; 
						RR_EX.PC_new = address[ID2_RR.currcomand[3]] ; 
					}
					RR_EX.is_9_stage = ID2_RR.is_9_stage ; 

					(RR_EX.WB)->RegWrite = (ID2_RR.WB)->RegWrite;
					(RR_EX.WB)->MemtoReg = (ID2_RR.WB)->MemtoReg;

					(RR_EX.MEM1)->MemRead = (ID2_RR.MEM1)->MemRead ; 

					(RR_EX.MEM2)->MemWrite = (ID2_RR.MEM2)->MemWrite ;

					(RR_EX.EX)->ALUSrc = (ID2_RR.EX)->ALUSrc ;
					(RR_EX.EX)->branch = (ID2_RR.EX)->branch ;
					(RR_EX.EX)->branch_not = (ID2_RR.EX)->branch_not ;
					(RR_EX.EX)->OP = (ID2_RR.EX)->OP;
					(RR_EX.EX)->RegDst = (ID2_RR.EX)->RegDst;
				}

                    
                }
            RR_EX.at_PC = ID2_RR.at_PC ; 
			prev_is_9_stage = ID2_RR.is_9_stage ; 
			prev_PC = ID2_RR.at_PC ; 


			// ID2 phase  (decoding control signals)
			// cout << "ID2" <<"\n" ; 
			
			if(ID1_ID2.at_PC >= 0 && id2_rr_write_enable) {
				string instruction_name = (ID1_ID2.currcomand)[0];
				// cout << "decoding instruction- "   ; 
				// print_vector(ID1_ID2.currcomand) ; 
				
                    // setting the control signals
				
					(ID2_RR.EX)->RegDst = true ; (ID2_RR.EX)->OP = 0 ; (ID2_RR.EX)->ALUSrc = false ;  

					if (instruction_name == "add"){(ID2_RR.EX)->OP = 0 ; }
					if (instruction_name == "sub"){(ID2_RR.EX)->OP= 1 ; }
					if (instruction_name == "mul") { (ID2_RR.EX)->OP = 2 ; } 
					if (instruction_name == "slt") { (ID2_RR.EX)->OP = 3 ; } 

					(ID2_RR.EX)->branch = false; (ID2_RR.MEM1)->MemRead = false; (ID2_RR.MEM2)->MemWrite = false;
                    (ID2_RR.EX)->branch_not = false ; 
					
					(ID2_RR.WB)->RegWrite = true ; (ID2_RR.WB)->MemtoReg = false ; 
					
					if (instruction_name == "lw"){ 
						
                        (ID2_RR.EX)->RegDst =0;
						(ID2_RR.EX)->ALUSrc =1;
						(ID2_RR.MEM1)->MemRead =1;
						(ID2_RR.WB)->MemtoReg =1;					
					}
					else if (instruction_name == "sw"){
                        
						(ID2_RR.EX)->ALUSrc =1;
						(ID2_RR.MEM2)->MemWrite=1;
						(ID2_RR.WB)->RegWrite=0;					
					}
					else if (instruction_name == "beq" ){ // inserting a stall whenver encountering a beq instruction as per piazza instruction 
						(ID2_RR.EX)->branch=1;
						(ID2_RR.EX)->branch_not=0;
						(ID2_RR.EX)->OP =1;  
						(ID2_RR.WB)->RegWrite=0;					
					}
					else if (instruction_name == "bne" ){ // inserting a stall whenver encountering a beq instruction as per piazza instruction 
						(ID2_RR.EX)->branch=0;
						(ID2_RR.EX)->branch_not=1;
						(ID2_RR.EX)->OP =1;  
						(ID2_RR.WB)->RegWrite=0;					
					}
					
					else if (instruction_name =="j"){
                        is_j_stall = true ;
                        PCnext = address[ ID1_ID2.currcomand[1] ] ; 
                        ID2_RR.PC_new = PCnext  ; 
                        (ID2_RR.WB)->RegWrite =0;
                        // cout << "jumping to new PC " << PCnext << "\n"  ; 
                        // cout << "inserting three stalls " << "\n" ; 
						
					}
					else if (instruction_name == "addi"){
						(ID2_RR.EX)->ALUSrc=1;	 
					}
                ID2_RR.currcomand = ID1_ID2.currcomand ;
				ID2_RR.is_9_stage = ID1_ID2.is_9_stage ; 
				ID2_RR.at_PC = ID1_ID2.at_PC ;
			}
			else if (!id2_rr_write_enable){
				// cout << "stalling" << "\n" ; 
			}
			else{
				ID2_RR.at_PC = ID1_ID2.at_PC ;
			}


            // ID1 STAGE (detect hazards in this stage ?)
            // cout <<"ID1 stage " << "\n" ; 
            if (IF2_ID1.at_PC >= 0 && id1_id2_write_enable){
                // write stall detection logic 
                bool temp = false ;  string instruction_name = (IF2_ID1.currcomand)[0]  ; 
                temp = (instruction_name == "lw"  || instruction_name == "sw" ) ; 
                if (ID1_ID2.at_PC >= 0 && ID1_ID2.is_9_stage && !temp){ // 9 stage followed by 7 stage 
                    is_fifo_stall = true ; if1_if2_write_enable =  if2_id1_write_enable = false ; 
                    // cout << "stalling due to fifo type 1" << "\n" ; 
                }
				
                else if (prev_PC >= 0 && prev_is_9_stage && !temp){ // 9 stage followed by 7 stage 
                    is_fifo_stall = true ; if1_if2_write_enable =  if2_id1_write_enable = false ;
                    // cout << "stalling due to fifo type 2" << "\n" ; 
                }

                ID1_ID2.is_9_stage = temp ; 
                ID1_ID2.currcomand = IF2_ID1.currcomand ;
				ID1_ID2.at_PC = IF2_ID1.at_PC ; 
            }
			else if (!id1_id2_write_enable){
                // cout << "stalling " << "\n" ; 
            }
			else {
				ID1_ID2.at_PC = IF2_ID1.at_PC ; 
			}
            
			

			// IF2 stage 
			// cout << "IF2 stage" << "\n" ;
			
			if ( IF1_IF2.at_PC >= 0 && if2_id1_write_enable) {
				 
                IF2_ID1.at_PC = IF1_IF2.at_PC ;
                IF2_ID1.currcomand = commands[IF2_ID1.at_PC] ; // fetching the instruction
                // cout << "fetched instruction is " << "\n" ; 
                // print_vector(IF2_ID1.currcomand) ;
				
                } 
            else if ( !if2_id1_write_enable){
                // cout << "stalling" << "\n" ; 
            }
			else {
				// cout << "no fetched instruction" << "\n" ; 
				IF2_ID1.at_PC = IF1_IF2.at_PC ;
			}


            // IF1 stage 
            // cout << "IF1 stage" << "\n" ; 
			if ( (PCcurr >= 0 && PCcurr < commands.size()) && if1_if2_write_enable ){
				 
                IF1_IF2.at_PC = PCcurr ;
                // cout << "fetched pc is " << PCcurr << "\n" ; 
			} 
            else if (!if1_if2_write_enable){
                // cout << "stalling" << "\n" ; 
            }
			else {
				// cout << "no fetched instruction" << "\n" ; 
				IF1_IF2.at_PC = -1 ;
			}

            // fifo stall 
            if (is_fifo_stall && !is_j_stall && !is_beq_stall && !is_RR_stall){
				// cout << "doing fifo stalls " << "\n" ; 
                ID1_ID2.at_PC = -1 ; 
                PCnext = PCcurr ; 
            }
			
			
			// doing j stalls
			if (is_j_stall && !is_beq_stall&& !is_RR_stall){
				IF1_IF2.at_PC = -1 ; 
                IF2_ID1.at_PC = -1 ; 
                ID1_ID2.at_PC = -1 ;  
				// cout << "performing j stalls " << "\n" ; 
			}

			// doing data dependency stalls
			if (is_RR_stall && !is_beq_stall){
				RR_EX.at_PC = -1 ; 
				PCnext = PCcurr ;  
				// cout << "rr stalling" << '\n' ; 
			}

			// doing beq stalls
			if (is_beq_stall){
				// cout << "doing beq stalls" << "\n" ; 
				IF1_IF2.at_PC = -1 ; 
                IF2_ID1.at_PC = -1 ; 
                ID1_ID2.at_PC = -1 ; 
                ID2_RR.at_PC = -1 ; 
                RR_EX.at_PC = -1; 
			}

			

			// updating program counter
			if (PCnext >= commands.size()) { PCnext = -1 ; }  
			PCcurr = PCnext ;  
			is_RR_stall = false ; is_beq_stall = false ; is_j_stall = false ; check_rs = check_rt = false ; 
            is_fifo_stall = false ; if1_if2_write_enable = if2_id1_write_enable = id1_id2_write_enable = id2_rr_write_enable =  true ; 
			vi.clear() ; 
			printRegistersAndMemoryDelta(clockCycles);
			cout << "\n" ; 
		}
		// printRegisters() ; 
		// cout << "printing memory " << "\n" ; 
		// cout << data[83] << "\n" ; 
		

		
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

};

#endif