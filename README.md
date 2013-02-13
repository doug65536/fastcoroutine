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

    template<typename Y>
    void Test(void (*coroutine)(YieldBuffer<Y>&))
    {
      for (Enumerator<int> enumerator(coroutine); enumerator.Next(); )
      {
        std::cout << enumerator.GetYield() << std::endl;
      }
    }
