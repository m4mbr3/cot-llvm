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

In 2004 A. Stoutchinin and G. Gao in [3] showed how an if-convertion optimization could be performed on programs in SSA-form. The exploit the duality between phi and psi nodes. In fact the meaning of a program doesn't change
if a phi node is converted in a psi node after the if-convertion.

In 2007 Psi-SSA form has been improved by F. De Ferriere in [4] extending the psi-function concept to work also with partial predicated instruction set. The only difference in the representation is
that when a psi operation is created as a replacemente for a phi operation some of its arguments may be defined by operations that cannot be predicated.

Basing on these previous work I implemented those concept inside llvm/clang compiler suite in such a way during the if-conversion on the program in SSA form all the phi nodes are converted into psi node. And afterwards all those new nodes are lowered depending the architecture we are dealing with. In the current implementation the lowering phase is implemented for the hexagon targer.



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

    $LLVM_ROOT/bin/llc -debug -print-after-all -disable-if-converter -enable-if-convertion-pre-reg-allocation -enable-psi-elimination -march=hexagon $LLVM_TEST/triangle.bc2
  
