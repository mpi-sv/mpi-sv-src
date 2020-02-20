#include <klee/Expr.h>
#include "mpise/commlog.h"
#include "mpise/commlogManager.h"
#include <glog/logging.h>
using namespace std;



/*prepare the information for modeling the Master-Slave Pattern
/*we need to do 4 things:
 * 1. count the energy(number of slaves, and repetitive patterns) in Master-Slave Pattern.
 * 2. locate the special recv in slave for modeling, and assign the energy to it
 * 3. locate the special send in slave for modeling
 * 4. locate the task allocation send in master for modeling
 *  5. the number of scheduled tasks and slaves are stored in the arguments
 *  */
void prepareMS(mpicommlog_ty mpicommlog, int& tasks, int& slaves);

/* dump the collected information for debugging*/
void dumpMS(mpicommlog_ty mpicommlog);
