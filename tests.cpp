// Copyright 2013 A. Douglas Gale
// Permission is granted to use the fastcoroutine implementation
// for any use (including commercial use), provided this copyright
// notice is present in your product's source code.

// tests.cpp

#include "fastcoroutine.h"
#include <iostream>

using namespace FastCoroutine;

void EmptyCoroutine(YieldBuffer<int> &)
{
}

void TypicalCoroutine(YieldBuffer<int> &out)
{
  int i = 1;
  do
  {
    i <<= 1;
    out.YieldReturn(i);
  }
  while (i < (1<<20));
}

void ThrowCatchCoroutine(YieldBuffer<int> &)
{
  try
  {
    throw std::logic_error("Expected exception for testing");
  }
  catch (const std::logic_error &e)
  {
    std::cout << "Exception caught" << std::endl;
  }
}

void YieldFloatsCoroutine(YieldBuffer<float> &out)
{
  out.YieldReturn(1.0f);
  out.YieldReturn(2.0f);
}

void NestedCoroutine(YieldBuffer<int> &out)
{
  for (Enumerator<float> enumerator(YieldFloatsCoroutine); enumerator.Next(); )
  {
    out.YieldReturn((int)enumerator.GetYield());
  }
}

template<typename Y>
void TestRunToCompletion(void (*coroutine)(YieldBuffer<Y>&))
{
  for (Enumerator<Y> enumerator(coroutine); enumerator.Next(); )
  {
    std::cout << enumerator.GetYield() << std::endl;
  }
}

template<typename Y>
void TestAbandon(void (*coroutine)(YieldBuffer<Y>&))
{
  for (Enumerator<Y> enumerator(coroutine); enumerator.Next(); )
  {
    std::cout << enumerator.GetYield() << std::endl;
    break;
  }
}

template<typename Y>
void Test(void (*coroutine)(YieldBuffer<Y>&))
{
  TestRunToCompletion(coroutine);
  TestAbandon(coroutine);
}

int main()
{
  Test(EmptyCoroutine);
  Test(TypicalCoroutine);
  Test(YieldFloatsCoroutine);
  Test(NestedCoroutine);
  return 0;
}
