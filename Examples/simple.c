int test(int x)
{
  int test2() {return x;}
  return test2;
}

int main()
{
  return test()(60);
}
