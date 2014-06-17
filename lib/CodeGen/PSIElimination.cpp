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
#include <algorithm>
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
    return;
}
