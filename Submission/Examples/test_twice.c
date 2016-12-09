function twice(function f) {
  int g(int x) { return f(f(x)); }
  return g;
}

void main()
{
  int addten(int n) {return n + 10;}
  return twice(addten)(2);
}
