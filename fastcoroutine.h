// Copyright 2013 A. Douglas Gale
// Permission is granted to use the fastcoroutine implementation
// for any use (including commercial use), provided this copyright
// notice is present in your product's source code.

#include <cstdint>
#include <stdexcept>
#include <functional>

namespace FastCoroutine
{
  struct SavedContextFrame;

  class Task
  {
    enum State
    {
      INVALID,
      ROOT,
      CREATED,
      EXITED,
    };

    SavedContextFrame *rsp;
    Task *owner;
    void *stack;
    size_t stacksize;
    State state;

  public:

    Task(Task *owner);

    ~Task();

  private:

    void GetTibStackRange();
    void SetTibStackRange();

  public:

    void CreateContext(void (*f)(void*,void*,void*,void*),
      void *a0 = nullptr, void *a1 = nullptr,
      void *a2 = nullptr, void *a3 = nullptr);

    void SwitchTo(Task &outgoing_task);
  };

  class CoroutineCanceled
    : public std::exception
  {
  public:
    CoroutineCanceled();
    const char *what();
  };

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

  // Enumerate objects of type Y by invoking Troutine as a coroutine
  template<typename Y>
  class Enumerator
  {
  private:
    typedef typename std::function<void(YieldBuffer<Y>&)> RoutineType;
    YieldBuffer<Y> buffer;
    RoutineType routine;
    bool started, done, cancel;
    Task workerTask, selfTask;

    // Thunk converts C interface back to object reference
    static void StartupThunk(void *a, void *, void *, void *);

    template<typename R>
    Enumerator(R &&routine,
      typename std::enable_if<
        !std::is_convertible<R,RoutineType>::value
      >::type * = nullptr)
      = delete;
  public:
    template<typename R>
    Enumerator(R &&routine,
      typename std::enable_if<
        std::is_convertible<R,RoutineType>::value
      >::type * = nullptr);

    ~Enumerator();

    void ReturnToOwner();

    void ReturnToCoroutine();

    // Returns true if the coroutine is NOT finished
    bool Next();

    Y &GetYield();
  };
}

// ============================================================================
// YieldBuffer<T>
// ============================================================================

template<typename Y>
FastCoroutine::YieldBuffer<Y>::YieldBuffer(Enumerator<Y> &owner)
  : owner(owner)
{
}

template<typename Y> template<typename R>
typename std::enable_if<std::is_convertible<R,Y>::value>::type
FastCoroutine::YieldBuffer<Y>::YieldReturn(R &&result)
{
  buffer = std::forward<R>(result);
  owner.ReturnToOwner();
}

// ============================================================================
// Enumerator<Y>
// ============================================================================

// Thunk converts C interface back to object reference
template<typename Y>
void FastCoroutine::Enumerator<Y>::StartupThunk(void *a, void *, void *, void *)
{
  Enumerator &self = *reinterpret_cast<Enumerator*>(a);
  try
  {
    self.routine(self.buffer);
  }
  catch (CoroutineCanceled)
  {
  }
  self.done = true;
  self.ReturnToOwner();
}

template<typename Y>
template<typename R>
FastCoroutine::Enumerator<Y>::Enumerator(R &&routine,
  typename std::enable_if<
    std::is_convertible<R,RoutineType>::value
  >::type *)
  : buffer(*this)
  , routine(std::forward<R>(routine))
  , started(false)
  , done(false)
  , cancel(false)
  , workerTask(0)
  , selfTask(&workerTask)
{
  // Create a task for the routine
  // We pass a function pointer to the compiler generated thunk for this yield type
  workerTask.CreateContext(StartupThunk, this, nullptr, nullptr, nullptr);
}

template<typename Y>
FastCoroutine::Enumerator<Y>::~Enumerator()
{
  if (!done && started)
  {
    // Need to force coroutine to exit

    // Setting the cancel flag causes an exception to be thrown
    // in the coroutine context.
    cancel = true;

    // The CoroutineCanceled exception is caught and execution is
    // resumed and the ReturnToCoroutine returns
    ReturnToCoroutine();
  }
}

template<typename Y>
void FastCoroutine::Enumerator<Y>::ReturnToOwner()
{
  selfTask.SwitchTo(workerTask);

  if (cancel)
    throw CoroutineCanceled();
}

template<typename Y>
void FastCoroutine::Enumerator<Y>::ReturnToCoroutine()
{
  workerTask.SwitchTo(selfTask);
}

// Returns true if the coroutine is NOT finished
template<typename Y>
bool FastCoroutine::Enumerator<Y>::Next()
{
  // Switch to the coroutine and execute
  // until it yields something or returns
  ReturnToCoroutine();
  return !done;
}

template<typename Y>
Y &FastCoroutine::Enumerator<Y>::GetYield()
{
  return buffer.buffer;
}

