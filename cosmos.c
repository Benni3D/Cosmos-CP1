#define _CRT_SECURE_NO_WARNINGS
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

typedef union Opcode {
	uint16_t value;
	struct {
		uint8_t op, data;
	};
} Opcode;
struct OpcodeInfo {
	const char* name;
	bool has_param;
};

static uint8_t acc, pc;
static Opcode mem[128];
static bool condition;
static volatile bool running;
static bool crash;
static struct OpcodeInfo opc_infos[] = {
	{ "data", true },
	{ "hlt", false },
	{ "print", false },
	{ "sleep", true },
	{ "ldc", true },
	{ "lda", true },
	{ "sta", true },
	{ "add", true },
	{ "sub", true },
	{ "jmp", true },
	{ "cmpe", true },
	{ "jc", true },
	{ "cmpgr", true },
	{ "cmple", true },
	{ "neg", false },
	{ "and", true },
	{ NULL, false },
	{ NULL, false },
	{ "pout", false }
};
static bool strieq(const char* s1, const char* s2) {
	for (size_t i = 0; ; ++i) {
		if (tolower(s1[i]) != tolower(s2[i])) return false;
		else if (s1[i] == 0) return true;
	}
}


static void reset(void) {
	acc = pc = 0;
	for (uint8_t i = 0; i < 128; ++i)
		mem[i].value = 0;
	running = condition = false;
}
#ifdef _DEBUG
#define wait() getchar()
#else
#define wait()
#endif
/*
	F001: Input Mistake
	F002: Illegal Opcode
	F003: Illegal Address
	F004: Invalid Opc/Addr input
	F005: Invalid Operand
	F006: Overflow/Underflow
	F007: Cassette Error
*/
static void error(int err) {
	static const char* msg[] = {
		NULL,
		"Input Mistake",
		"Illegal Opcode",
		"Illegal Address",
		"Invalid Opcode or Address input",
		"Invalid Operand",
		"Result bigger than 255 or less than 0",
		"Invalid values in Cassette"
	};
	puts(msg[err]);
	wait();
	exit(1);
}

inline static void check_addr(uint8_t addr) {
	if (addr > 127) error(3);
}
static void write(uint8_t addr, uint8_t val) {
	check_addr(addr);
	mem[addr].op = 0;
	mem[addr].data = val;
	// printf("write(%03d, %02d.%03d);\n", addr, opc.op, opc.data);
}
static Opcode read(uint8_t addr) {
	return check_addr(addr), mem[addr];
}

static void print_opc(Opcode opc) {
	printf("opc: %s", opc_infos[opc.op].name);
	if (opc_infos[opc.op].has_param)
		printf(" %d", opc.data);
	putchar('\n');
}
static void exec(Opcode op) {
	int16_t tmp;
	switch (op.op) {
	case 0: // DATA
		break;
	case 1: // HLT
		running = false;
		break;
	case 2: // PRINT
		printf("%03d\n", acc);
		break;
	case 3: // SLEEP xxx
		
		break;
	case 4: // LDC xxx
		acc = op.data;
		break;
	case 5: // LDA mem
		acc = read(op.data).data;
		break;
	case 6: // STA mem
		write(op.data, acc);
		break;
	case 7: // ADD mem
		tmp = acc + read(op.data).data;
		if (tmp > 255) { error(6); break; }
		acc = tmp;
		break;
	case 8: // SUB mem
		tmp = acc - read(op.data).data;
		if (tmp < 0) { error(6); break; }
		acc = tmp;
		break;
	case 9: // JMP addr
		pc = op.data;
		break;
	case 10: // CMPE mem
		condition = acc == read(op.data).data;
		break;
	case 11: // JC addr
		if (condition) pc = op.data;
		break;
	case 12: // CMPGR mem
		condition = acc > read(op.data).data;
		break;
	case 13: // CMPLE mem
		condition = acc < read(op.data).data;
		break;
	case 14: // NEG
		acc = -acc;
		break;
	case 15: // AND mem
		acc &= read(op.data).data;
		break;
	case 18: // POUT
		printf("out(%03d);\n", acc);
		break;
	default:
		error(2);
		break;
	}
}
static void run(void) {
	running = true;
	while (running) exec(mem[pc++]);
}

static uint8_t decode_opc(const char* str) {
	for (uint8_t i = 0; i <= 18; ++i) {
		if (opc_infos[i].name != NULL && strieq(opc_infos[i].name, str))
			return i;
	}
	printf("%03d: Unknown opcode '%s'\n", pc, str);
	wait();
	exit(1);
}
static void decode(FILE* file) {
	static char buf[50];
	size_t i = 0;
	char ch = fgetc(file);
	while (isspace(ch)) ch = fgetc(file);
	if (feof(file)) return;
	if (ch == '@') {
		int addr = 0;
		ch = fgetc(file);
		if (!isdigit(ch)) {
			puts("expected number after @");
			error(1);
		}
		while (isdigit(ch)) addr = addr * 10 + (ch - '0'), ch = fgetc(file);
		check_addr(addr);
		pc = addr;
		return;
	}
	else if (ch == ';') {
		while (ch != '\n') ch = fgetc(file);
		return;
	}
	while (isalpha(ch)) buf[i++] = ch, ch = fgetc(file);
	buf[i++] = '\0';
	Opcode opc;
	opc.op = decode_opc(buf);
	if (ch == ' ') {
		int val = 0;
		ch = fgetc(file);
		if (!isdigit(ch)) {
			printf("%03d: expected number got '%c'\n", pc, ch);
			error(1);
		}
		while (isdigit(ch)) (val = val * 10 + (ch - '0')), ch = fgetc(file);
		if (val > 255) {
			printf("%03d: operand too big: %d\n", pc, val);
			error(5);
		}
		opc.data = val;
	}
	else opc.data = 0;
	mem[pc++] = opc;
	if (ch != '\n')
	while (fgetc(file) != '\n' && !feof(file));
}

int main(int argc, char** argv) {
	if (argc < 2) {
		usage:
		printf("Usage: %s -h\n", argv[0]);
		wait();
		return 1;
	}
	const char* filename = NULL;
	bool show_asm = false;
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-h") == 0) {
			printf("Usage: %s [-a] <source>\n", argv[0]);
			printf("\t-a\t\tshow assembly output\n");
			return 0;
		}
		else if (strcmp(argv[i], "-a") == 0)
			show_asm = true;
		else filename = argv[i];
	}

	if (!filename) goto usage;
	FILE* file = fopen(filename, "r");
	if (!file) {
		printf("File %s not found!\n", filename);
		wait();
		return 1;
	}
	reset();
	while (!feof(file)) decode(file);

	if (show_asm) {
		for (uint8_t i = 0; i < 128; ++i) {
			Opcode opc = mem[i];
			if (opc.value) printf("%02d.%03d\n", opc.op, opc.data);
		}
	}
	else {
		puts("Start\n");
		pc = 0;
		run();
		puts("\nEnd");
	}
	wait();
	return 0;
}