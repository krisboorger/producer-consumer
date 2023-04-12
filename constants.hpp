/* variables that determine the speed of particular processes */
const double X = 4.0;
// The whole program works well with X = 4.0, but deadlocks quickly with X = 2.0
const double Y = 30.0;
const double Z = 20.0;

/* length of each queue */
const int ORDERS_Q_LEN = 10;
const int TOOLS_Q_LEN = 2;
const int MASTERS_Q_LEN = 2;
const int PRODUCTS_Q_LEN = 10;

/* color codes to avoid using magic constants */
const auto COL_RESET = "\033[0m";
const auto COL_R   = "\033[31m";
const auto COL_Y   = "\033[33m";
const auto COL_G   = "\033[32m";
const auto COL_B   = "\033[34m";
