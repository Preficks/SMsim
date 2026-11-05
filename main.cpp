#include <initializer_list>
#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>

constexpr int NO_REG = -1;
constexpr int REGS_PER_THREAD = 64;
constexpr int k_num_pipeline_phases = 6;

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

struct Register {
  int id;
  uint32_t value;
  bool pending_write; // If true then the register is waiting for a write to complete
};

class SMMemUnit { // Models the l1 cache and shared mem of the SM, fixed-latency for now, LRU set assosciative cache for later
  std::queue<MemRequest> requests;
  std::array<Register, 32> registers;
  uint32_t latency; // Latency of the memory unit in cycles

  void cycle() {
    
  }

  public:

  void request(MemRequest req) { // Another request pushed to the queue
    registers[req.dst_reg].pending_write = true;
    requests.push(req);
  }

  bool is_pending_write(int reg_id) {
    return registers[reg_id].pending_write;
  }
};

struct Warp {
  int id; // Warp ID
  int idx; //
  uint32_t mask; // Active mask, tells which elements are being operated on
  bool stalled; // If true then stalled, wait another clock cycle
  std::deque<Instruction> instructions;

  explicit Warp(int i) : id(i) {}
};

struct LogicUnit {
  int max_instructions; // max size of the queue
  int latency; // latency of the logic unit, in cycles
  std::queue<std::pair<Instruction, int>> enqueued_instructions;
  explicit LogicUnit(int m, int l) : max_instructions(m), latency(l) {}
  void enqueue(Instruction& inst, int cycle) {
    enqueued_instructions.push(std::pair<Instruction, int>{inst, cycle + latency});
  }
  bool ready(int cycle) {
    if(cycle >= enqueued_instructions.front().second) return true;
  }
  bool pop(int cycle) {
    if(ready(cycle)) {
      enqueued_instructions.pop();
      return true;
    } 
    return false;
  }
};

struct Scoreboard {
  std::unordered_map<int, bool> warps; 
  explicit Scoreboard(uint32_t num_warps) : warps(num_warps) {}
  bool is_valid(const Warp& x) { return warps[x.id]; }
};

class SM { 
  SMMemUnit memory;
  std::vector<Warp> warps;
  LogicUnit LU{INT_MAX, 1}; // Ideal LU, used for every instructin type for now
  int idx = 0; // for round robin scheduling
  int cycle_count{};

  public:

  void cycle() {
    writeback();
    execute();
    read_operands();
    issue();         
    decode();
    fetch();

    ++cycle_count;
  }

  void writeback() { //TODO: loop through LUs and pop, clear the screboard

  }

  bool can_issue(const Warp& warp) { 
    return true; 
  }

  bool issue() { // TODO: Add consideration of Logic units and if already pushed this phase for multiple schedulers
    const int n = warps.size();
    if (n == 0) return false;

    for(int i{}; i < n; i++) {
      int w = (idx + i) % n;
      auto& warp = warps[w];

      if(warp.instructions.empty()) continue;
      if(!can_issue(warp)) continue;

      LU.enqueued_instructions.push(std::move(warp.instructions.front()));
      warp.instructions.pop_front();           

      idx = (w + 1) % n;                      
      return true;
    }
    return false;    
  }
  
};
/* Not ready for this abstraction; wait till SM is fully fleshed out */
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

