#include <bits/chrono.h>
#include <chrono>
#include <cstdlib>
#include <list>
#include <thread>

#include "constants.hpp"
#include "producer_consumer_utils.hpp"

using namespace std::chrono_literals;

void consumerThread(int id, const std::shared_ptr<Monitor<Order>> &orders,
                    Priority priority) {
  Consumer consumer(id, orders, priority);
  consumer.run();
};

void Consumer::run() {
  int counter = 0;
  std::srand(m_id);
  while (true) {
    m_orders->push(Order(10000 * m_id + counter, m_priority));
    counter++;
    double timeToSleep = (1.0 + rand() % 3) / 2 * X;
    std::this_thread::sleep_for(std::chrono::duration<double>(timeToSleep));
  };
};

void workerThread(const std::shared_ptr<Monitor<Order>> &orders,
                  const std::shared_ptr<Monitor<Tool>> &tools,
                  const std::shared_ptr<Monitor<Order>> &mastersWorkbench) {
  Worker worker(orders, tools, mastersWorkbench);
  worker.run();
};

void Worker::run() {
  while (true) {
    auto order = m_orders->pop();
    auto tool = m_tools->pop();
    auto finishedOrder = workOnWith(order, tool);
    m_tools->push(tool);
    m_mastersWorkbench->push(finishedOrder);
    std::this_thread::sleep_for(1000ms); // rest
  };
};

Order Worker::workOnWith(Order order, Tool tool) {
  std::chrono::duration<double> timeToSleep;
  switch (tool.getType()) {
  case ToolType::A:
    timeToSleep = 1000ms;
    break;
  case ToolType::B:
    timeToSleep = 2000ms;
    break;
  };
  if (order.getPriority() >= Priority::ret_standard)
    order.markAsFixed();
  std::this_thread::sleep_for(timeToSleep);
  return order;
};

void masterThread(const std::shared_ptr<Monitor<Order>> &orders,
                  const std::shared_ptr<Monitor<Tool>> &tools,
                  const std::shared_ptr<Monitor<Order>> &mastersWorkbench,
                  const std::shared_ptr<Monitor<Order>> &products) {
  Master master(orders, tools, mastersWorkbench, products);
  master.run();
};

void Master::run() {
  std::srand(2137);
  while (true) {
    auto order = m_mastersWorkbench->pop();
    std::this_thread::sleep_for(500ms);
    if (rand() % 10) {
      auto tool = m_tools->pop();
      std::this_thread::sleep_for(1000ms);
      m_tools->push(tool);
      m_products->push(order);
    } else {
      order.markAsFaulty();
      m_orders->push(order);
    };
  };
};

void supervisorThread(const std::shared_ptr<Monitor<Order>> &orders,
                      const std::shared_ptr<Monitor<Tool>> &tools,
                      const std::shared_ptr<Monitor<Order>> &products) {
  Supervisor supervisor(orders, tools, products);
  supervisor.run();
};

void Supervisor::run() {
  while (true) {
    watch();
    work();
    std::cout << COL_B << "Supervisor has finished his intervention.\n"
              << COL_RESET;
  };
};

void Supervisor::watch() {
  while (true) {
    std::this_thread::sleep_for(5s);
    auto nextOrder = m_orders->top();
    if (nextOrder.getTimeDiff() > std::chrono::duration<double>(Y))
      return;
  };
};

void Supervisor::work() {
  std::cout << COL_B << "Supervisor starts his work\n" << COL_RESET;
  bool lastIteration = false;
  while (true) {
    auto order = m_orders->pop();
    if (order.getTimeDiff() < std::chrono::duration<double>(Z))
      lastIteration = true;
    auto tool = m_tools->pop();
    std::this_thread::sleep_for(2s);
    m_tools->push(tool);
    m_products->push(order);
    if (lastIteration)
      return;
    std::this_thread::sleep_for(1s);
  };
};

void deliveryThread(const std::shared_ptr<Monitor<Order>> &products) {
  while (true) {
    auto product = products->pop();
    std::cout << "Finished the order " << product.getId() << " in "
              << product.getTimeDiff() << "!\n";
    std::this_thread::sleep_for(500ms);
  };
};

void dataWatcher(const std::shared_ptr<Monitor<Order>> &orders,
                 const std::shared_ptr<Monitor<Tool>> &tools,
                 const std::shared_ptr<Monitor<Order>> &mastersWorkbench,
                 const std::shared_ptr<Monitor<Order>> &products) {
  while (true) {
    std::cout << "============================\n";
    std::cout << "Orders queue:\n";
    for (auto v : *orders) {
      switch (v.getPriority()) {
      case Priority::standard:
        break;
      case Priority::premium:
        std::cout << COL_G;
        break;
      case Priority::ret_standard:
        std::cout << COL_Y;
        break;
      case Priority::ret_premium:
        std::cout << COL_R;
        break;
      };
      std::cout << v.getId();
      std::cout << COL_RESET << " ";
    };
    std::cout << std::endl;

    std::cout << "Tools queue:\n";
    for (auto v : *tools)
      std::cout << v.getId() << " ";
    std::cout << std::endl;

    std::cout << "Master's workbench:\n";
    for (auto v : *mastersWorkbench)
      std::cout << v.getId() << " ";
    std::cout << std::endl;

    std::cout << "Products queue:\n";
    for (auto v : *products)
      std::cout << v.getId() << " ";
    std::cout << std::endl;

    std::this_thread::sleep_for(500ms);
  };
};
