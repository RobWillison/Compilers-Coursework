int times2(int n) {
  int times(int n, int m) {
    return n * m;
  }
  return times(n, 2);
}

int times(int n)
{
  return n;
}

int main()
{
  return times2(3);
}
