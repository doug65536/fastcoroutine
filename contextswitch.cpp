
// MINGW bug: need stdlib included first to get _aligned_malloc
#include <cstdlib>

#include <Windows.h>
#include <cstdio>
#include <cstdint>
#include <malloc.h>
#include <map>
#include <functional>
#include <exception>
#include <iostream>

struct SavedContextFrame;

// Assembly language stuff:
extern "C" void TaskBootup(void *rsp);
extern "C" void SwitchToTask(SavedContextFrame *rsp, SavedContextFrame **old_task_rsp);
//extern "C" void SwitchToNextTask();
extern "C" void StartNewTask();
extern "C" _NT_TIB *get_TEB();
extern "C" void task_terminate() { std::terminate(); }

// Since we're not *really* switching contexts, we only actually need to save
// the callee saved registers!
struct SavedContextFrame
{
  // Pair-up the xmmwords because we need to prevent 16-byte alignment of xmm6
  void *rbp;
  void *xmm6lo, *xmm6hi;
  void *xmm7lo, *xmm7hi;
  void *xmm8lo, *xmm8hi;
  void *xmm9lo, *xmm9hi;
  void *xmm10lo, *xmm10hi;
  void *xmm11lo, *xmm11hi;
  void *xmm12lo, *xmm12hi;
  void *xmm13lo, *xmm13hi;
  void *xmm14lo, *xmm14hi;
  void *xmm15lo, *xmm15hi;
  void *r15; void *r14;
  void *r13; void *r12;
  void *rdi; void *rsi;
  void *rbx;
};

// This has an extra entry at the bottom of
// the stack to handle thread startup
struct NewFrame
{
  SavedContextFrame context;

  void (*rip)();
};

class Task
{
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

  Task *owner;
  void *stack;
  size_t stacksize;

private:

  void GetTibStackRange()
  {
    // If you pretend the stack is a vector
    // StackBase = stack.end()
    // StackLimit = stack.begin()
    auto &teb = *get_TEB();
    stack = teb.StackLimit;
    stacksize = uintptr_t(teb.StackBase) - uintptr_t(stack);
  }

  void SetTibStackRange()
  {
    auto &teb = *get_TEB();
    teb.StackLimit = stack;
    teb.StackBase = (char*)stack + stacksize;
  }

public:

  Task(Task *owner)
    : owner(owner)
    , rsp(0)
    , state(INVALID)
    , stack(0)
  {
  }

  void CreateContext(void (*f)(void*,void*,void*,void*),
    void *a0 = nullptr, void *a1 = nullptr,
    void *a2 = nullptr, void *a3 = nullptr)
  {
    // Allocate a new stack and put the context at the end of it
    // We need to make sure that the stack is misaligned, because it
    // is always misaligned upon the call to SwitchTo*Task
    // So we add 8
    stacksize = 1<<20;
    stack = (void**)_aligned_malloc(stacksize, 16) + 8;

    // Compute the location of the starting context and get a reference to it
    NewFrame &frame = ((NewFrame*)((char*)stack + stacksize))[-1];
    memset(&frame, 0xee, sizeof(frame));

    //  frame.context.unused = 0;
    frame.rip = StartNewTask;

    // When we're creating a new task, we own all the registers
    // in the frame, so we put all the information into those
    // and make it start at a task creation thunk

    frame.context.rbx = (void*)uintptr_t(this);
    frame.context.rbp = (void*)f;
    frame.context.r12 = a0;
    frame.context.r13 = a1;
    frame.context.r14 = a2;
    frame.context.r15 = a3;
#ifndef NDEBUG
    // Fill initial SSE registers with
    frame.context.xmm6lo  = (void*)0xEEEEEE61EEEEEE60ULL;
    frame.context.xmm6hi  = (void*)0xEEEEEE61EEEEEE60ULL;
    frame.context.xmm7lo  = (void*)0xEEEEEE71EEEEEE70ULL;
    frame.context.xmm7hi  = (void*)0xEEEEEE71EEEEEE70ULL;
    frame.context.xmm8lo  = (void*)0xEEEEEE81EEEEEE80ULL;
    frame.context.xmm8hi  = (void*)0xEEEEEE81EEEEEE80ULL;
    frame.context.xmm9lo  = (void*)0xEEEEEE91EEEEEE90ULL;
    frame.context.xmm9hi  = (void*)0xEEEEEE91EEEEEE90ULL;
    frame.context.xmm10lo = (void*)0xEEEEEEA1EEEEEEA0ULL;
    frame.context.xmm10hi = (void*)0xEEEEEEA1EEEEEEA0ULL;
    frame.context.xmm11lo = (void*)0xEEEEEEB1EEEEEEB0ULL;
    frame.context.xmm11hi = (void*)0xEEEEEEB1EEEEEEB0ULL;
    frame.context.xmm12lo = (void*)0xEEEEEEC1EEEEEEC0ULL;
    frame.context.xmm12hi = (void*)0xEEEEEEC1EEEEEEC0ULL;
    frame.context.xmm13lo = (void*)0xEEEEEED1EEEEEED0ULL;
    frame.context.xmm13hi = (void*)0xEEEEEED1EEEEEED0ULL;
    frame.context.xmm14lo = (void*)0xEEEEEEE1EEEEEEE0ULL;
    frame.context.xmm14hi = (void*)0xEEEEEEE1EEEEEEE0ULL;
    frame.context.xmm15lo = (void*)0xEEEEEEF1EEEEEEF0ULL;
    frame.context.xmm15hi = (void*)0xEEEEEEF1EEEEEEF0ULL;
#endif
    rsp = &frame.context;
    state = Task::CREATED;
  }

  void SwitchTo(Task &outgoing_task)
  {
    outgoing_task.GetTibStackRange();
    SetTibStackRange();
    // Call assembly code:
    SwitchToTask(rsp, &outgoing_task.rsp);
  }

  void Exit()
  {
    state = Task::EXITED;
    owner->SwitchTo(*owner);
  }
};

//extern "C" SavedContextFrame *ScheduleTask(SavedContextFrame *rsp)
//{
//  // Save outgoing task's frame
//  current_task->rsp = rsp;
//
//  // Update our stack range
//  GetTibStackRange(current_task);
//
//  // This only loops if there is work to do and current task is root task
//  // Usually, it doesn't loop
//  do
//  {
//    // See if outgoing task needs self destruct
//    if (current_task->state == Task::EXITED)
//    {
//      current_task_iterator = tasks.erase(current_task->taskmapiter);
//      if (current_task_iterator == tasks.end())
//        current_task_iterator = tasks.begin();
//      current_task = &current_task_iterator->second;
//    }
//    else
//    {
//      // Advance it, and special case going off the end
//      if (++current_task_iterator == tasks.end())
//        current_task_iterator = tasks.begin();
//      current_task = &current_task_iterator->second;
//    }
//  }
//  while (current_task->state == Task::ROOT && tasks.size() > 1);
//
//  // Load TIB from task
//  SetTibStackRange(current_task);
//
//  return current_task->rsp;
//}

//void test_thread(void *a1, void *a2, void *a3, void *a4)
//{
//  for (auto i = 0; i < (1<<12); ++i)
//    SwitchToNextTask();
//}

//void test_thread2(void *a1, void *a2, void *a3, void *a4)
// {
//  for (auto i = 0; i < 1<<12; ++i)
//  {
//    union Horrible
//    {
//      __m128i mem;
//      int ints[4];
//    } h;
//
//    __m128i *readme = &h.mem;
//    __m128i r0, r1, r2, r3, r4, r5, r6, r7;
//    __m128i r8, r9,r10,r11,r12,r13,r14,r15;
//    r0  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
//    r1  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
//    r2  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
//    r3  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
//    r4  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
//    r5  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
//    r6  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
//    r7  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
//    r8  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
//    r9  = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
//    r10 = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
//    r11 = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
//    r12 = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
//    r13 = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
//    r14 = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
//    r15 = _mm_or_si128(_mm_setzero_si128(), *readme); SwitchToNextTask();
//    r0  = _mm_or_si128(r0 , r1 );
//    r2  = _mm_or_si128(r2 , r3 );
//    r4  = _mm_or_si128(r4 , r5 );
//    r6  = _mm_or_si128(r6 , r7 );
//    r8  = _mm_or_si128(r8 , r9 );
//    r10 = _mm_or_si128(r10, r11);
//    r12 = _mm_or_si128(r12, r13);
//    r14 = _mm_or_si128(r14, r15);
//    r0  = _mm_or_si128(r0 , r2 );
//    r4  = _mm_or_si128(r4 , r6 );
//    r8  = _mm_or_si128(r8 , r10);
//    r10 = _mm_or_si128(r10, r12);
//    r12 = _mm_or_si128(r12, r14);
//    r0  = _mm_or_si128(r0 , r4 );
//    r8  = _mm_or_si128(r8 , r12);
//    r0  = _mm_or_si128(r0 , r8);
//    *readme = r0;
//    //printf("0x%p 0x%p 0x%p 0x%p\n", a1, a2, a3, a4);
//    SwitchToNextTask();
//  }
//  //printf("0x%p 0x%p 0x%p 0x%p - done\n", a1, a2, a3, a4);
//}

template<typename C, typename R, typename A>
auto ArgumentType(R (C::*)(A)) -> A;

template<typename Y>
class Enumerator;

// Holds the data being yielded while doing context switch
template<typename Y>
class YieldBuffer
{
  Enumerator<Y> &owner;
  Y buffer;

  friend class Enumerator<Y>;
  YieldBuffer(Enumerator<Y> &owner);

public:
  // YieldReturn is called from the Enumerator to deliver
  // a result and switch context back to the caller

  template<typename R>
  typename std::enable_if<!std::is_convertible<R,Y>::value>::type
  YieldReturn(R &&result) = delete;

  template<typename R>
  typename std::enable_if<std::is_convertible<R,Y>::value>::type
  YieldReturn(R &&result);
};

template<typename Y>
YieldBuffer<Y>::YieldBuffer(Enumerator<Y> &owner)
  : owner(owner)
{
}

template<typename Y> template<typename R>
typename std::enable_if<std::is_convertible<R,Y>::value>::type
YieldBuffer<Y>::YieldReturn(R &&result)
{
  buffer = std::forward<R>(result);
  owner.ReturnToOwner();
}

// Enumerate objects of type Y by invoking Troutine as a coroutine
template<typename Y>
class Enumerator
{
private:
  struct AutoDone
  {
    bool &done;
    AutoDone(bool &done)
      : done(done)
    {
    }
    ~AutoDone()
    {
      done = true;
    }
  };

public:


private:
  typedef typename std::function<void(YieldBuffer<Y>&)> RoutineType;
  YieldBuffer<Y> buffer;
  RoutineType routine;
  bool done;
  Task workerTask, selfTask;

  // Thunk converts C interface back to object reference
  static void StartupThunk(void *a, void *, void *, void *)
  {
    Enumerator &self = *reinterpret_cast<Enumerator*>(a);
    AutoDone autodone(self.done);
    self.routine(self.buffer);
    self.ReturnToOwner();
  }

  template<typename R>
  Enumerator(R &&routine,
      typename std::enable_if<!std::is_convertible<R,RoutineType>::value>::type * = nullptr)
      = delete;
public:
  template<typename R>
  Enumerator(R &&routine,
      typename std::enable_if<std::is_convertible<R,RoutineType>::value>::type * = nullptr)
    : routine(std::forward<R>(routine))
    , done(false)
    , buffer(*this)
    , workerTask(0)
    , selfTask(&workerTask)
  {
    // Create a task for the routine
    // We pass a function pointer to the compiler generated thunk for this yield type
    workerTask.CreateContext(StartupThunk, nullptr, nullptr, nullptr, nullptr);
  }

  void ReturnToOwner()
  {
    selfTask.SwitchTo(workerTask);
  }

  void ReturnToCoroutine()
  {
    workerTask.SwitchTo(selfTask);
  }

  // Returns lvalue reference to buffer
  bool Next()
  {
    // Switch to the coroutine and execute
    // until it yields something or returns
    ReturnToCoroutine();
    return done;
  }

  Y &GetYield()
  {
    return buffer.buffer;
  }
};

void TestEnumeratorCoroutine(YieldBuffer<int> &out)
{
  int i = 1;
  do
  {
    i <<= 1;
    out.YieldReturn(i);
  }
  while (i < (1<<20));
};

void TestEnumerator()
{
  for (Enumerator<int> powersOfTwo(TestEnumeratorCoroutine); powersOfTwo.Next(); )
  {
    std::cout << powersOfTwo.GetYield() << std::endl;
  }
}

int main()
{
  TestEnumerator();
  return 0;
}
