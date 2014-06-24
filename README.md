COT Project  
=================================================
If Convertion Pre Register Allocation
------------------------------------------------

###Author :###

    Andrea Mambretti (Mtr. 783286)

###Advisors:###

    Prof. Giovanni Agosta 

    Prof. Michele Scandale

###Abstract:###
The aim of this project is to provide an if conversion implementation on the machine code when it is still in a SSA form. 
Everything has been implemented in llvm/clang compiler that already has two If conversion passes. 
1. IfConversioni pass: is designed to work on the machine code after the register allocation for architectures that support instruction predication (ex ARM, Hexagon). 
2. EarlyIfConversion pass: is aimed to work before the register allocation but is designed to exploit if conversion optmization on architectures such as x86 that don't support predicated instruction. Instead
it uses select instructions to express the same capability.

To do so I needed to extend the concept of SSA and phi-SSA implementing inside llvm an extension of the SSA form called PSI-SSA. 


###Introduction:###
In 2001, A. Stoutchinin and  F. De Ferriere presented in [1] an improvement of the SSA form  called PSI-SSA. This new SSA form is based on the idea of the CSSA presented in [2] 
by J.Lee at all. 



[1] Efficient static single assignment form for predication.
[2] Concurrent single static assignment form and constant propagation for explicitely parallel programs. 
