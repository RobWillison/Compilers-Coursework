int two()
{
  return 2;
}

int one()
{
  return 1;
}

int three()
{
  return 1;
}

int main()
{
  int x = one() * two();
  int y = one() * two();

  return x + y;
}
