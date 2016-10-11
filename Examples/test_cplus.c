function cplus(int a) {
  int cplusa(int b) { return a+b; }
  return cplusa;
}

int main()
{
  return cplus(5)(2);
}
