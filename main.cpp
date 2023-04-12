#include <iostream>
#include <thread>

#include "constants.hpp"
#include "producer_consumer_utils.hpp"

int main() {
  auto orders = std::make_shared<Monitor<Order>>(ORDERS_Q_LEN);

  auto tools = std::make_shared<Monitor<Tool>>(TOOLS_Q_LEN);
  tools->push(Tool(ToolType::A));
  tools->push(Tool(ToolType::B));

  auto mastersWorkbench = std::make_shared<Monitor<Order>>(MASTERS_Q_LEN);
  auto products = std::make_shared<Monitor<Order>>(PRODUCTS_Q_LEN);

  std::jthread watcher(dataWatcher, orders, tools, mastersWorkbench, products);
  std::jthread consumer1(consumerThread, 1, orders, Priority::standard);
  std::jthread consumer2(consumerThread, 2, orders, Priority::standard);
  std::jthread premiumConsumer1(consumerThread, 11, orders, Priority::premium);
  /* std::jthread premiumConsumer2(consumerThread, 12, orders, Priority::premium); */
  std::jthread worker1(workerThread, orders, tools, mastersWorkbench);
  std::jthread worker2(workerThread, orders, tools, mastersWorkbench);
  std::jthread master(masterThread, orders, tools, mastersWorkbench, products);
  std::jthread supervisor(supervisorThread, orders, tools, products);
  std::jthread delivery(deliveryThread, products);

};
