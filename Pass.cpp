#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include <fstream>

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

  struct GraphPass : public FunctionPass {
    static char ID;

    std::ofstream dot;
    const std::string kFileName       = "assets/graph.dot";
    const std::string kLoggerFunc     = "logInstruction";
    const std::string kLogInitializer = "initLogFile";

    bool   log_file_initted_{false};
    size_t last_constant_id_{0};

    GraphPass() : FunctionPass(ID) {
      dot.open(kFileName);

      dot << "digraph structs {\n"
                "graph[splines=true, overlap=false, pack=true];\n"
                "node[shape=Mrecord, style=filled, fillcolor=\"lightgray\", color=\"black\", fontsize=20];\n"
                "edge[color=\"darkblue\",fontcolor=\"yellow\",fontsize=12];\n\n";
    }

    ~GraphPass() {
      dot << "}\n";

      dot.close();
    }

    void initLogFile(Function& F) {
      if (log_file_initted_)
        return;

      LLVMContext& Ctx = F.getContext();
      IRBuilder<> builder(Ctx);

      ArrayRef<Type*> init_param_types = {};
      FunctionType* init_type = FunctionType::get(builder.getVoidTy(), init_param_types, false);
      FunctionCallee init_func = F.getParent()->getOrInsertFunction(kLogInitializer, init_type);

      BasicBlock& entry_BB = F.getEntryBlock();
      builder.SetInsertPoint(&entry_BB.front());

      builder.CreateCall(init_func);

      log_file_initted_ = true;
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

    unsigned int getNodeID(Value* V) {
      if (isa<Constant>(*V))
        return last_constant_id_;

      return reinterpret_cast<uint64_t>(V);
    }

    void defineNode(Value* V) {
      dot << getNodeID(V) << " [label=\"" << getName(V) << "\"];\n";
    }

    void constructEdge(Value* start, Instruction& end) {
      size_t weight = 1;

      dot << getNodeID(start) << " -> " << getNodeID(&end) << "[weight=" << weight << "]\n";

      if (isa<Constant>(*start)) {
        last_constant_id_++; // cloning constants to simplify graph
      }
    }

    bool isFuncLogger(StringRef func) {
      return func == kLoggerFunc || func == kLogInitializer;
    }

    void insertLogInstruction(IRBuilder<>& builder, LLVMContext& Ctx,
                              Function& F, Instruction& I, FunctionCallee& func_calee) {
      builder.SetInsertPoint(&I);

      if (auto* call = dyn_cast<CallBase>(&I)) {
        Function* callee = call->getCalledFunction();
        if (!callee || isFuncLogger(callee->getName())) {
          return;
        }
      }

      unsigned int node_id = getNodeID(&I);
      std::string opcode_name = I.getOpcodeName();
      std::string counter_name = "counter_" + opcode_name + std::to_string(node_id);
      GlobalVariable* counter = F.getParent()->getNamedGlobal(counter_name);

      if (!counter) {
        counter = new GlobalVariable(
          *F.getParent(),
          Type::getInt64Ty(Ctx),
          false,
          GlobalValue::InternalLinkage,
          ConstantInt::get(Type::getInt64Ty(Ctx), 0),
          counter_name
        );
      }

      Value* opcode = builder.CreateGlobalStringPtr(I.getOpcodeName());
      Value* id = ConstantInt::get(builder.getInt32Ty(), getNodeID(&I));
      Value* args[] = {opcode, counter, id};
      builder.CreateCall(func_calee, args);
    }

    void logInstruction(Function& F, Instruction& I) {
      if (isa<LandingPadInst>(I) || isa<PHINode>(I))
        return;

      LLVMContext& Ctx = I.getContext();
      IRBuilder<> builder(Ctx);

      Type* ret_type = Type::getVoidTy(Ctx);
      ArrayRef<Type*> log_instr_param_types = {builder.getInt8Ty()->getPointerTo(),
                                               builder.getInt64Ty()->getPointerTo(),
                                               builder.getInt32Ty()};
      FunctionType* log_instr_type = FunctionType::get(ret_type,
                                                       log_instr_param_types,
                                                       false);
      FunctionCallee func_log_instr = F.getParent()->getOrInsertFunction(kLoggerFunc,
                                                                         log_instr_type);

      insertLogInstruction(builder, Ctx, F, I, func_log_instr);
    }

    virtual bool runOnFunction(Function& F) {
      initLogFile(F);

      for (auto& B : F) {
        // Basic Block iterations
        for (auto& I : B) {
          // Instruction iterations
          logInstruction(F, I);

          defineNode(&I);

          for (auto& U : I.operands()) {
            Value* op = U.get();

            defineNode(op);
            constructEdge(op, I);
          }
        }
      }
      return true;
    }
  };
}

char GraphPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerMyPass(const PassManagerBuilder&,
                           legacy::PassManagerBase& PM) {
  PM.add(new GraphPass());
}
static RegisterStandardPasses
    RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                   registerMyPass);
