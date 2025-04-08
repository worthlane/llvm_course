#include "GraphBuilder.hpp"

namespace visual {

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

GraphBuilder::GraphBuilder(const std::string& file_name) {
dot_stream_.open(file_name);

dot_stream_ << "digraph structs {\n"
                "graph[splines=true, overlap=false, pack=true];\n"
                "node[shape=Mrecord, style=filled, fillcolor=\"lightgray\", color=\"black\", fontsize=20];\n"
                "edge[color=\"darkblue\",fontcolor=\"yellow\",fontsize=12];\n\n";
}

GraphBuilder::~GraphBuilder() {
  dot_stream_ << "}\n";

  dot_stream_.close();
}

void GraphBuilder::defineNode(const NodeId id, const std::string& label) {
  dot_stream_ << id << " [label=\"" << nameInDotFormat(label) << "\"];\n";
}

void GraphBuilder::constructEdge(const NodeId start, const NodeId end) {
  dot_stream_ << start << " -> " << end << "\n";
}

} // namespace visual
