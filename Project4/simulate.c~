/*
 * Instruction-level simulator for the LC
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000
#define MAXCACHESIZE 256

#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5
#define HALT 6
#define NOOP 7

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

typedef struct blockStruct
{
	int tag;
	int valid;
	int dirty;
}block;
typedef struct cacheStruct
{
	block blockMem[MAXCACHESIZE][MAXCACHESIZE];
}cache;

enum actionType
        {cacheToProcessor, processorToCache, memoryToCache, cacheToMemory,
        cacheToNowhere};

void printState(stateType *);
void run(stateType state, cache cacheMem, int blockSizeInWords, int numberOfSets, int blocksPerSets);
int convertNum(int);
void printAction(int address, int size, enum actionType type);


int
main(int argc, char *argv[])
{
    int i;
    int j;	
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;
    cache cacheMem;
    
    for(i=0;i<MAXCACHESIZE;i++)
    {
    	for(j=0;j<MAXCACHESIZE;j++)
    	{
    		cacheMem.blockMem[i][j].tag = -1;
    		cacheMem.blockMem[i][j].valid = 0;
    		cacheMem.blockMem[i][j].dirty = 0;
    	}
    }

    if (argc != 5) {
	printf("error: usage: %s <machine-code file> <blockSizeInWords> <numberOfSets> <blocksPerSet>\n", argv[0]);
	exit(1);
    }
    
    int blockSizeInWords = atoi(argv[2]);
    int numberOfSets = atoi(argv[3]);
    int blocksPerSet = atoi(argv[4]);

    /* initialize memories and registers */
    for (i=0; i<NUMMEMORY; i++) {
	state.mem[i] = 0;
    }
    for (i=0; i<NUMREGS; i++) {
	state.reg[i] = 0;
    }

    state.pc=0;

    /* read machine-code file into instruction/data memory (starting at
	address 0) */

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
	printf("error: can't open file %s\n", argv[1]);
	perror("fopen");
	exit(1);
    }

    for (state.numMemory=0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
	state.numMemory++) {
	if (state.numMemory >= NUMMEMORY) {
	    printf("exceeded memory size\n");
	    exit(1);
	}
	if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
	    printf("error in reading address %d\n", state.numMemory);
	    exit(1);
	}
	printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    printf("\n");
    
    /* run never returns */
    run(state, cacheMem, blockSizeInWords, numberOfSets, blocksPerSet);
    return(0);
}

/*
 * Log the specifics of each cache action.
 *
 * address is the starting word address of the range of data being transferred.
 * size is the size of the range of data being transferred.
 * type specifies the source and destination of the data being transferred.
 *     cacheToProcessor: reading data from the cache to the processor
 *     processorToCache: writing data from the processor to the cache
 *     memoryToCache: reading data from the memory to the cache
 *     cacheToMemory: evicting cache data by writing it to the memory
 *     cacheToNowhere: evicting cache data by throwing it away
 */
void
printAction(int address, int size, enum actionType type)
{
    printf("@@@ transferring word [%d-%d] ", address, address + size - 1);
    if (type == cacheToProcessor) {
        printf("from the cache to the processor\n");
    } else if (type == processorToCache) {
        printf("from the processor to the cache\n");
    } else if (type == memoryToCache) {
        printf("from the memory to the cache\n");
    } else if (type == cacheToMemory) {
        printf("from the cache to the memory\n");
    } else if (type == cacheToNowhere) {
        printf("from the cache to nowhere\n");
    }
}
void
run(stateType state, cache cacheMem, int blockSizeInWords, int numberOfSets, int blocksPerSets)
{
    int i;
    int j;
    int arg0, arg1, arg2, addressField;
    int instructions=0;
    int opcode;
    int maxMem=-1;	/* highest memory address touched during run */

    int use=1;
    for (; 1; instructions++) { /* infinite loop, exits when it executes halt */
	printState(&state);
	
	if (state.pc < 0 || state.pc >= NUMMEMORY) {
	    printf("pc went out of the memory range\n");
	    exit(1);
	}

//	cacheMem.blockMem[0][0].tag=12;
//	printf("cache state:\n");
//	for(i=0;i<numberOfSets;i++)
//	{
//		for(j=0;j<blocksPerSets;j++)
//		{
//			printf("~~%d %d %d     ",cacheMem.blockMem[i][j].tag,cacheMem.blockMem[i][j].valid,cacheMem.blockMem[i][j].dirty);
//		}
//		printf("\n");
//	}

	maxMem = (state.pc > maxMem)?state.pc:maxMem;

	/* this is to make the following code easier to read */
	opcode = state.mem[state.pc] >> 22;
	arg0 = (state.mem[state.pc] >> 19) & 0x7;
	arg1 = (state.mem[state.pc] >> 16) & 0x7;
	arg2 = state.mem[state.pc] & 0x7; /* only for add, nand */

        int offset = state.pc%blockSizeInWords;
        int LineIndex = (state.pc/blockSizeInWords)%numberOfSets;
        int TagRequired = state.pc/blockSizeInWords;

	int hit = 0;
	for(i=0;i<blocksPerSets;i++)
	{
		if(cacheMem.blockMem[LineIndex][i].tag == TagRequired)
	    	{
	    	    hit = 1;
	    	    cacheMem.blockMem[LineIndex][i].valid = use++;
	    	    break;
	    	}
	}
	if (hit==1)
	{
		printAction(state.pc, 1, cacheToProcessor);
	} 
	else
	{
		int change = 0;
		int LRU = cacheMem.blockMem[LineIndex][0].valid;
		for(i=0;i<blocksPerSets;i++)
		{
			if (cacheMem.blockMem[LineIndex][i].valid < LRU)
			{
				change = i;
				LRU = cacheMem.blockMem[LineIndex][i].valid;
			}
		}
//		printf("##%d",change);
		if(cacheMem.blockMem[LineIndex][change].dirty == 1)
		{
			if(cacheMem.blockMem[LineIndex][change].valid != 0)
			{
				printAction(cacheMem.blockMem[LineIndex][change].tag*blockSizeInWords, blockSizeInWords, cacheToMemory);
			}
		}
		else
		{
			if(cacheMem.blockMem[LineIndex][change].valid != 0)
			{
				printAction(cacheMem.blockMem[LineIndex][change].tag*blockSizeInWords, blockSizeInWords, cacheToNowhere);
			}
		}
		cacheMem.blockMem[LineIndex][change].valid = use++;
//		printf("!!! %d",cacheMem.blockMem[LineIndex][change].valid);
		cacheMem.blockMem[LineIndex][change].tag = TagRequired;
//		printf("!!! %d",cacheMem.blockMem[LineIndex][change].tag);
		printAction(cacheMem.blockMem[LineIndex][change].tag*blockSizeInWords, blockSizeInWords, memoryToCache);
		cacheMem.blockMem[LineIndex][change].dirty = 0;
		printAction(state.pc, 1, cacheToProcessor);
	}



	addressField = convertNum(state.mem[state.pc] & 0xFFFF); /* for beq,		
								    lw, sw */
								    
	state.pc++;
	if (opcode == ADD) {
	    state.reg[arg2] = state.reg[arg0] + state.reg[arg1];
	} else if (opcode == NAND) {
	    state.reg[arg2] = ~(state.reg[arg0] & state.reg[arg1]);
	} else if (opcode == LW) {
	    if (state.reg[arg0] + addressField < 0 ||
		    state.reg[arg0] + addressField >= NUMMEMORY) {
		printf("address out of bounds\n");
		exit(1);
	    }
	    state.reg[arg1] = state.mem[state.reg[arg0] + addressField];
	    if (state.reg[arg0] + addressField > maxMem) {
		maxMem = state.reg[arg0] + addressField;
	    }
//	    printf("cache state:\n");
//	    for(i=0;i<numberOfSets;i++)
//	    {
//		for(j=0;j<blocksPerSets;j++)
//		{
//			printf("~~%d %d %d     ",cacheMem.blockMem[i][j].tag,cacheMem.blockMem[i][j].valid,cacheMem.blockMem[i][j].dirty);
//		}
//		printf("\n");
//	    }
	    offset = (state.reg[arg0] + addressField)%blockSizeInWords;
	    LineIndex = ((state.reg[arg0] + addressField)/blockSizeInWords)%numberOfSets;
	    TagRequired = (state.reg[arg0] + addressField)/blockSizeInWords;
	    hit = 0;
	    for(i=0;i<blocksPerSets;i++)
	    {
		if(cacheMem.blockMem[LineIndex][i].tag == TagRequired)
	    	{
	    	    hit = 1;
	    	    cacheMem.blockMem[LineIndex][i].valid = use++;
	    	    break;
	    	}
	    }
	    if (hit==1)
	    {
		printAction(state.reg[arg0] + addressField, 1, cacheToProcessor);
	    } 
	    else
	    {
		int change = 0;
		int LRU = cacheMem.blockMem[LineIndex][0].valid;
		for(i=0;i<blocksPerSets;i++)
		{
			if (cacheMem.blockMem[LineIndex][i].valid < LRU)
			{
				change = i;
				LRU = cacheMem.blockMem[LineIndex][i].valid;
			}
		}
		if(cacheMem.blockMem[LineIndex][change].dirty == 1)
		{
			if(cacheMem.blockMem[LineIndex][change].valid != 0)
			{
				printAction(cacheMem.blockMem[LineIndex][change].tag*blockSizeInWords, blockSizeInWords, cacheToMemory);
			}
		}
		else
		{
			if(cacheMem.blockMem[LineIndex][change].valid != 0)
			{
				printAction(cacheMem.blockMem[LineIndex][change].tag*blockSizeInWords, blockSizeInWords, cacheToNowhere);
			}
		}
		cacheMem.blockMem[LineIndex][change].valid = use++;
		cacheMem.blockMem[LineIndex][change].tag = TagRequired;
		printAction(cacheMem.blockMem[LineIndex][change].tag*blockSizeInWords, blockSizeInWords, memoryToCache);
		cacheMem.blockMem[LineIndex][change].dirty = 0;
		printAction(state.reg[arg0] + addressField, 1, cacheToProcessor);
       	    }
	    
	    
	    
	} else if (opcode == SW) {
	    if (state.reg[arg0] + addressField < 0 ||
		    state.reg[arg0] + addressField >= NUMMEMORY) {
		printf("address out of bounds\n");
		exit(1);
	    }
	    state.mem[state.reg[arg0] + addressField] = state.reg[arg1];
	    if (state.reg[arg0] + addressField > maxMem) {
		maxMem = state.reg[arg0] + addressField;
	    }
//	    printf("cache state:\n");
//	    for(i=0;i<numberOfSets;i++)
//	    {
//		for(j=0;j<blocksPerSets;j++)
//		{
//			printf("~~%d %d %d     ",cacheMem.blockMem[i][j].tag,cacheMem.blockMem[i][j].valid,cacheMem.blockMem[i][j].dirty);
//		}
//		printf("\n");
//	    }
	    
	    offset = (state.reg[arg0] + addressField)%blockSizeInWords;
	    LineIndex = ((state.reg[arg0] + addressField)/blockSizeInWords)%numberOfSets;
	    TagRequired = (state.reg[arg0] + addressField)/blockSizeInWords;
	    hit = 0;
	    for(i=0;i<blocksPerSets;i++)
	    {
		if(cacheMem.blockMem[LineIndex][i].tag == TagRequired)
	    	{
	    	    hit = 1;
	    	    cacheMem.blockMem[LineIndex][i].valid = use++;
	    	    cacheMem.blockMem[LineIndex][i].dirty = 1;
	    	    break;
	    	}
	    }
	    if (hit==1)
	    {
		printAction(state.reg[arg0] + addressField, 1, processorToCache);
		
	    } 
	    else
	    {
		int change = 0;
		int LRU = cacheMem.blockMem[LineIndex][0].valid;
		for(i=0;i<blocksPerSets;i++)
		{
			if (cacheMem.blockMem[LineIndex][i].valid < LRU)
			{
				change = i;
				LRU = cacheMem.blockMem[LineIndex][i].valid;
			}
		}
		if(cacheMem.blockMem[LineIndex][change].dirty == 1)
		{
			if(cacheMem.blockMem[LineIndex][change].valid != 0)
			{
				printAction(cacheMem.blockMem[LineIndex][change].tag*blockSizeInWords, blockSizeInWords, cacheToMemory);
			}
		}
		else
		{
			if(cacheMem.blockMem[LineIndex][change].valid != 0)
			{
				printAction(cacheMem.blockMem[LineIndex][change].tag*blockSizeInWords, blockSizeInWords, cacheToNowhere);
			}
		}
		cacheMem.blockMem[LineIndex][change].valid = use++;
		cacheMem.blockMem[LineIndex][change].tag = TagRequired;
		printAction(cacheMem.blockMem[LineIndex][change].tag*blockSizeInWords, blockSizeInWords, memoryToCache);
		printAction(state.reg[arg0] + addressField, 1, processorToCache);
		cacheMem.blockMem[LineIndex][change].dirty = 1;
       	    }
	} else if (opcode == BEQ) {
	    if (state.reg[arg0] == state.reg[arg1]) {
		state.pc += addressField;
	    }
	} else if (opcode == JALR) {
	    state.reg[arg1] = state.pc;
            if(arg0 != 0)
 		state.pc = state.reg[arg0];
	    else
		state.pc = 0;
	} else if (opcode == NOOP) {
	} else if (opcode == HALT) {
	    printf("machine halted\n");
	    printf("total of %d instructions executed\n", instructions+1);
	    printf("final state of machine:\n");
	    printState(&state);
	    exit(0);
	} else {
	    printf("error: illegal opcode 0x%x\n", opcode);
	    exit(1);
	}
        state.reg[0] = 0;
    }
}

void
printState(stateType *statePtr)
{
    int i;
    printf("\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
	for (i=0; i<statePtr->numMemory; i++) {
	    printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
	}
    printf("\tregisters:\n");
	for (i=0; i<NUMREGS; i++) {
	    printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
	}
    printf("end state\n");
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

