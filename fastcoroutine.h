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
