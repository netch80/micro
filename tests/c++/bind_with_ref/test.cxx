#include <iostream>
#include <boost/bind.hpp>
#include <functional>

class Moo {
public:
  Moo() {
    std::cout << "Moo(): " << (void*)this << std::endl;
  }
  Moo(const Moo& other) {
    std::cout << "Moo(copy): " << (void*)this << " from " << (void*)(&other) << std::endl;
  }
  Moo(Moo&& other) {
    std::cout << "Moo(move): " << (void*)this << " from " << (void*)(&other) << std::endl;
  }
  ~Moo() {
    std::cout << "~Moo(): " << (void*)this << std::endl;
  }
  Moo& operator=(const Moo&) = delete;
  Moo& operator=(Moo&&) = delete;
};

using CB = std::function<void(void)>;
CB cbvar;

void cbfunc(const Moo& rm) {
  std::cout << "cbfunc(): &rm=" << (void*)(&rm) << std::endl;
}

void make_closure(const Moo& rm)
{
  std::cout << "make_closure(" << (void*)(&rm) << ")" << std::endl;
#if defined(USE_LAMBDA_A)
  cbvar = [rm=rm]() { cbfunc(rm); };
#elif defined(USE_LAMBDA)
  cbvar = [rm]() { cbfunc(rm); };
#elif defined(USE_LAMBDA_R)
  cbvar = [&rm]() { cbfunc(rm); };
#elif defined(USE_BIND_REF)
  cbvar = std::bind(cbfunc, std::ref(rm));
#else
  cbvar = std::bind(cbfunc, rm);
#endif
}

int main()
{
  cbvar = [](){};
  {
    Moo m1{};
    const Moo& rm1 = m1;
    std::cout << "Making closure" << std::endl;
    make_closure(rm1);
  }
  std::cout << "Calling" << std::endl;
  cbvar();
  std::cout << "Finish" << std::endl;
}
