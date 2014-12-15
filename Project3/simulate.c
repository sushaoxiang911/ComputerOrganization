#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 /* JALR will not implemented for Project 3 */
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000


typedef struct IFIDStruct {
    int instr;
    int pcPlus1;
} IFIDType;

typedef struct IDEXStruct {
    int instr;
    int pcPlus1;
    int readRegA;
    int readRegB;
    int offset;
} IDEXType;

typedef struct EXMEMStruct {
    int instr;
    int branchTarget;
    int aluResult;
    int readRegB;
} EXMEMType;

typedef struct MEMWBStruct {
    int instr;
    int writeData;
} MEMWBType;

typedef struct WBENDStruct {
    int instr;
    int writeData;
} WBENDType;

typedef struct stateStruct {
    int pc;
    int instrMem[NUMMEMORY];
    int dataMem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
    IFIDType IFID;
    IDEXType IDEX;
    EXMEMType EXMEM;
    MEMWBType MEMWB;
    WBENDType WBEND;
    int cycles; /* number of cycles run so far */
} stateType;



void printState (stateType *statePtr);
int field1(int instruction);
int field2(int instruction);
int field3(int instruction);
int opcode(int instruction);
void printInstruction(int instr);
void run(stateType state);
int convertNum(int num);

int
main(int argc, char *argv[])
{
    int i;
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;
    printf("a");
    if (argc != 2) {
	printf("error: usage: %s <machine-code file>\n", argv[0]);
	exit(1);
    }
    /* initialize memories and registers */
    for (i=0; i<NUMMEMORY; i++) {
	state.dataMem[i] = 0;
	state.instrMem[i]=0;
    }
    for (i=0; i<NUMREGS; i++) {
	state.reg[i] = 0;
    }

    state.pc=0;

    state.cycles=0;
    
    //initialize the pipline registers
    state.IFID.instr=NOOPINSTRUCTION;
    state.IDEX.instr=NOOPINSTRUCTION;
    state.EXMEM.instr=NOOPINSTRUCTION;
    state.MEMWB.instr=NOOPINSTRUCTION;
    state.WBEND.instr=NOOPINSTRUCTION;

    /* read machine-code file into instruction/data memory (starting at
	address 0) */

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s\n", argv[1]);
	perror("fopen");
	exit(1);
    }
//    printf("a");
    for (state.numMemory=0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
	state.numMemory++) {
	if (state.numMemory >= NUMMEMORY) {
	    printf("exceeded memory size\n");
	    exit(1);
	}
	if (sscanf(line, "%d", state.dataMem+state.numMemory) != 1) {
	    printf("error in reading address %d\n", state.numMemory);
	    exit(1);
	}
//      printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }
    
//initialize instrMem
    for(i=0;i<state.numMemory;i++)
    {
        state.instrMem[i]=state.dataMem[i];
    }
    
//print start state    
    for (i=0;i<state.numMemory;i++)
    {
        printf("memory[%d]=%d\n", i, state.dataMem[i]);
    }
    printf("%d memory words\n",state.numMemory);
    printf("\tinstruction memory:\n");
    for (i=0;i<state.numMemory;i++)
    {
        printf("\t\tinstrMem[ %d ] ",i);
        printInstruction(state.instrMem[i]);
    }
    
    printf("\n");
    
    /* run never returns */
    run(state);
    return(0);
}



void
printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate before cycle %d starts\n", statePtr->cycles);
    printf("\tpc %d\n", statePtr->pc);

    printf("\tdata memory:\n");
	for (i=0; i<statePtr->numMemory; i++) {
	    printf("\t\tdataMem[ %d ] %d\n", i, statePtr->dataMem[i]);
	}
    printf("\tregisters:\n");
	for (i=0; i<NUMREGS; i++) {
	    printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
	}
    printf("\tIFID:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->IFID.instr);
	printf("\t\tpcPlus1 %d\n", statePtr->IFID.pcPlus1);
    printf("\tIDEX:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->IDEX.instr);
	printf("\t\tpcPlus1 %d\n", statePtr->IDEX.pcPlus1);
	printf("\t\treadRegA %d\n", statePtr->IDEX.readRegA);
	printf("\t\treadRegB %d\n", statePtr->IDEX.readRegB);
	printf("\t\toffset %d\n", statePtr->IDEX.offset);
    printf("\tEXMEM:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->EXMEM.instr);
	printf("\t\tbranchTarget %d\n", statePtr->EXMEM.branchTarget);
	printf("\t\taluResult %d\n", statePtr->EXMEM.aluResult);
	printf("\t\treadRegB %d\n", statePtr->EXMEM.readRegB);
    printf("\tMEMWB:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->MEMWB.instr);
	printf("\t\twriteData %d\n", statePtr->MEMWB.writeData);
    printf("\tWBEND:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->WBEND.instr);
	printf("\t\twriteData %d\n", statePtr->WBEND.writeData);
}

int
field0(int instruction)
{
    return( (instruction>>19) & 0x7);
}

int
field1(int instruction)
{
    return( (instruction>>16) & 0x7);
}

int
field2(int instruction)
{
    return(instruction & 0xFFFF);
}

int opcode(int instruction)
{
    return(instruction>>22);
}

void
printInstruction(int instr)
{
    char opcodeString[10];
    if (opcode(instr) == ADD) {
	strcpy(opcodeString, "add");
    } else if (opcode(instr) == NAND) {
	strcpy(opcodeString, "nand");
    } else if (opcode(instr) == LW) {
	strcpy(opcodeString, "lw");
    } else if (opcode(instr) == SW) {
	strcpy(opcodeString, "sw");
    } else if (opcode(instr) == BEQ) {
	strcpy(opcodeString, "beq");
    } else if (opcode(instr) == JALR) {
	strcpy(opcodeString, "jalr");
    } else if (opcode(instr) == HALT) {
	strcpy(opcodeString, "halt");
    } else if (opcode(instr) == NOOP) {
	strcpy(opcodeString, "noop");
    } else {
	strcpy(opcodeString, "data");
    }

    printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
	field2(instr));
}

void run(stateType state)
{
    
    stateType newState;
    while (1) {

	printState(&state);

	/* check for halt */
	if (opcode(state.MEMWB.instr) == HALT) {
	    printf("machine halted\n");
	    printf("total of %d cycles executed\n", state.cycles);
	    exit(0);
	}

	newState = state;
	newState.cycles++;

	/* --------------------- IF stage --------------------- */
	if(opcode(state.IDEX.instr)==LW&&(field1(state.IDEX.instr)==field0(state.IFID.instr)||field1(state.IDEX.instr)==field1(state.IFID.instr)))
	{   
//	    printf("a");   
	}
	else
	{
	    newState.pc=state.pc+1;
	    newState.IFID.instr=state.instrMem[state.pc];
	    newState.IFID.pcPlus1=newState.pc;
	}

	/* --------------------- ID stage --------------------- */
	if(opcode(state.IDEX.instr)==LW&&(field1(state.IDEX.instr)==field0(state.IFID.instr)||field1(state.IDEX.instr)==field1(state.IFID.instr)))
	{    
	    newState.IDEX.instr=NOOPINSTRUCTION;
	    newState.IDEX.pcPlus1=state.IFID.pcPlus1;
	    newState.IDEX.readRegA=0;
	    newState.IDEX.readRegB=0;
	    newState.IDEX.offset=0;
	}
	else
	{
	    newState.IDEX.instr=state.IFID.instr;
	    newState.IDEX.pcPlus1=state.IFID.pcPlus1;
	    newState.IDEX.readRegA=state.reg[field0(state.IFID.instr)];
	    newState.IDEX.readRegB=state.reg[field1(state.IFID.instr)];
	    newState.IDEX.offset=convertNum(field2(state.IFID.instr));
	}
	

	/* --------------------- EX stage --------------------- */
	int regAtemp;
	int regBtemp;
	
	
	
	newState.EXMEM.instr=state.IDEX.instr;
	newState.EXMEM.branchTarget=state.IDEX.pcPlus1+state.IDEX.offset;
	
	
	regBtemp=state.IDEX.readRegB;
	regAtemp=state.IDEX.readRegA;
	

	//WBEND with IDEX, for register A
	if(field2(state.WBEND.instr)==field0(state.IDEX.instr)&&(opcode(state.WBEND.instr)==ADD||opcode(state.WBEND.instr)==NAND))
	{
	       regAtemp=state.WBEND.writeData;
	}
	if(field1(state.WBEND.instr)==field0(state.IDEX.instr)&&opcode(state.WBEND.instr)==LW)
	{
               regAtemp=state.WBEND.writeData;
	}
	
	//WBEND with IDEX, for register B
	if(field2(state.WBEND.instr)==field1(state.IDEX.instr)&&(opcode(state.WBEND.instr)==ADD||opcode(state.WBEND.instr)==NAND))
	{
	       regBtemp=state.WBEND.writeData;
	}
	if(field1(state.WBEND.instr)==field1(state.IDEX.instr)&&opcode(state.WBEND.instr)==LW)
	{
	       regBtemp=state.WBEND.writeData;
	}
	
	//MEMWB with IDEX, for register A
	if(field2(state.MEMWB.instr)==field0(state.IDEX.instr)&&(opcode(state.MEMWB.instr)==ADD||opcode(state.MEMWB.instr)==NAND))
	{
	       regAtemp=state.MEMWB.writeData;
	}
	if(field1(state.MEMWB.instr)==field0(state.IDEX.instr)&&opcode(state.MEMWB.instr)==LW)
	{
	       regAtemp=state.MEMWB.writeData;
	}
	
	//MEMWB with IDEX, for register B
	if(field2(state.MEMWB.instr)==field1(state.IDEX.instr)&&(opcode(state.MEMWB.instr)==ADD||opcode(state.MEMWB.instr)==NAND))
	{
	       regBtemp=state.MEMWB.writeData;
	}
	if(field1(state.MEMWB.instr)==field1(state.IDEX.instr)&&opcode(state.MEMWB.instr)==LW)
	{
	       regBtemp=state.MEMWB.writeData;
	}
	
	//EXMEM with IDEX, for register A
	if(field2(state.EXMEM.instr)==field0(state.IDEX.instr)&&(opcode(state.EXMEM.instr)==ADD||opcode(state.EXMEM.instr)==NAND))
	{
	       regAtemp=state.EXMEM.aluResult;
	}
	//EXMEM with IDEX, for register B
	if(field2(state.EXMEM.instr)==field1(state.IDEX.instr)&&(opcode(state.EXMEM.instr)==ADD||opcode(state.EXMEM.instr)==NAND))
	{
	       regBtemp=state.EXMEM.aluResult;
	}
	
	if(opcode(newState.EXMEM.instr)==ADD)
	{
	       newState.EXMEM.aluResult=regAtemp+regBtemp;
	}
	else if(opcode(newState.EXMEM.instr)==NAND)
	{
	       newState.EXMEM.aluResult=~(regAtemp&regBtemp);
	}
	else if(opcode(newState.EXMEM.instr)==LW)
	{
	       newState.EXMEM.aluResult=regAtemp+state.IDEX.offset;
	}
	else if(opcode(newState.EXMEM.instr)==SW)
	{
	       newState.EXMEM.aluResult=regAtemp+state.IDEX.offset;
	}
	else if(opcode(newState.EXMEM.instr)==BEQ)
	{
	       newState.EXMEM.aluResult=regAtemp-regBtemp;
	} 
	else if(opcode(newState.EXMEM.instr)==HALT)
	{
	       newState.EXMEM.aluResult=regAtemp+regBtemp;
	}
	else
	{
	       newState.EXMEM.aluResult=regAtemp+regBtemp;
	}
        
        newState.EXMEM.readRegB=regBtemp;
	/* --------------------- MEM stage --------------------- */
	
	newState.MEMWB.instr=state.EXMEM.instr;
	if(opcode(newState.MEMWB.instr)==ADD||opcode(newState.MEMWB.instr)==NAND)
	{
	    newState.MEMWB.writeData=state.EXMEM.aluResult;
	}
	if(opcode(newState.MEMWB.instr)==LW)
	{
	    newState.MEMWB.writeData=state.dataMem[state.EXMEM.aluResult];
	}
	if(opcode(newState.MEMWB.instr)==SW)
	{
	    newState.dataMem[state.EXMEM.aluResult]=state.EXMEM.readRegB;
	}
	if(opcode(newState.MEMWB.instr)==BEQ&&state.EXMEM.aluResult==0)
	{
	    newState.pc=state.EXMEM.branchTarget;
	    newState.IFID.instr=NOOPINSTRUCTION;
	    newState.IDEX.instr=NOOPINSTRUCTION;
	    newState.EXMEM.instr=NOOPINSTRUCTION;
	}

	/* --------------------- WB stage --------------------- */
	newState.WBEND.instr=state.MEMWB.instr;
	newState.WBEND.writeData=state.MEMWB.writeData;
	
	if(opcode(newState.WBEND.instr)==LW)
	{
	    newState.reg[field1(newState.WBEND.instr)]=state.MEMWB.writeData;
	}
	if(opcode(newState.WBEND.instr)==ADD||opcode(newState.WBEND.instr)==NAND)
	{
	    newState.reg[field2(newState.WBEND.instr)]=state.MEMWB.writeData;
	}

	state = newState; /* this is the last statement before end of the loop.
			    It marks the end of the cycle and updates the
			    current state with the values calculated in this
			    cycle */
			    
			    
    }

}

int
convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit Sun integer */
    if (num & (1<<15) ) {
	num -= (1<<16);
    }
    return(num);
}





