#include <initializer_list>
#include <iostream>
#include <vector>
#include <queue>

constexpr int NO_REG = -1;
constexpr int REGS_PER_THREAD = 64;

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
  std::vector<Instruction> instructions;

  explicit Warp(int i) : id(i) {}

};

struct Scoreboard {
  std::vector<std::array<bool, REGS_PER_THREAD>> pending; 
public: 
  explicit Scoreboard(int nwarps) : pending(nwarps) {}
  bool has_hazard(int w, const Instruction& in) const {
    for (int r : in.src) {
      if (r != NO_REG && pending[w][r]) return true;   // RAW
    }
    return in.dst != NO_REG && pending[w][in.dst];                        // WAW
  }
  void reserve(int w, int r) { if (r != NO_REG) pending[w][r] = true; }
  void release(int w, int r) { if (r != NO_REG) pending[w][r] = false; }
};

class FixedLatencyUnit {
  struct inFlight { int warp; int dst; uint64_t readyCycle; };
  std::deque<inFlight> inflight;
  uint32_t latency, capacity;
public: 
  FixedLatencyUnit(uint32_t l, uint32_t c) : latency(l), capacity(c) {}
  bool isFull() const { return inflight.size() >= capacity; }
  void push(int w, int d, uint64_t cycle) { inflight.push_back({w,d,cycle+latency}); }
  void cycle(uint64_t cycle, Scoreboard& sb) {
    while (!inflight.empty() && inflight.front().readyCycle <= cycle) {
      sb.release(inflight.front().warp, inflight.front().dst);
      inflight.pop_front();
    }
  }
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
    for(int i{}; i < warps.size(); i++) {
      if(warps[i].stalled) {
        continue;
      }
      memory.request(MemRequest{warps[i].id, warps[i].idx, MemSpace::SHARED, {0 /* Doesnt matter rn*/}, 0xFFFFFFFF});
    }
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

