#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

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
                "graph[splines=true, overlap=false, pack=true];\n"
                "node[shape=Mrecord, style=filled, fillcolor=\"lightgray\", color=\"black\", fontsize=20];\n"
                "edge[color=\"darkblue\",fontcolor=\"yellow\",fontsize=12];\n\n";
    }

    ~MyPass() {
      outs() << "}\n";
    }

    std::string getBasicBlockLabel(BasicBlock* B) {
      std::string str;
      raw_string_ostream rso(str);
      B->print(rso);
      size_t label_end = str.find(':');
      return rso.str().substr(0, label_end + 1);
    }

    std::string getDumpedValue(Value* V) {
      std::string str;
      raw_string_ostream rso(str);
      V->print(rso);
      return rso.str();
    }

    std::string getName(Value* V) {
      std::string dot_name;

      if (Instruction* I = dyn_cast<Instruction>(V)) {
        dot_name = std::string(I->getOpcodeName());
      } else if (BasicBlock* B = dyn_cast<BasicBlock>(V)) {
        dot_name = getBasicBlockLabel(B);
      } else if (V->hasName()) {
        dot_name = V->getName().str();
      } else {
        dot_name = getDumpedValue(V);
      }

      return nameInDotFormat(dot_name);
    }

    size_t last_constant_id_{0};
    unsigned int getNodeID(Value* V) {
      if (isa<Constant>(*V))
        return last_constant_id_;

      return reinterpret_cast<uint64_t>(V);
    }

    void defineNode(Value* V) {
      outs() << getNodeID(V) << " [label=\"" << getName(V) << "\"];\n";
    }

    void constructEdge(Value* start, Instruction& end) {
      size_t weight = 1;

      outs() << getNodeID(start) << " -> " << getNodeID(&end) << "[weight=" << weight << "]\n";

      if (isa<Constant>(*start)) {
        last_constant_id_++; // cloning constants to simplify graph
      }
    }

    const std::string kLoggerFunc = "logInstruction";
    bool isFuncLogger(StringRef func) {
      return func == kLoggerFunc;
    }

    void logInstruction(Function& F, Instruction& I) {
      LLVMContext& Ctx = I.getContext();
      IRBuilder<> builder(Ctx);

      Type* ret_type = Type::getVoidTy(Ctx);
      ArrayRef<Type*> log_instr_param_types = {builder.getInt8Ty()->getPointerTo()}; //Type::getInt8PtrTy(Ctx),
                                                  //Type::getInt64PtrTy(Ctx)};
      FunctionType* log_instr_type = FunctionType::get(ret_type,
                                                       log_instr_param_types,
                                                       false);
      FunctionCallee func_log_instr = F.getParent()->getOrInsertFunction(kLoggerFunc,
                                                                         log_instr_type);

      //std::string opcode_name = I.getOpcodeName();
      //Value* instr_name = builder.CreateGlobalStringPtr(opcode_name);

      /*GlobalVariable* counter = new GlobalVariable(
        *I.getModule(),
        Type::getInt64Ty(Ctx),
        false,
        GlobalValue::InternalLinkage,
        ConstantInt::get(Type::getInt64Ty(Ctx), 0),
        opcode_name + "_counter"
      );*/

      if (auto* call = dyn_cast<CallInst>(&I)) {
        builder.SetInsertPoint(call);

        Function* callee = call->getCalledFunction();
        if (callee && !isFuncLogger(callee->getName())) {
          Value* opcode = builder.CreateGlobalStringPtr(call->getOpcodeName());
          Value* args[] = {opcode};
          builder.CreateCall(func_log_instr, args);
        }
      }
    }

    virtual bool runOnFunction(Function& F) {
      for (auto& B : F) {
        // Basic Block iterations
        for (auto& I : B) {
          // Instruction iterations
          //defineNode(&I);

          I.print(outs(), true);
          outs() << "\n";

          logInstruction(F, I);

          for (auto& U : I.operands()) {
            Value* op = U.get();

            //defineNode(op);
            //constructEdge(op, I);
          }
        }
      }
      outs() << "return\n";
      return true;
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
