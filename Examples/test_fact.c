int fact(int n) {
    int inner_fact(int n, int a) {
      if (n==0) return a;
        return inner_fact(n-1,a*n);
      }
    return inner_fact(n,1);
}


int main()
{
  return fact(1);
}
