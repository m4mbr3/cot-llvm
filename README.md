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
