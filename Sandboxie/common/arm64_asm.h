#ifndef ARM64_ASM_H
#define ARM64_ASM_H

typedef union {
    ULONG OP;
    struct {
        ULONG
            Rd : 5,
            imm16 : 16,
            hw : 2,
            op1 : 6,
            op2 : 2,
            sf : 1;
    };
} MOV;

#define IS_MOV(x) (x.hw == 0b00 && x.op1 == 0b100101 && x.op2 == 0b10 && x.sf == 0b1)

typedef union {
    ULONG OP;
    struct {
        ULONG
            imm26 : 26,
            op1 : 5,
            op2 : 1;
    };
} B;

#define IS_B(x) (x.op1 == 0b00101 && x.op2 == 0b0)

typedef union {
    ULONG OP;
    struct {
        ULONG
            cond : 4,
            zero1 : 1,
            imm19 : 19,
            zero2 : 1,
            op : 7;
    };
} B_COND;

#define IS_B_COND(x) (x.zero1 == 0b0 && x.zero2 == 0b0 && x.op == 0b0101010)

typedef union {
    ULONG OP;
    struct {
        ULONG
            imm26 : 26,
            op1 : 5,
            op2 : 1;
    };
} BL;

#define IS_BL(x) (x.op1 == 0b00101 && x.op2 == 0b1)

typedef union {
    ULONG OP;
    struct {
		ULONG
			zero1 : 5,
			rn : 5,
			zero2 : 6,
			one : 5,
			op1 : 2,
			zero3 : 2,
			op2 : 7;
    };
} BR;

#define IS_BR(x) (x.op2 == 0b1101011 && x.zero3 == 0b00 && x.op1 == 0b00 && x.one == 0b11111)

typedef union {
    ULONG OP;
    struct {
        ULONG
            Rd : 5,
            immHi : 19,
            op1 : 5,
            immLo : 2,
            op2 : 1;
    };
} ADRP;

#define IS_ADRP(x) (x.op1 == 0b10000 && x.op2 == 0b1)

typedef union {
    ULONG OP;
    struct {
		ULONG
			Rt : 5,
			Rn : 5,
			imm12 : 12,
			op1 : 2,
			op2 : 2,
			op3 : 1,
			op4 : 3,
			size : 2;
    };
} LDR;

#define IS_LDR(x) (x.op4 == 0b111 && x.op3 == 0b0 && x.op2 == 0b01 && x.op1 == 0b01)

typedef union {
    ULONG OP;
    struct {
		ULONG
			Rt : 5,
			Rn : 5,
			zero : 2,
			imm9 : 9,
			op1: 1,
			op2 : 2,
			op3 : 2,
			op4 : 1,
			op5 : 3,
			size : 2;
    };
} LDUR;

#define IS_LDUR(x) (x.size = 0b11 && x.op5 == 0b111 && x.op4 == 0b0 && x.op3 == 0b01 && x.op2 == 0b01 && x.op1 == 0b0 && x.zero == 0b00)

typedef union {
    ULONG OP;
    struct {
        ULONG
            Rd : 5,
            Rn : 5,
            imm12 : 12,
            shift : 2,
            op1 : 5,
            S : 1,
            op2 : 1,
            sf : 1;
    };
} ADD;

#define IS_ADD(x) (x.sf == 0b1 && x.op2 == 0b0 && x.S == 0b0 && x.op1 == 0b10001 && x.shift == 0)

typedef union {
    ULONG OP;
    struct {
        ULONG
            op1 : 2,
            zero1 : 3,
            imm16 : 16,
            zero2 : 3,
            op2 : 8;
    };
} SVC;

#define IS_SVC(x) (x.op1 == 0x01 && x.zero1 == 0b000 && x.zero2 == 0b000 && x.op2 == 0b11010100)

#endif /* ARM64_ASM_H */