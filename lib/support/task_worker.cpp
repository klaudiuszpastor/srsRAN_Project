/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsran/support/executors/task_worker.h"
#include <future>

using namespace srsran;

template <concurrent_queue_policy Policy, concurrent_queue_wait_policy BlockingPolicy>
static unique_function<void()> make_blocking_pop_task(concurrent_queue<unique_task, Policy, BlockingPolicy>& queue)
{
  return [&queue]() {
    while (true) {
      if (not queue.pop_blocking([](const unique_task& task) { task(); })) {
        break;
      }
    }
    srslog::fetch_basic_logger("ALL").info("Task worker {} finished.", this_thread_name());
  };
}

task_worker::task_worker(std::string                      thread_name,
                         unsigned                         queue_size,
                         os_thread_realtime_priority      prio,
                         const os_sched_affinity_bitmask& mask) :
  pending_tasks(queue_size), t_handle(thread_name, prio, mask, make_blocking_pop_task(pending_tasks))
{
}

task_worker::~task_worker()
{
  stop();
}

void task_worker::stop()
{
  if (t_handle.running()) {
    pending_tasks.request_stop();
    t_handle.join();
  }
}

void task_worker::wait_pending_tasks()
{
  std::packaged_task<void()> pkg_task([]() { /* do nothing */ });
  std::future<void>          fut = pkg_task.get_future();
  push_task_blocking(std::move(pkg_task));
  // blocks for enqueued task to complete.
  fut.get();
}
