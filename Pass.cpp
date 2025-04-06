#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {
  struct MyPass : public FunctionPass {
    static char ID;
    MyPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function& F) {
      outs() << "In a function called " << F.getName() << "!\n";

      for (auto& B : F) {
        outs() << "-------- NEW BASIC BLOCK -------\n";

        for (auto& I : B) {
          //I.print(outs(), true);
          outs() << I.getOpcodeName() << "\n";
        }

        outs() << "\n";
      }

      return false;
    }
  };
}

char MyPass::ID = 0;

static void registerMyPass(const PassManagerBuilder&,
                           legacy::PassManagerBase& PM) {
  PM.add(new MyPass());
}
static RegisterStandardPasses
    RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                   registerMyPass);
