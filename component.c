
/*
 * Designer:   YIN ZIQI,
        1809853D-I011-0160,
        yinziqi.celia@outlook.com
*/

#include "minicpu.h"

/* ALU */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    *Zero=0;

    if(ALUControl==0x0){
        *ALUresult=(signed)A + (signed)B;//add
        if(*ALUresult==0)
            *Zero=1;
    }
    else if(ALUControl==0x1){
        *ALUresult=(signed)A - (signed)B;//sub
        if(*ALUresult==0)
            *Zero=1;
    }
    else if(ALUControl==0x2){ // less than
        if((signed)A < (signed)B)
            *ALUresult=1;
        else{
            *ALUresult=0;
            *Zero=1;
        }
    }
    else if(ALUControl==0x3){ //unsigned less than
        if(A<B)
            *ALUresult=1;
        else{
            *ALUresult=0;
            *Zero=1;
        }
    }
    else if(ALUControl==0x4){
        *ALUresult=A&B;//and
        if(*ALUresult==0)
            *Zero=1;
    }
    else if(ALUControl==0x5){
        *ALUresult=A|B;//or
        if(*ALUresult==0)
            *Zero=1;
    }
    else if(ALUControl==0x6){
        *ALUresult=B<<16;//shift B left by 16bits
        if(*ALUresult==0)
            *Zero=1;
    }
    else if(ALUControl==0x7){
        *ALUresult=A^B;//nor
        if(*ALUresult==0)
            *Zero=1;
    }
    
}

/* instruction fetch */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    if(*instruction>=0x00000000 && *instruction<=0xFFFFFFFF){
        if(PC%4 != 0)
            return 1;

        *instruction=Mem[PC>>2];
        return 0;

    }
    else
        return 1;
}


/* instruction partition */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    unsigned temp;
    *op = instruction >> 26; // instruction [31-26]

    temp = instruction << 6;
    *r1 = temp >> 27; // instruction [25-21]

    temp = instruction << 11;
    *r2 = temp >> 27; // instruction [20-16]

    temp = instruction << 16;
    *r3 = temp >> 27; // instruction [15-11]

    temp = instruction << 26;
    *funct = temp >> 26; // instruction [5-0]

    temp = instruction << 16;
    *offset = temp >> 16; // instruction [15-0]

    temp = instruction << 6;
    *jsec = temp >> 6; // instruction [25-0]
}


/* instruction decode */
int instruction_decode(unsigned op,struct_controls *controls)
{
    if(op==0x0){//R-format
        controls->RegWrite = 1;
        controls->RegDst = 1; // 1 means rd
        controls->MemRead = 2;
        controls->MemWrite = 0;
        controls->ALUSrc = 0;
        controls->Branch = 0;
        controls->Jump = 0;
        controls->MemtoReg = 0; //0 means from ALU to Reg
        controls->ALUOp = 7;
    }
    //I-type
    else if(op == 0x8){ // addi
        controls->RegWrite = 1;
        controls->RegDst = 0; // 0 means rt
        controls->MemRead = 2;
        controls->MemWrite = 0;
        controls->ALUSrc = 1;
        controls->Branch = 0;
        controls->Jump = 0;
        controls->MemtoReg = 0;
        controls->ALUOp = 0; //add
    }
    else if(op == 0xF){ // lui 001111
        controls->RegWrite = 1;
        controls->RegDst = 0;
        controls->MemRead = 0; // not read Mem
        controls->MemWrite = 0;
        controls->ALUSrc = 1;
        controls->Branch = 0;
        controls->Jump = 0;
        controls->MemtoReg = 0; //0 means from ALU to Reg
        controls->ALUOp = 6; // shift left 16 bits
    }
    else if(op == 0x23){ // lw
        controls->RegWrite = 1;
        controls->RegDst = 0;
        controls->MemRead = 1;
        controls->MemWrite = 0;
        controls->ALUSrc = 1;
        controls->Branch = 0;
        controls->Jump = 0;
        controls->MemtoReg = 1; //1 means from Mem to Reg
        controls->ALUOp = 0; //add
    }
    else if(op == 0x2B){ // sw
        controls->RegWrite = 0;
        controls->RegDst = 2;
        controls->MemRead = 0;
        controls->MemWrite = 1;
        controls->ALUSrc = 1;
        controls->Branch = 0;
        controls->Jump = 0;
        controls->MemtoReg = 2; //2 means don't care
        controls->ALUOp = 0; //add
    }
    else if(op == 0x4){ // beq
        controls->RegWrite = 0;
        controls->RegDst = 2;
        controls->MemRead = 0;
        controls->MemWrite = 0;
        controls->ALUSrc = 0;
        controls->Branch = 1;
        controls->Jump = 0;
        controls->MemtoReg = 2;
        controls->ALUOp = 1; //sub
    }
    else if(op == 0xA){ // slti 001010
        controls->RegWrite = 1;
        controls->RegDst = 0;
        controls->MemRead = 2;
        controls->MemWrite = 0;
        controls->ALUSrc = 1;
        controls->Branch = 0;
        controls->Jump = 0;
        controls->MemtoReg = 0;
        controls->ALUOp = 2; //010
    }
    else if(op == 0xB){ // sltiu 001011
        controls->RegWrite = 1;
        controls->RegDst = 0;
        controls->MemRead = 2;
        controls->MemWrite = 0;
        controls->ALUSrc = 1;
        controls->Branch = 0;
        controls->Jump = 0;
        controls->MemtoReg = 0;
        controls->ALUOp = 3; //010
    }
    else if(op == 0x2){ // jump
        controls->RegWrite = 2;
        controls->RegDst = 2;
        controls->MemRead = 2;
        controls->MemWrite = 0;//
        controls->ALUSrc = 2;
        controls->Branch = 2;
        controls->Jump = 1;
        controls->MemtoReg = 0;//
        controls->ALUOp = 2;
    }
    else return 1;    //invalid instruction

    return 0;

}


/* Read Register */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}


/* Sign Extend */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    unsigned temp = offset>>15;
    if(temp==1)
        *extended_value = offset | 0xFFFF0000;
    else
        *extended_value = offset & 0x0000FFFF;
}

/* ALU operations */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
    if(ALUSrc == 1) //I-type: no funct part!
        data2=extended_value;

    // Temp variable
    unsigned TempALUOp;
    
    if(ALUOp==7){
        // R-type
            // funct = 0x20 = 0010 0000 = 32
            if(funct==0x20)  //add
                TempALUOp=0x0;
                //ALU(data1,data2,0x0,ALUresult,Zero);
            else if (funct==0x22) // 10 0010 Sub
                TempALUOp=0x1;
            else if (funct==0x24) // 10 0100 and
                TempALUOp=0x4;
            else if (funct==0x25) // 10 0101 or
                TempALUOp=0x5;
            else if (funct==0x2A) // 10 1010 slt
                TempALUOp=0x2;
            else if (funct==0x2B) // 10 1011 sltu
                TempALUOp=0x3;
            else if (funct==0x27) // 10 0111 nor
                TempALUOp=0x7;
            else return 1;    //invalid funct
    }
    else
        TempALUOp=ALUOp;
    
    ALU(data1,data2,TempALUOp,ALUresult,Zero);
    
    return 0;
}

/* Read / Write Memory */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
    if(MemRead==1){
        //only halt when ALUresult is an address
        if((ALUresult%4) != 0)
            return 1;

        *memdata = Mem[ALUresult>>2];
    }
    else if(MemWrite==1){
        //only halt when ALUresult is an address
        if((ALUresult%4) != 0)
            return 1;

        Mem[ALUresult>>2] = data2;
    }
    return 0;
}


/* Write Register */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    if(RegWrite==1){
        if(MemtoReg==1){ //select from Mem
            if(RegDst==0)
                Reg[r2] = memdata; // rt
            else
                Reg[r3] = memdata; // rd
        }
        else{ //select from ALU
            if(RegDst==0)
                Reg[r2] = ALUresult;
            else
                Reg[r3] = ALUresult;
        }
    }

}


/* PC update */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    *PC+=4;

    if(Jump==1) //jump: (target PC) = (upper 4 bits of (PC + 4) : offset) << 2
        *PC = ((((*PC+4)>>27)<<26) + jsec) << 2;
    else if(Branch && Zero) //branch: if zero=1
        *PC += extended_value<<2;

}



