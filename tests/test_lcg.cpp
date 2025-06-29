#include "diesel/random.h"

#include "test_shared.h"

// Runs the Diesel LCG three times with the same seed.
TEST_ENTRY(lcg) {
  diesel::DieselLCG lcg = diesel::DieselLCG();

  lcg.set_seed(0);
  { // Tests from the source of the LCG constants
    TEST_EQUAL(lcg.get_state(), 0x00000000);
    lcg.random();
    TEST_EQUAL(lcg.get_state(), 0x3C6EF35F);
    lcg.random();
    TEST_EQUAL(lcg.get_state(), 0x47502932);
    lcg.random();
    TEST_EQUAL(lcg.get_state(), 0xD1CCF6E9);
    lcg.random();
    TEST_EQUAL(lcg.get_state(), 0xAAF95334);
    lcg.random();
    TEST_EQUAL(lcg.get_state(), 0x6252E503);
    lcg.random();
    TEST_EQUAL(lcg.get_state(), 0x9F2EC686);
    lcg.random();
    TEST_EQUAL(lcg.get_state(), 0x57FE6C2D);
    lcg.random();
    TEST_EQUAL(lcg.get_state(), 0xA3D95FA8);
    lcg.random();
    TEST_EQUAL(lcg.get_state(), 0x81FDBEE7);
    lcg.random();
    TEST_EQUAL(lcg.get_state(), 0x94F0AF1A);
    lcg.random();
    TEST_EQUAL(lcg.get_state(), 0xCBF633B1);
  }


  lcg.set_seed(0);
  {
    TEST_DOUBLE_EQUAL(lcg.random(12), 3);
    TEST_DOUBLE_EQUAL(lcg.random(12), 4);
    TEST_DOUBLE_EQUAL(lcg.random(12), 10);
    TEST_DOUBLE_EQUAL(lcg.random(12), 9);
    TEST_DOUBLE_EQUAL(lcg.random(12), 5);
    TEST_DOUBLE_EQUAL(lcg.random(12), 8);
    TEST_DOUBLE_EQUAL(lcg.random(12), 5);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.64003562927246);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.50777810811996);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.58179754018784);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.7967255115509);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.73756557703018);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.61382895708084);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.88511604070663);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.0067492397502065);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.51475071907043);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.70773285627365);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.3116751909256);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.38401851058006);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.64467841386795);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 31);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 31);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 28);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 25);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 20);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 25);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 40);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 32);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 29);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 22);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 27);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 20);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 27);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 27);
  }
  lcg.set_seed(0);
  {
    TEST_DOUBLE_EQUAL(lcg.random(12), 3);
    TEST_DOUBLE_EQUAL(lcg.random(12), 4);
    TEST_DOUBLE_EQUAL(lcg.random(12), 10);
    TEST_DOUBLE_EQUAL(lcg.random(12), 9);
    TEST_DOUBLE_EQUAL(lcg.random(12), 5);
    TEST_DOUBLE_EQUAL(lcg.random(12), 8);
    TEST_DOUBLE_EQUAL(lcg.random(12), 5);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.64003562927246);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.50777810811996);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.58179754018784);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.7967255115509);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.73756557703018);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.61382895708084);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.88511604070663);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.0067492397502065);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.51475071907043);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.70773285627365);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.3116751909256);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.38401851058006);
    TEST_DOUBLE_EQUAL(lcg.random(), 0.64467841386795);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 31);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 31);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 28);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 25);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 20);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 25);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 40);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 32);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 29);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 22);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 27);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 20);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 27);
    TEST_DOUBLE_EQUAL(lcg.random(20, 40), 27);
  }

  TEST_SUCCESS;
}