COT Project  
=================================================
If Convertion Pre Register Allocation
------------------------------------------------

###Author :###

    Andrea Mambretti (Mtr. 783286) - Politecnico di Milano

###Advisors:###

    Prof. Giovanni Agosta - Politecnico di Milano

    Prof. Michele Scandale - Politecnico di Milano

###Abstract:###
The aim of this project is to provide an if conversion implementation on the machine code when it is still in a SSA form. 
Everything has been implemented in llvm/clang compiler that already has two If conversion passes. 

1. IfConversion pass: is designed to work on the machine code after the register allocation for architectures that support instruction predication (ex ARM, Hexagon). 

2. EarlyIfConversion pass: is aimed to work before the register allocation but is designed to exploit if conversion optmization on architectures such as x86 that don't support many predicated instruction. Instead
it uses cmov instructions to obtain the same capability.

To do so I needed to extend the concept of SSA and phi-SSA implementing inside llvm an extension of the SSA form called PSI-SSA. 


###Introduction:###
In 2001, A. Stoutchinin and  F. De Ferriere presented in [1] an improvement of the SSA form  called PSI-SSA. This new SSA form is based on the idea of the CSSA presented in [2] 
by J.Lee at all. In [2] they introduced pi-functions that collect the data flow information for shared variables referenced in different program threads. Instead, in the psi-form
this value is unique determined by the scheduling order of predicated assignments to a variable and the runtime values of predicates.

In 2004 A. Stoutchinin and G. Gao in [3] showed how an if-convertion optimization could be performed on programs in SSA-form. They exploit the duality between phi and psi nodes. In fact the meaning of a program doesn't change
if a phi node is converted in a psi node after the if-convertion.

In 2007 Psi-SSA form has been improved by F. De Ferriere in [4] extending the psi-function concept to work also with partial predicated instruction set. The only difference in the representation is
that when a psi operation is created as a replacemente for a phi operation some of its arguments may be defined by operations that cannot be predicated.

Basing on these previous work I implemented those concept inside llvm/clang compiler suite in such a way during the if-conversion on the program in SSA form all the phi nodes are converted into psi node. And afterwards all those new nodes are lowered depending the architecture we are dealing with. In the current implementation the lowering phase is implemented for the hexagon target.



[1] Efficient static single assignment form for predication.

[2] Concurrent single static assignment form and constant propagation for explicitely parallel programs. 

[3] If-Conversion in SSA form

[4] Improvements to the Psi-SSA representation.

###Implementation###
The whole work is based on two passes added into the machine code pass chain. The first is named IfConversionPreRegAllocation (location: $LLVM\_SRC/lib/CodeGen/IfConvertionPreRegAllocation.cpp) that uses most of the structures already present inside IfConverstion pass to detect possible places where apply the transformation. Futhermore it detects the phi instructions inside the program and convert them into psi instruction. This pass is inserted before the register allocation so the pass must maintains the SSA property on the code. 

The psi instruction used is a new istruction inserted into llvm. It is a generic instruction valid for every architecture and same as the phi instruction should be, later in the chain, lowered in the equivalent machine dependent set of instructions.

As mentioned above the lowering pass has been implemented only for the hexagon target. Hexagon is an architecture that supports predicated instructions designed by Qualcomm. 

So to lower the psi instructions has been added a new pass in the hexagon dependent pass chain (location: $LLVM\_SRC/lib/Target/Hexagon/HexagonPSIElimination.cpp) right after the PHIElimination pass and before the register allocation. In this pass all the psi instruction are converted into TFR instruction conveniently predicated with the information provided in the psi instruction. This second pass work on the machine code already out from the SSA form.

To Avoid interference with other targets both of the passes are by default disabled. To include them in the chain of passes two parameters from command line have to be specified.

They are:

1. -enable-if-convertion-pre-reg-allocation 
2. -enable-psi-elimination 

Example of usage:

    $LLVM_ROOT/bin/llc -debug -print-after-all -disable-if-converter -enable-if-convertion-pre-reg-allocation -enable-psi-elimination -march=hexagon $LLVM_TEST/triangle.bc

I also inserted an option to disable the previous pass of if convertion to avoid useless operations during the compilation.
  
1. -disable-if-converter

To obtain the phi nodes the bytecode should be passed through the mem2reg pass in the opt tool.

The following results are produced using the code in

    $LLVM_SRC/ifconv_test/diamond.c
    $LLVM_SRC/ifconv_test/triangle.c
    
Using the following commands I produced the bytecode in phi form and execute the two analysis analysis:

    $LLVM_ROOT/bin/clang -emit-llvm -c $FILEC -o $FILEBC
    $LLVM_ROOT/bin/opt -mem2reg $FILEBC -o $FILEBC2
    $LLVM_ROOT/bin/llc  -debug -print-after-all -disable-if-converter -enable-if-convertion-pre-reg-allocation -enable-psi-elimination -march=hexagon $FILEBC2

###Results###

####Case Triangle####

#####IR Code:#####

    ; Function Attrs: nounwind uwtable
    define i32 @main() #0 {
    entry:
      %rem = srem i32 3, 2
      %cmp = icmp eq i32 %rem, 0
      br i1 %cmp, label %if.then, label %if.else

    if.then:                                          ; preds = %entry
      %inc = add nsw i32 3, 1
      %inc1 = add nsw i32 %inc, 1
      br label %if.end

    if.else:                                          ; preds = %entry
      br label %if.end

    if.end:                                           ; preds = %if.else, %if.then
      %ciao.0 = phi i32 [ 3, %if.then ], [ -3, %if.else ]
      %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([3 x i8]* @.str, i32 0, i32 0), i32 %ciao.0)
      ret i32 0
    }


#####Before IfConvertionPreRegAllocation:#####


    BB#0: derived from LLVM BB %entry
            %vreg2<def> = TFRI 0; IntRegs:%vreg2
            %vreg1<def> = TFRI -3; IntRegs:%vreg1
            %vreg3<def> = TFR_PdRs %vreg2<kill>; PredRegs:%vreg3 IntRegs:%vreg2
            %vreg4<def> = NOT_p %vreg3; PredRegs:%vreg4,%vreg3
            %vreg5<def> = XOR_pp %vreg3, %vreg4<kill>; PredRegs:%vreg5,%vreg3,%vreg4
            JMP_t %vreg5<kill>, <BB#2>, %PC<imp-def,dead>; PredRegs:%vreg5
            JMP <BB#1>, %PC<imp-def,dead>
        Successors according to CFG: BB#1(12) BB#2(20)

    BB#1: derived from LLVM BB %if.then
        Predecessors according to CFG: BB#0
            %vreg6<def> = TFRI 3; IntRegs:%vreg6
        Successors according to CFG: BB#2

    BB#2: derived from LLVM BB %if.end
        Predecessors according to CFG: BB#0 BB#1
            %vreg0<def> = PHI %vreg1, <BB#0>, %vreg6, <BB#1>; IntRegs:%vreg0,%vreg1,%vreg6
            %vreg7<def> = COPY %R29; IntRegs:%vreg7
            STriw %vreg7, 0, %vreg0; mem:ST4[<unknown>] IntRegs:%vreg7,%vreg0
            %vreg8<def> = TFRI_V4 <ga:@.str>; IntRegs:%vreg8
            ADJCALLSTACKDOWN 4, %R29<imp-def,dead>, %R30<imp-def,dead>, %R31<imp-use>, %R30<imp-use>, %R29<imp-use>
            %R0<def> = COPY %vreg8; IntRegs:%vreg8
            CALLv3 <ga:@printf>, %D0<imp-def>, %R31<imp-def,dead>, %R0<imp-use>, ...
            ADJCALLSTACKUP 4, 0, %R29<imp-def,dead>, %R30<imp-def,dead>, %R31<imp-def,dead>, %R29<imp-use>
            %vreg10<def> = TFRI 0; IntRegs:%vreg10
            %R0<def> = COPY %vreg10; IntRegs:%vreg10
            JMPret %R31, %PC<imp-def,dead>, %R0<imp-use>

#####During IfConvertionPreRegAllocation:#####

    Ifcvt: function (0) 'main'
    Ifcvt (Triangle false): BB#0 (T:2,F:1) succeeded!

    Removing MBB: BB#1: derived from LLVM BB %if.then
        Successors according to CFG: BB#2

    Merging into block: BB#0: derived from LLVM BB %entry
            %vreg2<def> = TFRI 0; IntRegs:%vreg2
            %vreg1<def> = TFRI -3; IntRegs:%vreg1
            %vreg3<def> = TFR_PdRs %vreg2<kill>; PredRegs:%vreg3 IntRegs:%vreg2
            %vreg4<def> = NOT_p %vreg3; PredRegs:%vreg4,%vreg3
            %vreg5<def> = XOR_pp %vreg3, %vreg4<kill>; PredRegs:%vreg5,%vreg3,%vreg4
            %vreg6<def> = TFRI_cNotPt %vreg5, 3; IntRegs:%vreg6 PredRegs:%vreg5
        Successors according to CFG: BB#2(12)
    From MBB: BB#2: derived from LLVM BB %if.end
        Predecessors according to CFG: BB#0
            %vreg0<def> = PSI %vreg1, %vreg6, %vreg5<kill>, 0, %vreg5<kill>; IntRegs:%vreg0,%vreg1,%vreg6 PredRegs:%vreg5
            %vreg7<def> = COPY %R29; IntRegs:%vreg7
            STriw %vreg7, 0, %vreg0; mem:ST4[<unknown>] IntRegs:%vreg7,%vreg0
            %vreg8<def> = TFRI_V4 <ga:@.str>; IntRegs:%vreg8
            ADJCALLSTACKDOWN 4, %R29<imp-def,dead>, %R30<imp-def,dead>, %R31<imp-use>, %R30<imp-use>, %R29<imp-use>
            %R0<def> = COPY %vreg8; IntRegs:%vreg8
            CALLv3 <ga:@printf>, %D0<imp-def>, %R31<imp-def,dead>, %R0<imp-use>, ...
            ADJCALLSTACKUP 4, 0, %R29<imp-def,dead>, %R30<imp-def,dead>, %R31<imp-def,dead>, %R29<imp-use>
            %vreg10<def> = TFRI 0; IntRegs:%vreg10
            %R0<def> = COPY %vreg10; IntRegs:%vreg10
            JMPret %R31, %PC<imp-def,dead>, %R0<imp-use>

    Removing MBB: BB#2: derived from LLVM BB %if.end


#####After IfConvertionPreRegAllocation:#####

    # *** IR Dump After If Convertion on machine code before register allocation ***:
    # Machine code for function main: SSA

    BB#0: derived from LLVM BB %entry
        %vreg2<def> = TFRI 0; IntRegs:%vreg2
        %vreg1<def> = TFRI -3; IntRegs:%vreg1
        %vreg3<def> = TFR_PdRs %vreg2<kill>; PredRegs:%vreg3 IntRegs:%vreg2
        %vreg4<def> = NOT_p %vreg3; PredRegs:%vreg4,%vreg3
        %vreg5<def> = XOR_pp %vreg3, %vreg4<kill>; PredRegs:%vreg5,%vreg3,%vreg4
        %vreg6<def> = TFRI_cNotPt %vreg5, 3; IntRegs:%vreg6 PredRegs:%vreg5
        %vreg0<def> = PSI %vreg1, %vreg6, %vreg5<kill>, 0, %vreg5<kill>; IntRegs:%vreg0,%vreg1,%vreg6 PredRegs:%vreg5
        %vreg7<def> = COPY %R29; IntRegs:%vreg7
        STriw %vreg7, 0, %vreg0; mem:ST4[<unknown>] IntRegs:%vreg7,%vreg0
        %vreg8<def> = TFRI_V4 <ga:@.str>; IntRegs:%vreg8
        ADJCALLSTACKDOWN 4, %R29<imp-def,dead>, %R30<imp-def,dead>, %R31<imp-use>, %R30<imp-use>, %R29<imp-use>
        %R0<def> = COPY %vreg8; IntRegs:%vreg8
        CALLv3 <ga:@printf>, %D0<imp-def>, %R31<imp-def,dead>, %R0<imp-use>, ...
        ADJCALLSTACKUP 4, 0, %R29<imp-def,dead>, %R30<imp-def,dead>, %R31<imp-def,dead>, %R29<imp-use>
        %vreg10<def> = TFRI 0; IntRegs:%vreg10
        %R0<def> = COPY %vreg10; IntRegs:%vreg10
        JMPret %R31, %PC<imp-def,dead>, %R0<imp-use>

#####After PSIElimination:#####

    # *** IR Dump After Eliminate PSI nodes for register allocation ***:
    # Machine code for function main: Post SSA

    BB#0: derived from LLVM BB %entry
        %vreg2<def> = TFRI 0; IntRegs:%vreg2
        %vreg1<def> = TFRI -3; IntRegs:%vreg1
        %vreg3<def> = TFR_PdRs %vreg2<kill>; PredRegs:%vreg3 IntRegs:%vreg2
        %vreg4<def> = NOT_p %vreg3; PredRegs:%vreg4,%vreg3
        %vreg5<def> = XOR_pp %vreg3<kill>, %vreg4<kill>; PredRegs:%vreg5,%vreg3,%vreg4
        %vreg6<def> = TFRI_cNotPt %vreg5, 3; IntRegs:%vreg6 PredRegs:%vreg5
        %vreg0<def> = TFR_cNotPt %vreg5, %vreg6; IntRegs:%vreg0,%vreg6 PredRegs:%vreg5
        %vreg0<def> = TFR_cPt %vreg5, %vreg1; IntRegs:%vreg0,%vreg1 PredRegs:%vreg5
        %vreg7<def> = COPY %R29; IntRegs:%vreg7
        STriw %vreg7<kill>, 0, %vreg0<kill>; mem:ST4[<unknown>] IntRegs:%vreg7,%vreg0
        %vreg8<def> = TFRI_V4 <ga:@.str>; IntRegs:%vreg8
        ADJCALLSTACKDOWN 4, %R29<imp-def>, %R30<imp-def>, %R31<imp-use>, %R30<imp-use>, %R29<imp-use>
        %R0<def> = COPY %vreg8<kill>; IntRegs:%vreg8
        CALLv3 <ga:@printf>, %D0<imp-def,dead>, %R31<imp-def>, %R0<imp-use,kill>, ...
        ADJCALLSTACKUP 4, 0, %R29<imp-def>, %R30<imp-def>, %R31<imp-def>, %R29<imp-use>
        %vreg10<def> = TFRI 0; IntRegs:%vreg10
        %R0<def> = COPY %vreg10<kill>; IntRegs:%vreg10
        JMPret %R31, %PC<imp-def,dead>, %R0<imp-use,kill> 


####Case Diamond####

#####IR Code:#####

    ; Function Attrs: nounwind uwtable
    define i32 @main() #0 {
    entry:
      %rem = srem i32 3, 2
      %cmp = icmp eq i32 %rem, 0
      br i1 %cmp, label %if.then, label %if.else

    if.then:                                          ; preds = %entry
      %inc = add nsw i32 3, 1
      %inc1 = add nsw i32 %inc, 1
      br label %if.end

    if.else:                                          ; preds = %entry
      %inc2 = add nsw i32 3, 1
      %add = add nsw i32 -3, 1
      br label %if.end

    if.end:                                           ; preds = %if.else, %if.then
      %ciao.0 = phi i32 [ 3, %if.then ], [ %add, %if.else ]
      %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([3 x i8]* @.str, i32 0, i32 0), i32 %ciao.0)
      ret i32 0
    }

#####Before IfConvertionPreRegAllocation:#####

    BB#0: derived from LLVM BB %entry
            %vreg2<def> = TFRI 0; IntRegs:%vreg2
            %vreg3<def> = TFR_PdRs %vreg2<kill>; PredRegs:%vreg3 IntRegs:%vreg2
            %vreg4<def> = NOT_p %vreg3; PredRegs:%vreg4,%vreg3
            %vreg5<def> = XOR_pp %vreg3, %vreg4<kill>; PredRegs:%vreg5,%vreg3,%vreg4
            JMP_t %vreg5<kill>, <BB#2>, %PC<imp-def,dead>; PredRegs:%vreg5
            JMP <BB#1>, %PC<imp-def,dead>
        Successors according to CFG: BB#1(12) BB#2(20)

    BB#1: derived from LLVM BB %if.then
        Predecessors according to CFG: BB#0
            %vreg6<def> = TFRI 3; IntRegs:%vreg6
            JMP <BB#3>, %PC<imp-def,dead>
        Successors according to CFG: BB#3

    BB#2: derived from LLVM BB %if.else
        Predecessors according to CFG: BB#0
            %vreg0<def> = TFRI -2; IntRegs:%vreg0
        Successors according to CFG: BB#3

    BB#3: derived from LLVM BB %if.end
        Predecessors according to CFG: BB#2 BB#1
            %vreg1<def> = PHI %vreg0, <BB#2>, %vreg6, <BB#1>; IntRegs:%vreg1,%vreg0,%vreg6
            %vreg7<def> = COPY %R29; IntRegs:%vreg7
            STriw %vreg7, 0, %vreg1; mem:ST4[<unknown>] IntRegs:%vreg7,%vreg1
            %vreg8<def> = TFRI_V4 <ga:@.str>; IntRegs:%vreg8
            ADJCALLSTACKDOWN 4, %R29<imp-def,dead>, %R30<imp-def,dead>, %R31<imp-use>, %R30<imp-use>, %R29<imp-use>
            %R0<def> = COPY %vreg8; IntRegs:%vreg8
            CALLv3 <ga:@printf>, %D0<imp-def>, %R31<imp-def,dead>, %R0<imp-use>, ...
            ADJCALLSTACKUP 4, 0, %R29<imp-def,dead>, %R30<imp-def,dead>, %R31<imp-def,dead>, %R29<imp-use>
            %vreg10<def> = TFRI 0; IntRegs:%vreg10
            %R0<def> = COPY %vreg10; IntRegs:%vreg10
            JMPret %R31, %PC<imp-def,dead>, %R0<imp-use>

#####During IfConvertionPreRegAllocation:#####

    Ifcvt: function (0) 'main'
    Ifcvt (Diamond): BB#0 (T:2,F:1) succeeded!

    Removing MBB: BB#1: derived from LLVM BB %if.then
        Successors according to CFG: BB#2

    Removing MBB: BB#2: derived from LLVM BB %if.else
        Successors according to CFG: BB#3

    Removing MBB: BB#3: derived from LLVM BB %if.end


#####After IfConvertionPreRegAllocation:#####

    # *** IR Dump After If Convertion on machine code before register allocation ***:
    # Machine code for function main: SSA

    BB#0: derived from LLVM BB %entry
            %vreg2<def> = TFRI 0; IntRegs:%vreg2
            %vreg3<def> = TFR_PdRs %vreg2<kill>; PredRegs:%vreg3 IntRegs:%vreg2
            %vreg4<def> = NOT_p %vreg3; PredRegs:%vreg4,%vreg3
            %vreg5<def> = XOR_pp %vreg3, %vreg4<kill>; PredRegs:%vreg5,%vreg3,%vreg4
            %vreg0<def> = TFRI_cPt %vreg5, -2; IntRegs:%vreg0 PredRegs:%vreg5
            %vreg6<def> = TFRI_cNotPt %vreg5, 3; IntRegs:%vreg6 PredRegs:%vreg5
            %vreg1<def> = PSI %vreg0, %vreg6, %vreg5<kill>, 0, %vreg5<kill>; IntRegs:%vreg1,%vreg0,%vreg6 PredRegs:%vreg5
            %vreg7<def> = COPY %R29; IntRegs:%vreg7
            STriw %vreg7, 0, %vreg1; mem:ST4[<unknown>] IntRegs:%vreg7,%vreg1
            %vreg8<def> = TFRI_V4 <ga:@.str>; IntRegs:%vreg8
            ADJCALLSTACKDOWN 4, %R29<imp-def,dead>, %R30<imp-def,dead>, %R31<imp-use>, %R30<imp-use>, %R29<imp-use>
            %R0<def> = COPY %vreg8; IntRegs:%vreg8
            CALLv3 <ga:@printf>, %D0<imp-def>, %R31<imp-def,dead>, %R0<imp-use>, ...
            ADJCALLSTACKUP 4, 0, %R29<imp-def,dead>, %R30<imp-def,dead>, %R31<imp-def,dead>, %R29<imp-use>
            %vreg10<def> = TFRI 0; IntRegs:%vreg10
            %R0<def> = COPY %vreg10; IntRegs:%vreg10
            JMPret %R31, %PC<imp-def,dead>, %R0<imp-use>

#####After PSIElimination:#####

    # *** IR Dump After Eliminate PSI nodes for register allocation ***:
    # Machine code for function main: Post SSA

    BB#0: derived from LLVM BB %entry
            %vreg2<def> = TFRI 0; IntRegs:%vreg2
            %vreg3<def> = TFR_PdRs %vreg2<kill>; PredRegs:%vreg3 IntRegs:%vreg2
            %vreg4<def> = NOT_p %vreg3; PredRegs:%vreg4,%vreg3
            %vreg5<def> = XOR_pp %vreg3<kill>, %vreg4<kill>; PredRegs:%vreg5,%vreg3,%vreg4
            %vreg0<def> = TFRI_cPt %vreg5, -2; IntRegs:%vreg0 PredRegs:%vreg5
            %vreg6<def> = TFRI_cNotPt %vreg5, 3; IntRegs:%vreg6 PredRegs:%vreg5
            %vreg1<def> = TFR_cNotPt %vreg5, %vreg6; IntRegs:%vreg1,%vreg6 PredRegs:%vreg5
            %vreg1<def> = TFR_cPt %vreg5, %vreg0; IntRegs:%vreg1,%vreg0 PredRegs:%vreg5
            %vreg7<def> = COPY %R29; IntRegs:%vreg7
            STriw %vreg7<kill>, 0, %vreg1<kill>; mem:ST4[<unknown>] IntRegs:%vreg7,%vreg1
            %vreg8<def> = TFRI_V4 <ga:@.str>; IntRegs:%vreg8
            ADJCALLSTACKDOWN 4, %R29<imp-def>, %R30<imp-def>, %R31<imp-use>, %R30<imp-use>, %R29<imp-use>
            %R0<def> = COPY %vreg8<kill>; IntRegs:%vreg8
            CALLv3 <ga:@printf>, %D0<imp-def,dead>, %R31<imp-def>, %R0<imp-use,kill>, ...
            ADJCALLSTACKUP 4, 0, %R29<imp-def>, %R30<imp-def>, %R31<imp-def>, %R29<imp-use>
            %vreg10<def> = TFRI 0; IntRegs:%vreg10
            %R0<def> = COPY %vreg10<kill>; IntRegs:%vreg10
            JMPret %R31, %PC<imp-def,dead>, %R0<imp-use,kill>
