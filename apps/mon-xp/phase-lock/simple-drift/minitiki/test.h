#define TEST_PORT P1

#define _POUT(port) port ## OUT
#define POUT(...) _POUT(__VA_ARGS__)

void test()
{
  POUT(TEST_PORT) = 1;
}
