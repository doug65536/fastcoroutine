fastcoroutine
=============

Extremely lightweight x86-64 coroutine implementation.

Currently implements an enumerator style coroutine interface:

    void TestEnumeratorCoroutine(YieldBuffer<int> &out)
    {
      int i = 1;
      do
      {
        i <<= 1;
        out.YieldReturn(i);
      }
      while (i < (1<<20));
    }

    void Test()
    {
      for (Enumerator<int> enumerator(TestEnumeratorCoroutine); enumerator.Next(); )
      {
        std::cout << enumerator.GetYield() << std::endl;
      }
    }

A coroutine allows you to "return" a value in the middle of a function,
then continue execution and possibly return an unlimited number of values.
The term *yield* used to describe this special type of returning a value.

Your coroutine must have one parameter of type `YieldBuffer<T> &`, where T
is the type of object yielded from the enumerator.

Constructing an `Enumerator<T>` prepares the coroutine for execution, but
does not start executing it.

The `Next()` method transfers control to the coroutine, and execution
continues until a value is yielded or the coroutine returns.

When a value is yielded the coroutine context is saved, the owner's
context is restored, and the call to `Next()` returns.

The return value from `Next()` is designed for use in `for` loops. It returns
true if there may be more objects yielded, and false when the coroutine
has completed.

If the last call to `Next()` returned true, then calling `GetYield()` will
return a reference to the last object yielded by the coroutine.

If you destruct an Enumerator<T> before the coroutine runs to completion,
control will be transferred back to the coroutine, and a
`CoroutineCanceled` exception will be thrown. This exception will be caught
and handled by this library, and control will be returned to the
`Enumerator<T>` destructor.
