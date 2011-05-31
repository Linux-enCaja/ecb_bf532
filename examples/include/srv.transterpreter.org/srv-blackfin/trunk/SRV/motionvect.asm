/*******************************************************************************
Copyright(c) 2000 - 2002 Analog Devices. All Rights Reserved.
Developed by Joint Development Software Application Team, IPDC, Bangalore, India
for Blackfin DSPs  ( Micro Signal Architecture 1.0 specification).

By using this module you agree to the terms of the Analog Devices License
Agreement for DSP Software. 
********************************************************************************
Module Name     : init_tss.asm
Label Name      : __init_tss
Version         :   1.0
Change History  :

                Version     Date          Author            Comments
                1.0         04/12/2001    Vijay             Original 

Description     : This routine initializes the tss_struct structure, which is
                  used by the TSS motion vector estimation code. The 
                  initialization is done as follows:

                  tss_struct tss;
                  tss.hmv[9] = {0, -SS, 0, SS, -SS, SS, -SS, 0, SS};
                  tss.vmv[9] = {0, -SS, -SS, -SS, 0,0, SS, SS, SS};
                  tss.modifier[0] = 0;
                  for(i=1;i<9;i++)
                  {
                    tss.modifier[i]   = tss.vmv[i]*WINWIDTH+tss.hmv[i];
                    tss.modifier[i+8] = (tss.vmv[i]/2)*WINWIDTH+(tss.hmv[i]/2); 
                    tss.modifier[i+16] = (tss.vmv[i]/4)*WINWIDTH+(tss.hmv[i]/4);
                  }
                  where SS is the initial step size
                  WINWIDTH is the width of the reference window
                  The / operator represents an integer division

Prototype       : void _init_tss(tss_struct *ptr_tss, int WINWIDTH, int SS); 

Registers used  : R0-R3, I0-I3, M0, M1, L0-L3, P0, P1.

Performance     :
                Code size   : 192 Bytes.
                Cycle count : 154 Cycles.
*******************************************************************************/
.section L1_code;
.align 8;
.global __init_tss;
.extern _vhpel;
.extern _hhpel;

__init_tss:

    L0 = 0;
    L1 = 0;
    L2 = 0;
    L3 = 0;
    I2 = R0;
    I3 = R0;
    P1 = R0;
    I0.L = _vhpel;
    I0.H = _vhpel;
    I1.L = _hhpel;
    I1.H = _hhpel;
    M0 = 18;
    M1 = 16;
    P0 = 9;
    P1 += 36;

    LSETUP(ST_MOD_STEP1, END_MOD_STEP1) LC0 = P0;
    I3 += M0 || R0.L = W[I0++]; 
ST_MOD_STEP1:
        R0.L = R2.L*R0.L (IS) || R3.L = W[I1++];    
        R3.L = R2.L*R3.L (IS) || W[I2++] = R0.L;
                            // Store vmv[i] 
        R0.L = R1.L*R0.L (IS) || W[I3++] = R3.L;
                            // Store hmv[i] 
        R3.L = R0.L + R3.L (S) || R0.L = W[I0++];   
END_MOD_STEP1:
        W[P1++] = R3;       // Store modifier[i]
    P0 = 8;
    R2 = R2 >> 1 || I0 -= M0;
    I1 -= M1 || R0.L = W[I0++];

    LSETUP(ST_MOD_STEP2, END_MOD_STEP2) LC0 = P0;               
ST_MOD_STEP2:
        R0.L = R2.L*R0.L (IS) || R3.L = W[I1++];    
        R3.L = R2.L*R3.L (IS);
        R0.L = R1.L*R0.L (IS);
        R3.L = R0.L + R3.L (S) || R0.L = W[I0++];
END_MOD_STEP2:
        W[P1++] = R3;       // Store modifier[i]
    R2 = R2 >> 1 || I0 -= M0;
    I1 -= M1 || R0.L = W[I0++];

    LSETUP(ST_MOD_STEP3, END_MOD_STEP3) LC0 = P0;
ST_MOD_STEP3:
        R0.L = R2.L*R0.L (IS) || R3.L = W[I1++];    
        R3.L = R2.L*R3.L (IS);
        R0.L = R1.L*R0.L (IS);
        R3.L = R0.L + R3.L (S) || R0.L = W[I0++];   
END_MOD_STEP3:
        W[P1++] = R3;       // Store modifier[i]
    RTS;
__init_tss.end:

/*******************************************************************************
Module Name     : hpel.asm
Label Name      : __hpel
Version         :   1.0
Change History  :

                Version     Date          Author            Comments
                1.0         04/12/2001    Vijay             Original 

Description     : This routine does the half pixel computation for the motion 
                  estimation.

Assumption      : The routine assumes that WINWIDTH is a multiple of 4.

Prototype       : int _hpel(unsigned char *best_match, int min_SAD, 
                            unsigned char *target, int WINWIDTH);

                    best_match -> Address of the best matching reference block
                    min_SAD    -> The minimum SAD corresponding to the best 
                                  matching block.
                    target     -> Address of the target macro block (16x16)
                    WINWIDTH   -> Width of the reference window (WINWIDTH)

                    The output is the half pel positions which are returned to 
                    the calling routine

Registers used  : A0, A1, R0-R7, I0-I3, M0, M1, M3, L0-L3, P0, P2-P5, LC0.

Performance:
            Code size                       :  768 bytes
            Cycle count for half pixelation : 1695 cycles
*******************************************************************************/

// Half pixel computation

.section L1_code;
.align 8;
.global __hpel;

__hpel:

    [--SP] = (R7:4, P5:3);
    [--SP] = RETS;
    L0 = 0;
    L1 = 0;
    L2 = 0;
    L3 = 0;
    P5 = R0;                // Address of the best matching block
    R5 = R1;                // SAD corresponding to the best match
    P4 = R2;                // Address of the target block
    P3 = [SP + 44];         // Width of the reference window
    P0 = 344;
    SP -= P0;
    I3 = SP;                // Temporary buffer in stack
    R4 = 0;
/******************** INTERPOLATE DIAGONAL BLOCKS ********************/
    M0 = 7;
    M3 = -3 (X);
    R0 = P5;                // Address of the best match
    R0 += -1;
    I0 = R0;
    I1 = R0;
    R2 = P3;                // WINWIDTH
    R1 = R0 - R2 (S);
    I2 = R1;                // Address of best match - (WINWIDTH+1)
    R2 += -17;
    M1 = R2;
    P2 = 17;

    LSETUP(AVG4_ST, AVG4_END) LC0 = P2;
    DISALGNEXCPT || R0 = [I1++] || R2 = [I2++];
    DISALGNEXCPT || R1 = [I1++M3] || R3 = [I2++M3];
    R7 = BYTEOP2P(R1:0,R3:2) (RNDL) || R0 = [I1++M0];
AVG4_ST:    
        DISALGNEXCPT || I0 += M3 || R2 = [I2++M0];
        R6 = BYTEOP2P(R1:0,R3:2) (RNDH) || R0 = [I1++M3] || R2 = [I2++M3];
        R7 = R6 + R7 (NS) || I0 -= M3;
    
        R7 = BYTEOP2P(R1:0,R3:2) (RNDL, R) || [I3++] = R7 || R1 = [I1++M0];
        DISALGNEXCPT || I0 += M3 || R3 = [I2++M0];
        R6 = BYTEOP2P(R1:0,R3:2) (RNDH, R) || R1 = [I1++M3] || R3 = [I2++M3];
        R7 = R6 + R7 (NS) || I0 -= M3;
    
        R7 = BYTEOP2P(R1:0,R3:2) (RNDL) || [I3++] = R7 || R0 = [I1++M0];
        DISALGNEXCPT || I0 += M3 || R2 = [I2++M0];
        R6 = BYTEOP2P(R1:0,R3:2) (RNDH) || R0 = [I1++M3] || R2 = [I2++M3];
        R7 = R6 + R7 (NS) || I0 -= M3;
    
        R7 = BYTEOP2P(R1:0,R3:2) (RNDL, R) || [I3++] = R7 || R1 = [I1++M0];
        DISALGNEXCPT || I0 += M3 || R3 = [I2++M0];
        R6 = BYTEOP2P(R1:0,R3:2) (RNDH, R) || R1 = [I1++M3] || R3 = [I2++M3];
        R7 = R6 + R7 (NS) || I0 -= M3;
    
        R7 = BYTEOP2P(R1:0,R3:2) (RNDL) || [I3++] = R7 || R0 = [I1++M1];
        DISALGNEXCPT || I0 += M3 || R2 = [I2++M1];
        R6 = BYTEOP2P(R1:0,R3:2) (RNDH) || R0 = [I1++] || R2 = [I2++];
        R7 = R6 + R7 (NS) || I0 -= M3;
    
        DISALGNEXCPT || R1 = [I1++M3] || R3 = [I2++M3];
AVG4_END:
        R7 = BYTEOP2P(R1:0,R3:2) (RNDL) || [I3++] = R7 || R0 = [I1++M0];       
    
/**************** CALCULATE SAD FOR DIAGONAL BLOCKS *******************/
    R7 = -1;                // (V,H) -> R7.H = -1, R7.L = -1
    R0 = P4;
    R1 = SP;
    CALL _compute_sad;
    CC = R0 < R5;
    IF CC R5 = R0;
    IF CC R4 = R7;
    
    R7.L = 1;               // (V,H) -> R7.H = -1, R7.L = 1
    R0 = P4;
    R1 = SP;
    R1 += 1;
    CALL _compute_sad;
    CC = R0 < R5;
    IF CC R5 = R0;
    IF CC R4 = R7;
    
    R7.H = 1;               // (V,H) -> R7.H = 1, R7.L = 1
    R0 = P4;
    R1 = SP;
    R1 += 21;
    CALL _compute_sad;
    CC = R0 < R5;
    IF CC R5 = R0;
    IF CC R4 = R7;
    
    R7.L = -1;              // (V,H) -> R7.H = 1, R7.L = -1
    R0 = P4;
    R1 = SP;
    R1 += 20;
    CALL _compute_sad;
    CC = R0 < R5;
    IF CC R5 = R0;
    IF CC R4 = R7;
    
/******************** INTERPOLATE LEFT/RIGHT BLOCKS ********************/
    R0 = P5;                // Address of the best match
    I0 = R0;                // Address of best match
    R0 += -1;
    I1 = R0;                // Address of best match - 1
    I3 = SP;                // Output buffer
    P2 = 16 (Z);
    R1 = P3;                // WINWIDTH
    R1 += -20;
    M1 = R1;
    MNOP;

    LSETUP(AVG2_LR_ST, AVG2_LR_END) LC0 = P2;
    DISALGNEXCPT || R0 = [I0++] || R2 = [I1++];
AVG2_LR_ST:
        DISALGNEXCPT || R1 = [I0++] || R3  =[I1++]; 
        R6 = BYTEOP1P(R1:0,R3:2) || R0 = [I0++] || R2  =[I1++]; 
        R6 = BYTEOP1P(R1:0,R3:2)(R) || [I3++] = R6 || R1 = [I0++];
        DISALGNEXCPT  || [I3++] = R6 || R3  =[I1++];            
        R6 = BYTEOP1P(R1:0,R3:2) || R0 = [I0++] || R2  =[I1++];
        R6 = BYTEOP1P(R1:0,R3:2)(R) || [I3++] = R6 || R1 = [I0++M1]; 
        DISALGNEXCPT || [I3++] = R6 || R3  =[I1++M1];               
        R6 = BYTEOP1P(R1:0,R3:2) || R0 = [I0++];
AVG2_LR_END:
        DISALGNEXCPT || [I3++] = R6 || R2  =[I1++];
    
/**************** CALCULATE SAD FOR LEFT/RIGHT BLOCKS *******************/
    R7 = 1;                 // (V,H) -> R7.H = 0, R7.L = 1
    R0 = P4;
    R1 = SP;
    R1 += 1;
    CALL _compute_sad;
    CC = R0 < R5;
    IF CC R5 = R0;
    IF CC R4 = R7;
    
    R7.L = -1;              // (V,H) -> R7.H = 0, R7.L = -1
    R0 = P4;
    R1 = SP;
    CALL _compute_sad;
    CC = R0 < R5;
    IF CC R5 = R0;
    IF CC R4 = R7;
    
/******************** INTERPOLATE TOP/BOTTOM BLOCKS ********************/
    R1 = P3;                // WINWIDTH
    R0 = P5;                // Address of the best match
    I0 = R0;                // Address of best match
    R0 = R0 - R1(S) || NOP;
    I1 = R0;                // Address of best match - WINWIDTH
    I3 = SP;                // Output buffer
    P2 = 17;
    R1 += -16;
    M1 = R1;

    LSETUP(AVG2_TB_ST, AVG2_TB_END) LC0 = P2;
    DISALGNEXCPT || R0 = [I0++] || R2 = [I1++];
AVG2_TB_ST:
        DISALGNEXCPT || R1 = [I0++] || R3  =[I1++]; 
        R6 = BYTEOP1P(R1:0,R3:2) || R0 = [I0++] || R2  =[I1++]; 
        R6 = BYTEOP1P(R1:0,R3:2)(R) || [I3++] = R6 || R1 = [I0++];
        DISALGNEXCPT  || [I3++] = R6 || R3  =[I1++];            
        R6 = BYTEOP1P(R1:0,R3:2) || R0 = [I0++M1] || R2  =[I1++M1];
        R6 = BYTEOP1P(R1:0,R3:2)(R) || [I3++] = R6 || R0 = [I0++]; 
AVG2_TB_END:
        DISALGNEXCPT || [I3++] = R6 || R2  =[I1++];             
    
/**************** CALCULATE SAD FOR TOP/BOTTOM BLOCKS *******************/
    R7.H = -1;              // (V,H) -> R7.H = -1, R7.L = 0
    R7.L = 0;
    R0 = P4;
    R1 = SP;
    CALL _compute_sad_aligned;
    CC = R0 < R5;
    IF CC R5 = R0;
    IF CC R4 = R7;
    
    R7.H = 1;               // (V,H) -> R7.H = 1, R7.L = 0
    R0 = P4;
    R1 = SP;
    R1 += 16;
    CALL _compute_sad_aligned;
    CC = R0 < R5;
    IF CC R5 = R0;
    IF CC R4 = R7;
    
/********************************************************************/
    R0 = R4;                // Return horizontal and vertical half pel
    P0 = 344;
    SP = SP + P0;
    RETS = [SP++];
    (R7:4, P5:3) = [SP++];
    RTS;
    
    
.align 8;
_compute_sad:
    I0 = R0;                // Address of the target
    I1 = R1;                // Address of the interpolated block
    P0 = 16 (Z);
    A1 = A0 = 0;

    LSETUP (MAD_START1, MAD_END1) LC0=P0;
    DISALGNEXCPT || R0 = [I0++] || R2 = [I1++]; 
MAD_START1:
        DISALGNEXCPT || R3 = [I1++];
        SAA (R1:0,R3:2) || R1 = [I0++]  || R2 = [I1++];
                            // Compute absolute difference and acc 
        SAA (R1:0,R3:2) (R) || R0 = [I0++] || R3 = [I1++];
        SAA (R1:0,R3:2) || R1 = [I0++] || R2 = [I1++];
MAD_END1:
        SAA (R1:0,R3:2) (R) || R0 = [I0++] || R2 = [I1++];
    R3=A1.L+A1.H,R2=A0.L+A0.H;    
    R0 = R2 + R3 (S);       // Add the accumulated values in both MACs
    RTS;
    
.align 8;
_compute_sad_aligned:
    I0 = R0;                // Address of the target
    I1 = R1;                // Address of the interpolated block
    P0 = 16;

    LSETUP (MAD_START, MAD_END) LC0=P0;
    A1=A0=0 || R0 = [I0++] || R2 = [I1++];
                            // Initialize accumulators 
MAD_START:
        SAA (R1:0,R3:2) || R1 = [I0++]  || R3 = [I1++];
                            // Compute absolute difference and acc. 
        SAA (R1:0,R3:2) (R) || R0 = [I0++] || R2 = [I1++];
        SAA (R1:0,R3:2) || R1 = [I0++] || R3 = [I1++];
MAD_END:SAA (R1:0,R3:2) (R) || R0 = [I0++] || R2 = [I1++];
    R3=A1.L+A1.H,R2=A0.L+A0.H;    
    R0 = R2 + R3 (S);       // Add the accumulated values in both MACs
    RTS;
__hpel.end:

/*******************************************************************************
Module Name     : three_step_search.asm
Label Name      : __three_step_search
Version         :   2.0
Change History  :

                Version     Date          Author        Comments
                2.0        01/09/2007    Arjun            Tested with VDSP++4.5
                            Compiler 7.2.3.2
        1.2         11/18/2002    Swarnalatha   Tested with VDSP++ 3.0
                                                        compiler 6.2.2 on 
                                                        ADSP-21535 Rev.0.2
                1.1         11/13/2002    Swarnalatha   Tested with VDSP++ 3.0
                                                        on ADSP-21535 Rev.0.2
                1.0         07/02/2001    Vijay         Original 

Description     : This routine does the motion vector computation using the 
                  Three step search algorithm for a given macroblock. The 
                  integer pel motion vector is first computed and a half pixel 
                  correction is given. The range covered by different step sizes
                  is shown below :

                  =====================================
                  |   Initial       |                 |
                  | step size (SS)  |     Range (SR)  |
                  =====================================
                  |     4           |  -7.5 to  7.5   |
                  |     5           |  -8.5 to  8.5   |
                  |     6           | -10.5 to 10.5   |
                  |     7           | -11.5 to 11.5   |
                  |     8           | -14.5 to 14.5   |
                  |     9           | -15.5 to 15.5   |
                  =====================================

                  The input and output to this routine is through a structure. 
                  The motion vectors are written back to the same structure.
                  The input/output structure is declared as follows :

                  typedef struct
                  {
                    unsigned char *ptr_target;
                                        // Target block address (16x16) 
                    unsigned char *ptr_reference;
                                        // Reference window address 
                    int winwidth;       // Width of the reference window
                    int step_size;      // Initial step size
                    tss_struct *ptr_tss;// Pointer to the initialized tss_struct
                    short mv_x;         // Address of the horizontal MV
                    short mv_y;         // Address of the vertical MV
                  }tss_par;

                  where tss_struct is defined as
                  typedef struct
                  {
                    short vmv[9];
                    short hmv[9];
                    short modifier[25];
                  }tss_struct;

                  The tss_struct is initialized by calling the function
                  __init_tss() once before invoking the TSS routine.
                  This structure is used by the TSS routine in the motion vector
                  computation (Refer init_tss.asm for initialization details).

                  The target block is assumed to be stored in a 16x16 buffer and
                  the pointer to this buffer is initialized in the tss_par 
                  structure.

                  The reference block stores all the required data for covering
                  the range of the step size and for doing the half pixel 
                  interpolation, that is, if the initial step size is SS, then 
                  the range covered is SR = (SS + (SS>>1) + (SS>>2)). In 
                  addition to this we need one more row/column for half pixel 
                  interpolation.
                  Thus, in the reference window we have to store a stretch of 
                  {2*(SR + 1) + 16} pixels around the target block. The width of
                  the reference window becomes WINWIDTH = {2*(SR+1) + 16}. The 
                  following picture depicts the data storage in the reference 
                  window.

                  Data layout of the reference window

                            <-------  WINWIDTH  ---------->  
                            -------------------------------  ---
                            | --------------------------- |   |
                            | |           |             | |   |
                 1 pel gap->| |<-        SR             | |   |
                  for half  | |           |             | |   W
                  pel inter.| |       -----------       | |   I
                            | |      |  TARGET  |       | |   N
                            | |      |  (ZERO   |       | |   W
                            | |< SR >|  MOTION  |       | |   I
                            | |      | POSITION)|       | |   D
                            | |      |          |       | |   T
                            | |       -----------       | |   H
                            | |      <--- 16 --->       | |   |
                            | |                         | |   |
                            | --------------------------- |   |
                            ------------------------------  ---

Assumption      : The width of the reference window is assumed tobe a multiple 
                  of 4.

Prototype       : void three_step_search(tss_par *tss_in_out);

Registers used  : A0, A1, R0-R7, I0-I2, M0, M1, L0-L2, P0-P5, LC0, LC1.

Performance     :
        Code size :
            three_step_search :    688 bytes
            hpel              :    768 bytes
            init_tss          :    192 bytes

        Total cycle count       : 4239 cycles
        Cycle count split up :
        Integer pel estimation  :   2544 cycles
        Half pel estimation     :   1695 cycles

    The cycle count given above is for the first iteration of the test case 1 in
    test_three_step_search.c 
*******************************************************************************/
#define PTR_MACROBLOCK 0
#define PTR_REFERENCE  4
#define WINWIDTH       8
#define STEP_SIZE     12
#define PTR_TSS       16
#define H_MV          20
#define V_MV          22

.section L1_code;
.align 8;
.global _three_step_search;

.extern __hpel;
    
_three_step_search:
    
    [--SP] = (R7:4, P5:3);
    P5 = R0;                // Address of tss parameter structure
    [--SP] = RETS;
    SP += -36;
    L0 = 0;
    L1 = 0;
    L2 = 0;

    I2 = SP;                // I2 points to mv_ind array
    R7 = R7 - R7 (S) || R0 = [P5 + STEP_SIZE];
                            // Initial step size 
    R1 = R0 >>> 1 || R3 = [P5 + WINWIDTH];
                            // WINWIDTH 
    R2 = R0 >>> 2 || P4 = [P5 + PTR_TSS];
                            // Address of tss structure 
    R0 = R0 + R1(S);
    R0 = R0 + R2 (S) || R1 = [P5 + PTR_MACROBLOCK] || [I2++] = R7;
                            // Fetch the address of the target block 
    R0 += 1;                // Search Range (SR)
    I0 = R1;
    R1 = R0.L*R3.L (IS) || R2 = [P5 + PTR_REFERENCE];
                            // (SR*(WINWIDTH+1)),Address of reference window 
    R1 = R0 + R1 (NS) || [I2++] = R7;
                            // [SP] = mv_ind[0] 
    R1 = R2 + R1 (NS) || [I2--] = R7;
                            // ref_ptr = reference + SR(WINWIDTH + 1),
                            // [SP + 8] = mv_ind[2] 
    I1 = R1;                // Address of reference data
    R4 = R4 - R4 (S) || [SP + 20] = R1;
                            // [SP + 20] = ref_ptr 
    P4 += 36;
    A1 = A0 = 0 || [SP + 24] = P4;
                            // [SP + 24] = Address of modifier[0] 
    P4 += 2;
    M0 = -260 (X);
    R3 = [P5 + WINWIDTH];
    R3 += -16;
    M1 = R3;                // Modifier for the reference window
    P2 = 16 (Z);
    [SP + 28] = P2;         // To retrieve the loop count if it is modified
/******************** ZERO MOTION VECTOR POSITION *****************************/
    DISALGNEXCPT || R0 = [I0++] || R2 = [I1++];
                            // Fetch the first data from the two blocks 

    LSETUP (MAD_START2, MAD_END2) LC0=P2;
MAD_START2:
        DISALGNEXCPT || R3 = [I1++];
        SAA (R1:0,R3:2) || R1 = [I0++]  || R2 = [I1++];
                            // Compute absolute difference and acc 
        SAA (R1:0,R3:2) (R) || R0 = [I0++] || R3 = [I1++];  
        SAA (R1:0,R3:2) || R1 = [I0++] || R2 = [I1 ++ M1];  
MAD_END2:SAA (R1:0,R3:2) (R) || R0 = [I0++] || R2 = [I1++];  
                            // Dummy fetch using I0, modifier[k++]
    R3=A1.L+A1.H,R2=A0.L+A0.H || R0 = [I0 ++ M0] || R1 = W[P4++] (X);     
    R5 = R2 + R3 (S) || R0 = [SP + 20];
                            // Add the accumulated values in both MACs 
    R1 = R0 + R1 (S) || I2 -= 4;
    I1 = R1;    
/************************ THREE STEP SEARCH **********************************/
STEP_LOOP:  
    P3 = 2;

    LSETUP (ST_SEARCH, END_SEARCH) LC1=P2>>1;
ST_SEARCH:
        A1 = A0 = 0;

        LSETUP (MAD_START3, MAD_END3) LC0=P2;
        DISALGNEXCPT || R0 = [I0++] || R2 = [I1++]; 
MAD_START3: DISALGNEXCPT || R3 = [I1++];
            SAA (R1:0,R3:2) || R1 = [I0++]  || R2 = [I1++];
                            // Compute absolute difference and acc 
            SAA (R1:0,R3:2) (R) || R0 = [I0++] || R3 = [I1++];
            SAA (R1:0,R3:2) || R1 = [I0++] || R2 = [I1 ++ M1];
MAD_END3:   SAA (R1:0,R3:2) (R) || R0 = [I0++] || R2 = [I1++];
                            // Dummy fetch using I0, modifier[k++]
        R3=A1.L+A1.H,R2=A0.L+A0.H || R0 = [I0 ++ M0] || R1 = W[P4++] (X);   
        R6 = R2 + R3 (S) || R0 = [SP + 20];
                            // Add the accumulated values in both MACs 
        R1 = R0 + R1;
        I1 = R1;
        CC = R6 == R5;
        IF !CC JUMP LESS (BP);
        R3 = P3;
        R0 = [P5 + PTR_TSS];// Address of vmv[]
        R1 = 18;
        R1 = R1 + R0 (S) || [I2] = R4;
                            // Address of hmv[] 
        R2 = SP;            // Address of mv_ind[]
        R3 = R3 >> 1 || [SP + 16] = R7;
                            // step number (step) 
        [SP + 12] = R3;     // Current index (c)
        CALL __compute_distance;
        R4 = R0 << 0 || P2 = [SP + 28];
                            // Store the returned index, Restore loop count 
LESS:   CC = R6 < R5;
        IF CC R5 = R6;
        IF CC R4 = P3;
END_SEARCH:
        P3 += 2;            // Index increment
                            // ref_ptr update
    P4 += -2;
    R1 = R7 << 4 || R0 = [SP + 24];
    R1 = R1 + R4 (NS) || [I2++] = R4;
                            // store mv_ind[j] 
    R0 = R0 + R1 (NS) || R3 = W[P4++] (X);
    P0 = R0;
    R2 = R2 - R2 (S) || R1 = [SP + 20];
    R7 += 1;
    CC = R4 == 0;
    R4 = R4 - R4 (S) || R0 = W[P0] (X);
    IF CC R0 = R2;
    R0 = R0 + R1;
    R1 = R0 + R3(NS) || [SP + 20] = R0;
                            // Updated ref_ptr 
    I1 = R1;
    CC = R7 == 3;
    IF !CC JUMP STEP_LOOP (BP);
/********************** MOTION VECTOR COMPUATION **************************/
    P3 = [SP + 8];
    P0 = [P5 + PTR_TSS];    // Address of vmv
    P4 = [P5 + PTR_TSS];
    P4 += 18;               // Address of hmv
    
    P1 = P0 + P3;
    R0.H = W[P1];           // vmv[mv_ind[2]]
    P3 = P4 + P3;
    R0.L = W[P3];           // hmv[mv_ind[2]]
    
    R1 = R0 >> 14 (V) || P3 = [SP + 4];
    P1 = P0 + P3;
    P3 = P4 + P3;
    R0 = R0 +|+ R1 || R2.H = W[P1];
                            // vmv[mv_ind[1]] 
    R3 = R0 >>> 2 (V) || R2.L = W[P3];
                            // hmv[mv_ind[1]] 
    
    R1 = R2 >> 15 (V) || P3 = [SP];
                            // mv_ind[0] 
    P1 = P0 + P3;
    P3 = P4 + P3;
    R2 = R2 +|+ R1 || R0.H = W[P1];
                            // vmv[mv_ind[0]] 
    R2 = R2 >>> 1 (V) || R0.L = W[P3];
                            // hmv[mv_ind[0]] 
    R3 = R3 +|+ R2;
                            // R3.H -> HMV, R3.L -> VMV (scaled)
    R3 = R3 +|+ R0, R4 = R3 -|- R0 (ASL) || R0 = [SP + 20];
                            // R0->Best matching block 
/************************* HALF PIXEL CCOMPUTATION *************************/
    R3 = R3 >> 16 || W[P5 + H_MV] = R3;
                            // Store Horizontal MV 
    W[P5 + V_MV] = R3;      // Store Vertical MV
    R1 = R5;                // SAD corresponding to the best match
    R2 = [P5 + PTR_MACROBLOCK];
                            // Address of the target macroblock 
    R3 = [P5 + WINWIDTH];
    [SP + 12] = R3;
    CALL __hpel;            // __hpel is in hpel.asm
    
/***************************************************************************/
    R1 = W[P5 + H_MV](Z);      // Get horizontal integer pel MV
    R2 = W[P5 + V_MV](Z);      // Get vertical integer pel MV
    R1.L = R1.L + R0.L (S); // Compute half pel horizontal MV
    R2.L = R2.L + R0.H (S); // Compute half pel vertical MV
    W[P5 + H_MV] = R1;
    W[P5 + V_MV] = R2;
    SP += 36;
    RETS = [SP++];
    (R7:4, P5:3) = [SP++];
    RTS;
_three_step_search.end:    
/* If the SAD of two check points are same this routine computes the
motion vector of the two check points and calculates the distance of
each from the target block. Whichever checkpoint is close to the 
target block that index is returned 
*/
    
.global __compute_distance;
.align 8;
__compute_distance:
    P2 = R2;                // Address of mv_ind[];
    P0 = R0;                // Address of vmv[]
    [--SP] = (R7:6, P5:4);
    R0 = R0 - R0 (S) || R3 = [SP + 32];
                            // step 
    R6 = R6 - R6 (S) || P4 = [P2++];            
    P1 = R1;                // Address of hmv[]
    R2 = -16;
    R3 = -R3;
LP_START:
    P5 = P0 + P4;
    R1.L = W[P5];
    P5 = P1 + P4;
    R7 = R2 - R0(S) || R1.H = W[P5];
    R7 = LSHIFT R1 BY R7.L (V) || P4 = [P2++];
    R1 = R1 +|+ R7;
    R7 = ASHIFT R1 BY R0.L (V);
    R6 = R6 +|+ R7;
    R0 += -1;
    CC = R3 <= R0;
    IF CC JUMP LP_START;
    
    R7 = R6 -|- R7 || P4 = [SP + 28];
    A1 = R6.L * R6.L (IS) || R0 = [SP + 28];
                            // R0 -> c (current index) 
    A1 += R6.H*R6.H (IS);
    P5 = P0 + (P4<<1);
    R0 = R0 << 1 || R1.L = W[P5];
    P5 = P1 + (P4<<1);
    R6 = R2 - R3(S) || R1.H = W[P5];
    R6 = LSHIFT R1 BY R6.L (V) || P4 = [P2--];
    R1 = R1 +|+ R6;
    R6 = ASHIFT R1 BY R3.L (V) || P4 = [P2--];
    R6 = R6 +|+ R7;
    A0 = R6.L * R6.L (IS);
    A0 += R6.H * R6.H (IS) || R1 = [P2];
                            // R1 -> p[step] 
                            // A0 -> Current MV distance, 
                            // A1 -> Previous MV distance
                            // R0 -> Current MV index, R1 -> Previous MV index
    CC = A0 < A1;
    IF !CC R0 = R1;
    (R7:6, P5:4) = [SP++];
    RTS;
__compute_distance.end:    

