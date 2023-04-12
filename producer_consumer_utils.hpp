#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <ostream>
#include <ranges>
#include <vector>

template <class T> class Monitor {
  using guard_t = std::lock_guard<std::mutex>;
  using lock_t = std::unique_lock<std::mutex>;

public:
  explicit Monitor(int queueLen)
      : m_buffer(std::vector<T>{}), m_len(queueLen){};
  void push(T item);
  T pop();
  T top();

  auto begin() const { return m_buffer.begin(); };
  auto end() const { return m_buffer.end(); };

  int getCapacity() const { return m_capacity; };
  void reserveSpace() { m_capacity--; };
  void freeSpace() { m_capacity++; };

private:
  std::vector<T> m_buffer;
  lock_t m_lock{};
  std::mutex m_mtx;
  const int m_len;
  int m_capacity = m_len;
  std::condition_variable cnd_freeSpace{};
  std::condition_variable cnd_notEmpty{};
};


template <class T> T Monitor<T>::top() {
  auto cmp = [](const T &lhs, const T &rhs) { return lhs < rhs; };
  lock_t lck{m_mtx};
  auto maxPriority = std::max_element(m_buffer.begin(), m_buffer.end(), cmp);
  auto poppedItem = *maxPriority;
  return poppedItem;
};


template <class T> T Monitor<T>::pop() {
  auto cmp = [](const T &lhs, const T &rhs) { return lhs < rhs; };
  lock_t lck{m_mtx};
  cnd_notEmpty.wait(lck, [&] { return this->getCapacity() < this->m_len; });
  auto maxPriority = std::max_element(m_buffer.begin(), m_buffer.end(), cmp);
  auto poppedItem = *maxPriority;
  m_buffer.erase(maxPriority);
  freeSpace();
  cnd_freeSpace.notify_all();
  return poppedItem;
};

template <class T> void Monitor<T>::push(T item) {
  lock_t lck{m_mtx};
  cnd_freeSpace.wait(lck, [&] { return this->getCapacity() > 0; });
  m_buffer.push_back(item);
  reserveSpace();
  cnd_notEmpty.notify_all();
  return;
};

enum Priority { standard = 1, premium, ret_standard, ret_premium };

class Item {

public:
  Item(int id, Priority priority) : m_id(id), m_priority(priority){};
  void setId(int id) { this->m_id = id; };
  constexpr int getId() const { return this->m_id; };
  void setPriority(Priority priority) { this->m_priority = priority; };
  constexpr Priority getPriority() const { return this->m_priority; };

  const auto operator<=>(const Item &rhs) const {
    return getPriority() <=> rhs.getPriority();
  };
  constexpr auto operator==(const Item &rhs) {
    return getPriority() <=> rhs.getPriority() == 0;
  };

private:
  int m_id;
  Priority m_priority;
};

class Order : public Item {
public:
  Order(int id, Priority priority)
      : Item(id, priority), m_creationTime(std::chrono::system_clock::now()){};
  auto getTimeDiff() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now() - m_creationTime);
  };
  void markAsFaulty() {
    setPriority(static_cast<Priority>(getPriority() + 2));
  };
  void markAsFixed() { setPriority(static_cast<Priority>(getPriority() - 2)); };

private:
  std::chrono::time_point<std::chrono::system_clock> m_creationTime;
};

enum ToolType { A = 1, B };

class Tool : public Item {
public:
  explicit Tool(ToolType type) : Item(type, Priority::standard){};
  ToolType getType() { return static_cast<ToolType>(getId()); };
};

void consumerThread(int id, const std::shared_ptr<Monitor<Order>> &orders,
                    Priority priority);
class Consumer {
public:
  Consumer(int id, std::shared_ptr<Monitor<Order>> orders, Priority priority)
      : m_id(id), m_orders(orders), m_priority(priority){};
  void run();

private:
  int m_id;
  std::shared_ptr<Monitor<Order>> m_orders;
  Priority m_priority;
};

void workerThread(const std::shared_ptr<Monitor<Order>> &orders,
                  const std::shared_ptr<Monitor<Tool>> &tools,
                  const std::shared_ptr<Monitor<Order>> &mastersWorkbench);

class Worker {
public:
  Worker(std::shared_ptr<Monitor<Order>> orders,
         std::shared_ptr<Monitor<Tool>> tools,
         std::shared_ptr<Monitor<Order>> mastersWorkbench)
      : m_orders(orders), m_tools(tools),
        m_mastersWorkbench(mastersWorkbench){};
  void run();
  Order workOnWith(Order order, Tool tool);

protected:
  std::shared_ptr<Monitor<Order>> m_orders;
  std::shared_ptr<Monitor<Tool>> m_tools;
  std::shared_ptr<Monitor<Order>> m_mastersWorkbench;
};

void masterThread(const std::shared_ptr<Monitor<Order>> &orders,
                  const std::shared_ptr<Monitor<Tool>> &tools,
                  const std::shared_ptr<Monitor<Order>> &mastersWorkbench,
                  const std::shared_ptr<Monitor<Order>> &products);

class Master : Worker {
public:
  Master(std::shared_ptr<Monitor<Order>> orders,
         std::shared_ptr<Monitor<Tool>> tools,
         std::shared_ptr<Monitor<Order>> mastersWorkbench,
         std::shared_ptr<Monitor<Order>> products)
      : Worker(orders, tools, mastersWorkbench), m_products(products){};
  void run();

private:
  std::shared_ptr<Monitor<Order>> m_products;
};

void supervisorThread(const std::shared_ptr<Monitor<Order>> &orders,
                      const std::shared_ptr<Monitor<Tool>> &tools,
                      const std::shared_ptr<Monitor<Order>> &products);

class Supervisor {
public:
  Supervisor(std::shared_ptr<Monitor<Order>> orders,
             std::shared_ptr<Monitor<Tool>> tools,
             std::shared_ptr<Monitor<Order>> products)
      : m_orders(orders), m_tools(tools), m_products(products){};
  void run();
  void watch();
  void work();

private:
  std::shared_ptr<Monitor<Order>> m_orders;
  std::shared_ptr<Monitor<Tool>> m_tools;
  std::shared_ptr<Monitor<Order>> m_products;
};

void deliveryThread(const std::shared_ptr<Monitor<Order>> &products);

void dataWatcher(const std::shared_ptr<Monitor<Order>> &orders,
                 const std::shared_ptr<Monitor<Tool>> &tools,
                 const std::shared_ptr<Monitor<Order>> &mastersWorkbench,
                 const std::shared_ptr<Monitor<Order>> &products);
