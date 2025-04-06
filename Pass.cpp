#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {
  static std::string nameInDotFormat(const std::string& name) {
    std::string result;

    static const size_t kMaxSymbols = 100;

    size_t cnt = 0;
    for (char c : name) {
      if (c == '"' || c == '\\' || c == '<' || c == '>' || c == '{' || c == '}' || c == '|') {
        result += '\\';
      }
      result += c;

      if (++cnt > kMaxSymbols)
        break;
    }

    return result;
  }

  struct MyPass : public FunctionPass {
    static char ID;
    MyPass() : FunctionPass(ID) {
      outs() << "digraph structs {\n"
                "node[color=\"black\",fontsize=14];\n"
                "edge[color=\"darkblue\",fontcolor=\"yellow\",fontsize=12];\n\n";
    }

    ~MyPass() {
      outs() << "}\n";
    }

    std::string getName(Value* V) {
      std::string dot_name;

      if (Instruction* I = dyn_cast<Instruction>(V)) {
        dot_name = std::string(I->getOpcodeName());
      } else if (Constant* C = dyn_cast<Constant>(V)) {
        std::string str;
        raw_string_ostream rso(str);
        C->print(rso);
        dot_name = rso.str();
      } else if (BasicBlock* B = dyn_cast<BasicBlock>(V)) {
        dot_name = (B->hasName()) ? B->getName().str() :
                                    std::string("unnamed_label");
      } else if (V->hasName()) {
        dot_name = V->getName().str();
      } else {
        dot_name = "unnamed_value";
      }

      return nameInDotFormat(dot_name);
    }

    unsigned int getNodeID(Value* V) {
      return reinterpret_cast<uint64_t>(V);
    }

    void defineNode(Value* V) {
      outs() << getNodeID(V) << " [shape=Mrecord, style=filled, fillcolor=\"lightblue\","
                                "color = \"darkblue\", label=\"" << getName(V) << "\"];\n";
    }

    virtual bool runOnFunction(Function& F) {
      for (auto& B : F) {
        // Basic Block iterations

        for (auto& I : B) {
          // Instruction iterations

          defineNode(&I);

          /*outs() << "Uses:\n";
          for (auto& U : I.uses()) {
            User* user = U.getUser();
            outs() << getName(&I) << " -> " << getName(user) << "\n";
          }*/

          for (auto& U : I.operands()) {
            Value* op = U.get();

            if (dyn_cast<Constant>(op))
              continue;

            defineNode(op);

            outs() << getNodeID(op) << " -> " << getNodeID(&I) << "\n";
          }
        }

        outs() << "\n";
      }

      return false;
    }
  };
}

char MyPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerMyPass(const PassManagerBuilder&,
                           legacy::PassManagerBase& PM) {
  PM.add(new MyPass());
}
static RegisterStandardPasses
    RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                   registerMyPass);
