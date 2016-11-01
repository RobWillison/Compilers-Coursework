int test()
{
  int x = 6;
  int test2() {return x;}
  return test2();
}

int main()
{
  return test();
}
