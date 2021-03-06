#define DEBUG_TYPE "phielim"
#include "llvm/CodeGen/Passes.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LiveIntervalAnalysis.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "Hexagon.h"
#include <algorithm>
#include <vector>
using namespace llvm;

namespace {
    class PSIElimination : public MachineFunctionPass {
        const TargetInstrInfo *TII;

        public:
            static char ID;
            PSIElimination() : MachineFunctionPass(ID) {
                initializePHIEliminationPass (*PassRegistry::getPassRegistry());
            }

            virtual bool runOnMachineFunction (MachineFunction &MF);
            virtual void getAnalysisUsage(AnalysisUsage &AU) const;

        private:
            void LowerPSINode(MachineBasicBlock &MBB); 
    };
}
char PSIElimination::ID=0;
char& llvm::PSIEliminationID = PSIElimination::ID;

INITIALIZE_PASS_BEGIN(PSIElimination, "psi-node-elimination",
                            "Eliminate PSI nodes  for register allocation",
                            false, false)
INITIALIZE_PASS_DEPENDENCY(PHIElimination)
INITIALIZE_PASS_DEPENDENCY(TwoAddressInstructionPass)
INITIALIZE_PASS_END(PSIElimination, "psi-node-elimination", 
                            "Eliminate PSI nodes for register allocation",
                            false,false)
void PSIElimination::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addPreserved<LiveVariables>();
    MachineFunctionPass::getAnalysisUsage(AU);
}

bool PSIElimination::runOnMachineFunction(MachineFunction &MF) {
    TII = MF.getTarget().getInstrInfo();
    if (!TII) return false;
    for (MachineFunction::iterator I = MF.begin(),
         E = MF.end();
         I != E; 
         ++I) {
        MachineBasicBlock &MBB = *I;
        LowerPSINode(MBB);       
    }
    return true;
}

void PSIElimination::LowerPSINode(MachineBasicBlock &MBB) {
    for (MachineBasicBlock::iterator I = MBB.begin(), 
         E = MBB.end(); 
         I != E;
         ++I) {
        MachineInstr *ins = I;
        if ( ins->getOpcode() == TargetOpcode::PSI ) {
            unsigned DestReg = ins->getOperand(0).getReg();
            unsigned firstReg = ins->getOperand(1).getReg();
            unsigned secondReg = ins->getOperand(2).getReg();
            int i = 3;
            SmallVector<MachineOperand, 4> Cond_1;
            SmallVector<MachineOperand, 4> Cond_2;
            if (ins->getOperand(i).isImm() && ins->getOperand(i).getImm() == 0) { 
                dbgs() << "Saving operand1: " << ins->getOperand(i).getImm() << "\n";
                Cond_1.push_back(ins->getOperand(i));
                i++;
            }
            Cond_1.push_back(ins->getOperand(i));
            dbgs() << "Size Cond1: "<< Cond_1.size() << "\n";
            i++;
            if (ins->getOperand(i).isImm() && ins->getOperand(i).getImm() == 0) {
                dbgs() << "Saving operand2: " << ins->getOperand(i).getImm() << "\n";
                Cond_2.push_back(ins->getOperand(i)); 
                i++;
            }
            Cond_2.push_back(ins->getOperand(i));
            dbgs() << "Size Cond2: "<< Cond_2.size() << "\n";
            MachineBasicBlock::iterator AfterPSIsIt = next(I);
            MachineInstr *MPhi = MBB.remove(I);
            I = AfterPSIsIt;
            BuildMI(MBB, AfterPSIsIt, MPhi->getDebugLoc(), TII->get(Hexagon::TFR), DestReg).addReg(firstReg);
            TII->PredicateInstruction(--AfterPSIsIt, Cond_1);
            BuildMI(MBB, AfterPSIsIt, MPhi->getDebugLoc(), TII->get(Hexagon::TFR), DestReg).addReg(secondReg);
            TII->PredicateInstruction(--AfterPSIsIt, Cond_2);
        }
    }
    return;
}
