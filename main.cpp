#include <iostream>

enum class OpClass { INT, FP32, FP64 };
struct Instruction {
  OpClass op;
  int dst;
  int src[3];
};

struct Warp {
  int id;
  int idx;
  uint32_t mask;
  bool stalled;
  std::vector<Instruction> instructions;
};

class SM { // This will be the thing that handles next instruction
  std::vector<Warp> warps;

  public:
  void cycle() {

  }
};

class GPU {
  uint64_t clock_cycles;
  std::vector<SM> sms;

  public:

  GPU() : clock_cycles(0u), sms() {}


};
