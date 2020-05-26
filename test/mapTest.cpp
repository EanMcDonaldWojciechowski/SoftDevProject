//lang::CwC cahnges
#include "../src/dataframe/modified_dataframe.h"

int main(int argc, char** argv) {

  char* chunkKeyText = new char[1024];
  strcat(chunkKeyText, "main_0_0");
  Key *k1 = new Key(chunkKeyText, 0);
  Value *v1 = new Value("2}80}I}0}1}2}3}4}5}6}7}8}9}10}11}12}13}14}15}16}17}18}19}20}21}22}23}24}25}26}27}28}29}30}31}32}33}34}35}36}37}38}39}40}41}42}43}44}45}46}47}48}49}");

  char* chunkKeyText2 = new char[1024];
  strcat(chunkKeyText2, "RSP");
  Key *k2 = new Key(chunkKeyText2, 0);
  Value *v2 = new Value("50}51}52}53}54}55}56}57}58}59}60}61}62}63}64}65}66}67}68}69}70}71}72}73}74}75}76}77}78}79}");


  Hashmap *hm = new Hashmap();
  hm->put(k1, v1);
  hm->printall();
  hm->put(k2, v2);
  hm->printall();
  hm->remove(k2);
  hm->printall();

  std::cout << "COMPLETED" << "\n";


}
