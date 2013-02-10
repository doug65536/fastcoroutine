
// MINGW bug: need stdlib included first to get _aligned_malloc
#include <cstdlib>

#include <Windows.h>
//#include <cstddef>
//#include <cassert>
#include <cstdio>
#include <cstdint>
#include <malloc.h>
#include <map>

// Since we're not really switching contexts, we only actually need to save
// the call saved registers!
struct SavedContextFrame
{
  // I pair-up the xmmwords because I need to prevent 16-byte alignment of size
  void *r15;
  void *xmm6lo, *xmm6hi;
  void *xmm7lo, *xmm7hi;
  void *xmm8lo, *xmmh8i;
  void *xmm9lo, *xmmh9i;
  void *xmm10lo, *xmm10hi;
  void *xmm11lo, *xmm11hi;
  void *xmm12lo, *xmm12hi;
  void *xmm13lo, *xmm13hi;
  void *xmm14lo, *xmm14hi;
  void *xmm15lo, *xmm15hi;
  void *r14;
  void *r13; void *r12;
  void *rdi; void *rsi;
  void *rbp; void *rbx;
};

// This has a few extra entries at the bottom of
// the stack to handle a thread returning
struct NewFrame
{
  SavedContextFrame context;

  void (*rip)();
};

struct Task
{
  typedef std::map<int,Task> TaskMap;

  // rsp at beginning of structure for convenience from asm
  SavedContextFrame *rsp;

  enum State
  {
    INVALID,
    ROOT,
    CREATED,
    EXITED,
  };
  State state;

  void *stack;
  size_t stacksize;

  TaskMap::iterator taskmapiter;

  Task()
    : rsp(0)
    , state(INVALID)
    , stack(0)
  {
  }
};

typedef Task::TaskMap TaskMap;
TaskMap tasks;

extern "C" Task *current_task;
Task *current_task;
TaskMap::iterator current_task_iterator;

int next_task_id = 1;

extern "C" void TaskBootup(void *rsp);
extern "C" void SwitchToTask(void *rsp, void **old_task_rsp);
extern "C" void SwitchToNextTask();
extern "C" void StartNewTask();

extern "C" _NT_TIB *get_TEB();

void SetTibStackRange(const Task *task)
{
  auto teb = get_TEB();
  teb->StackLimit = current_task->stack;
  teb->StackBase = (char*)current_task->stack + current_task->stacksize;
}

void GetTibStackRange(const Task *task)
{
  // If you pretend the stack is a vector
  // StackBase = stack.end()
  // StackLimit = stack.begin()
  auto teb = get_TEB();
  current_task->stack = teb->StackLimit;
  current_task->stacksize = uintptr_t(teb->StackBase) - uintptr_t(current_task->stack);
}

extern "C" SavedContextFrame *ScheduleTask(SavedContextFrame *rsp)
{
  // Save outgoing task's frame
  current_task->rsp = rsp;

  // Update our stack range
  GetTibStackRange(current_task);

  // This only loops if there is work to do and current task is root task
  // Usually, it doesn't loop
  do
  {
    // See if outgoing task needs self destruct
    if (current_task->state == Task::EXITED)
    {
      current_task_iterator = tasks.erase(current_task->taskmapiter);
      if (current_task_iterator == tasks.end())
        current_task_iterator = tasks.begin();
      current_task = &current_task_iterator->second;
    }
    else
    {
      // Advance it, and special case going off the end
      if (++current_task_iterator == tasks.end())
        current_task_iterator = tasks.begin();
      current_task = &current_task_iterator->second;
    }
  }
  while (current_task->state == Task::ROOT && tasks.size() > 1);

  // Load TIB from task
  SetTibStackRange(current_task);

  return current_task->rsp;
}

extern "C" void task_exit()
{
  current_task->state = Task::EXITED;
  SwitchToNextTask();
}

void test_thread(void *a1, void *a2, void *a3, void *a4)
{
  for (auto i = 0; i < (1<<12); ++i)
    SwitchToNextTask();
}

void test_thread2(void *a1, void *a2, void *a3, void *a4)
 {
  for (auto i = 0; i < 1<<12; ++i)
  {
    union Horrible
    {
      __m128i mem;
      int ints[4];
    } h;

    __m128i *readme = &h.mem;
    __m128i r0, r1, r2, r3, r4, r5, r6, r7;
    __m128i r8, r9,r10,r11,r12,r13,r14,r15;
    r0  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
    r1  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
    r2  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
    r3  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
    r4  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
    r5  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
    r6  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
    r7  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
    r8  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
    r9  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
    r10 = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
    r11 = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
    r12 = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
    r13 = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
    r14 = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
    r15 = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
    r0  = _mm_or_si128(r0 , r1 );
    r2  = _mm_or_si128(r2 , r3 );
    r4  = _mm_or_si128(r4 , r5 );
    r6  = _mm_or_si128(r6 , r7 );
    r8  = _mm_or_si128(r8 , r9 );
    r10 = _mm_or_si128(r10, r11);
    r12 = _mm_or_si128(r12, r13);
    r14 = _mm_or_si128(r14, r15);
    r0  = _mm_or_si128(r0 , r2 );
    r4  = _mm_or_si128(r4 , r6 );
    r8  = _mm_or_si128(r8 , r10);
    r10 = _mm_or_si128(r10, r12);
    r12 = _mm_or_si128(r12, r14);
    r0  = _mm_or_si128(r0 , r4 );
    r8  = _mm_or_si128(r8 , r12);
    r0  = _mm_or_si128(r0 , r8);
    *readme = r0;
    //printf("0x%p 0x%p 0x%p 0x%p\n", a1, a2, a3, a4);
    SwitchToNextTask();
  }
  //printf("0x%p 0x%p 0x%p 0x%p - done\n", a1, a2, a3, a4);
}

void CreateTask(void (*f)(void*,void*,void*,void*),
  void *a0, void *a1, void *a2, void *a3)
{
  // Allocate a task ID
  int new_task_id = next_task_id++;

  // Create a new task with that ID in the task map
  auto ins_iter = tasks.insert(tasks.end(), TaskMap::value_type(new_task_id, Task()));

  Task &task = ins_iter->second;
  task.taskmapiter = ins_iter;

  // Allocate a new stack and put the context at the end of it
  // We need to make sure that the stack is misaligned, because it
  // is always misaligned upon the call to SwitchTo*Task
  // So we add 8
  task.stacksize = 1<<20;
  task.stack = (void**)_aligned_malloc(task.stacksize, 16) + 8;

  auto &frame = ((NewFrame*)((char*)task.stack + task.stacksize))[-1];
  memset(&frame, 0xee, sizeof(frame));

//  frame.context.unused = 0;
  frame.rip = StartNewTask;

  // When we're creating a new task, we own all the registers
  // in the frame, so we put all the information into those
  // and make it start at a task creation thunk

  frame.context.rbx = (void*)uintptr_t(uint32_t(new_task_id));
  frame.context.rbp = (void*)f;
  frame.context.r12 = a0;
  frame.context.r13 = a1;
  frame.context.r14 = a2;
  frame.context.r15 = a3;
  frame.context.xmm6lo = (void*)0x2222222211111111ULL;
  frame.context.xmm6hi = (void*)0x4444444433333333ULL;
  frame.context.xmm7lo = (void*)0x6666666655555555ULL;
  frame.context.xmm7hi = (void*)0x8888888877777777ULL;

  task.rsp = &frame.context;

  task.state = Task::CREATED;

  if (current_task)
    return;

  // We only reach here first time
  // Creates the "root" task, won't resume until no work left
  // Current task is the root (id=0) task
  current_task_iterator = tasks.insert(TaskMap::value_type(0, Task())).first;
  current_task = &current_task_iterator->second;
  current_task->state = Task::ROOT;

  // Read stack limits
  GetTibStackRange(current_task);
}

int main()
{
  int64_t retry = 0;
  int64_t total = 0;
  for ( ; retry < 1024; ++retry)
  {
    for (auto i = (char *)0; i < (char *)8; ++i)
      CreateTask(test_thread, i + 1, i + 2, i + 3, i + 4);

    auto c1 = __rdtsc();
    SwitchToNextTask();
    auto c2 = __rdtsc();

    total += c2-c1;

    printf("Average cycles per context switch: %lld\n", (c2-c1)>>15);
  }

  printf("\nAverage cycles per context switch: %lld\n", (total/retry)>>15);

  return 0;
}
