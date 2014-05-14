//===-- LateIfConversion.cpp - Machine code if conversion pass. ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the machine instruction level if-conversion pass on the
// machine code after the registry allocation.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "ifcvtmc"
#include "llvm/CodeGen/Passes.h"
#include "BranchFolding.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineBranchProbabilityInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetSchedule.h"
#include "llvm/MC/MCInstrItineraries.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include <iostream>

using namespace llvm;

namespace {
    class IfConvertionPreRegAllocation : public MachineFunctionPass {
public:
            static char ID;
            IfConvertionPreRegAllocation() : MachineFunctionPass(ID) {}
            //virtual void getAnalysisUsage(AnalysisUsage &AU) const; 
            virtual bool runOnMachineFunction(MachineFunction &MF);
    };
}


bool IfConvertionPreRegAllocation::runOnMachineFunction(MachineFunction &MF) {
    return false;
}

char IfConvertionPreRegAllocation::ID = 0;
char &llvm::IfConvertionPreRegAllocationID = IfConvertionPreRegAllocation::ID;

INITIALIZE_PASS_BEGIN(IfConvertionPreRegAllocation, "if-convertion-pre-reg-allocation", "If Convertion on machine code before register allocation", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineBranchProbabilityInfo);
INITIALIZE_PASS_END(IfConvertionPreRegAllocation, "if-convertion-pre-reg-allocation", "If Convertion on machine code before register allocation", false, false)
