#include "network.h"
#include "rt/rt_api.h"
//#include "/home/hayate/pulpissimo/pulp-sdk/test_code/app_10layer_angelo/inc/hyperbus_test.h"


// on fabric controller
int main () {
  	network_setup();
  	network_run_FabricController();
}
