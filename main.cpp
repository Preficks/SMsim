#include <initializer_list>
#include <iostream>


enum class OpClass { INT, FP32, FP64 }; // Classes of types since operations within each class take roughly the same amount of time

struct Instruction {
  OpClass op; // Underlying operation
  int dst; // Destination registers
  int src[3]; // Source registers
};

enum class MemSpace { SHARED, GLOBAL };
struct MemRequest {
  int warp;
  int dst_reg;
  MemSpace space;
  uint64_t addr[32];
  uint32_t active_mask;
};

class SMMemUnit { // Models the l1 cache and shared mem of the SM, fixed-latency for now, LRU set assosciative cache for later
  std::queue<MemRequest> requests;
  void cycle() {

  }
};

struct Warp {
  int id; // Warp ID
  int idx; //
  uint32_t mask; // Active mask, tells which elements are being operated on
  bool stalled; // If true then stalled, wait another clock cycle
  std::vector<Instruction> instructions;

  explicit Warp(int i) : id(i) {}

};

class SM { // This will be the thing that handles next instruction
  std::queue<Warp> pipeline; // Models the current pipeline (6 stages, each 1 cycle)
  SMMemUnit memory;
  std::vector<Warp> warps;

  public:

  void cycle() {
    run_scheduler();
  }

  void run_scheduler() { // loose round robin scheduling

  }
};

class GPU {
  std::vector<SM*> sms;
  uint64_t clock_cycles;

  public:

// TODO: add scheduling functionality and move sms to private


  GPU() : clock_cycles(0u), sms() {
    sms.push_back(new SM());
  }

  void cycle() {
    for(auto& x : sms) {
      x->cycle();
    }
    clock_cycles++;
  }
};


int main() {

}

/*
 * What kind of questions do I want to ask
 * How can I most effectively prepare for the role?
 * What kind of things are you looking for?
 * Any gaps in my knowledge I should fill?
 * I should lowkey ask if graduating in 2029 is chill
 *
 *
 */
