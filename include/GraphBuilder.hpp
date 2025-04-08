#pragma once

#include <fstream>

namespace visual {

class GraphBuilder {
  using NodeId = unsigned int;

 public:
  explicit GraphBuilder(const std::string&);

  // Non-movable
  GraphBuilder(GraphBuilder&&) = delete;
  GraphBuilder& operator=(GraphBuilder&&) = delete;

  // Non-copyable
  GraphBuilder(const GraphBuilder&) = delete;
  GraphBuilder& operator=(const GraphBuilder&) = delete;

  ~GraphBuilder();

  void defineNode(const NodeId id, const std::string& label);
  void constructEdge(const NodeId start, const NodeId end);

 private:
  std::ofstream dot_stream_;
};

} // namespace visual
