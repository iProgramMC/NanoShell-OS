/*
  The younger brother to the NanoShell Interpreter, implemented it because
  I was too lazy to implement a proper compiler. 
  It's called Simple Language, however, the name is too lame, so what if I
  change it to NanoShell Lite Assembly?
  
  https://hackaday.io/project/25583-simple-compiler
*/

/* CPU MODEL */
// 16 bit data bus and address bus
// int Ax;             // Primary registeer
// int Bx;             // Secondary register
// int Fx;             // Flag register
int LiteInt_DP=49152;          // Moves up (DP=DP+1)
int LiteInt_SP=65504;          // Moves down (SP=SP-1)
int LiteInt_Memory[65536];

/* Interpreter Constants */
const int _movAxVar=  1;
const int _movAxImm=  2;
const int _movAxBx =  3;
const int _movBxAx =  4;
const int _movVarAx=  5;
const int _pushAx  =  6;
const int _pushBx  =  7;
const int _pushFx  =  8;
const int _popAx   =  9;
const int _popBx   = 10;
const int _popFx   = 11;
const int _addBx   = 12;
const int _subBx   = 13;
const int _mulBx   = 14;
const int _divBx   = 15;
const int _setEq   = 16;
const int _setNE   = 17;
const int _setLT   = 18;
const int _setLE   = 19;
const int _setGT   = 20;
const int _setGE   = 21;
const int _jmp     = 22;
const int _jz      = 23;
const int _orAxAx  = 24;
const int _xorAxAx = 25;
const int _cmpBx   = 26;
const int _wrtAx   = 27;
const int _wrtLn   = 28;
const int _rdAx    = 29;
const int _halt    =255;

/* CPU Functions */
const int LiteInt_ZF=0x40;
const int LiteInt_SF=0x80;
int LiteInt_F=0x40;       // ZF set

void SetFlag(int *Flags, int FlagMask)
{
  *Flags=(*Flags)|FlagMask;
}

bool FlagIsSet(int Flags, int FlagMask)
{
  return ((Flags&FlagMask)!=0);
}

void Test(int LiteInt_Ax)
{
  LiteInt_F=0;
  if (LiteInt_Ax<0) {
    SetFlag(&LiteInt_F, LiteInt_SF);
  } else if (LiteInt_Ax==0) {
    SetFlag(&LiteInt_F, LiteInt_ZF);
  }
}

void LiteInt_push(int LiteInt_Ax)
{
  LiteInt_Memory[LiteInt_SP--] = LiteInt_Ax;
}

int LiteInt_pop(void)
{
  return LiteInt_Memory[LiteInt_SP++];
}

bool LiteInt_Interpret(int *IP)
{
  static int Ax=0;
  static int Bx=0;
  bool running=true;
  int operand;
  int IR;

  //Fetch Instruction
  IR=LiteInt_Memory[*IP];
  *IP=*IP+1;
  if (IR==_movAxVar) {
    // Fetch Address
    operand=LiteInt_Memory[*IP];
    *IP=*IP+1;
    Ax=LiteInt_Memory[operand];
  } else if (IR==_movVarAx) {
    // Fetch Address
    operand=LiteInt_Memory[*IP];
    *IP=*IP+1;
    LiteInt_Memory[operand]=Ax;
  } else if (IR==_movAxImm) {
    // Fetch Address
    operand=LiteInt_Memory[*IP]; 
    *IP=*IP+1;
    Ax=operand;
  } else if (IR==_movAxBx) {
    Ax=Bx;
  } else if (IR==_movBxAx) {
    Bx=Ax;
  } else if (IR==_pushAx) {
    LiteInt_push(Ax);
  } else if (IR==_popAx) {
    Ax=LiteInt_pop();
  } else if (IR==_popBx) {
    Bx=LiteInt_pop();
  } else if (IR==_addBx) {
    Ax=Ax+Bx;
  } else if (IR==_subBx) {
    Ax=Ax-Bx;
  } else if (IR==_mulBx) {
    Ax=Ax*Bx;
  } else if (IR==_divBx) {
    Ax=Ax/Bx;
  } else if (IR==_jmp) {
    // Fetch Address
    operand=LiteInt_Memory[*IP];
    *IP=operand;
  } else if (IR==_jz) {
    if (FlagIsSet(LiteInt_F, LiteInt_ZF)) {
      // Fetch Address
	  operand=LiteInt_Memory[*IP];
      *IP=operand;
    } else {
      *IP=*IP+1;
    }
  } else if (IR==_orAxAx) {
    Test(Ax);
  } else if (IR==_xorAxAx) {
    Ax=0;
    Test(Ax);
  } else if (IR==_cmpBx) {
    Test(Ax-Bx);
  } else if (IR==_setEq) {
    Ax=0;
    if (FlagIsSet(LiteInt_F, LiteInt_ZF)) Ax=1;
  } else if (IR==_setNE) {
    Ax=0;
    if (!FlagIsSet(LiteInt_F, LiteInt_ZF)) Ax=1;
  } else if (IR==_setLT) {
    Ax=0;
    if (FlagIsSet(LiteInt_F, LiteInt_SF)) Ax=1;
  } else if (IR==_setLE) {
    Ax=0;
    if (FlagIsSet(LiteInt_F, LiteInt_SF)||FlagIsSet(LiteInt_F, LiteInt_ZF)) Ax=1;
  } else if (IR==_setGT) {
    Ax=1;
    if (FlagIsSet(LiteInt_F, LiteInt_SF)||FlagIsSet(LiteInt_F, LiteInt_ZF)) Ax=0;
  } else if (IR==_setGE) {
    Ax=1;
    if (FlagIsSet(LiteInt_F, LiteInt_SF)) Ax=0;
  } else if (IR==_wrtAx) {
    printf("%d ",Ax);
  } else if (IR==_wrtLn) {
    printf("\n");
  } else if (IR==_rdAx) {
    // To do!
  } else if (IR==_halt) {
    running=false;
  } else {
    // Error
    printf("Error: Unknown instruction detected, this may cause problems... %d\n",IR);
  }
  return running;
}

void LiteInt_SimpleInt(uint8_t* ExecutableLocation, uint16_t Size)
{
  int IP=0;

  fast_memcpy(LiteInt_Memory, ExecutableLocation, Size);
  
  while (LiteInt_Interpret(&IP));
}
