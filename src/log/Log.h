#pragma once

/***********************************************************
 * @file  Log.h
 * @brief 日志类文件，提供了日志组件的基础功能
 * @author 周汉杰
 * @version 0.0.1
 * @date 2018/3/1
 * @email miller_maxwell@buaa.edu.cn
 * @license GNU General Public License (GPL)
 *
 * 修改历史：
 * ----------------------------------------------
 * 日期     | 版本号  |   作者   |      描述
 * ----------------------------------------------
 * 2018/3/1 | 0.0.1  | 周汉杰   | 实现基础功能
 * ----------------------------------------------
 *
 *
 ***********************************************************/

#include <chrono>
#include <fstream>
#include <memory>
#include "common/Thread.h"

#include <queue>

namespace hvs {
class Entry;

/// 日志条目类Entry的智能指针
using EntryPtr = std::shared_ptr<Entry>;

/// 日志类，负责提供日志记录接口
/**
 * @author: 周汉杰
 * @date: 2018/3/1
 *
 * 日志类Log的设计模式是单例模型，在整个进程中只有一个Log对象，目前暂时由hvs::HvsContext记录单例对象的指针。
 * 日志类Log继承了线程类Thread，实现了入口函数，为其他线程提供打印日志的接口。
 * 日志类提供多线程以及日志等级的支持，其他线程提交的entry会被submit到Log的队列中。日志在本线程，只要queue非空，
 * 就会调用flush将queue中的entry刷入日志文件。如果queue为空，则会等待信号量直到有其他线程提交entry。
 *
 * 日志等级的设计是用来过滤日志。例如同样的日志打印代码`dout(20) << "DEBUG DATA"
 * << dendl;`，在日志收集等级小于20时， 并不会输出到日志文件中。又例如`dout(-1)
 * << "SYSTEM ERROR" <<
 * dendl;`，通常是系统无法忽略的致命错误，将会被打印到stderr中。
 *
 * 日志类的使用方式比较简单，在`#include
 * "debug.h"`以后，直接使用宏`dout(${DEBUG_LEVEL}) << "你的DEBUG信息" <<
 * dendl;`即可，也 可以多行使用。
 * ``` cpp
 * dout(10) << MyClassObj.name
 *          << MyClassObj.age
 *          << MyClassObj.dump()
 *          << dendl;
 * ```
 * 日志类会自动帮你添加上日期、时间、线程tid、debug level等信息，示例如下：
 * ```plain
 * 2019-03-04 20:32:48.874 7f4a8324e740 1 YOUR DEBUG DATA
 * 2019-03-04 20:32:48.875 7f4a8324e740 1 YOUR DEBUG DATA2
 * ```
 *
 * 测试用例可见
 */
class Log : private Thread {
  using log_lock = std::chrono::
      system_clock;  ///< 使用标准库的系统时钟作为时钟，可能会因为系统时间调整而造成日志时间错乱
  log_lock clock;  ///< 只在创建日志记录entry的时候用到，用来打时间戳timepoint

  pthread_mutex_t m_queue_mutex;  ///< 日志提交队列m_new的锁
  pthread_mutex_t
      m_flush_mutex;  ///< 日志flush的锁，用来互斥操作文件和冲刷日志的操作
  pthread_cond_t
      m_cond_loggers;  ///< 当日志队列满的时候用来让其他线程等待的信号量，可能会阻塞调用dout的线程
  pthread_cond_t
      m_cond_flusher;  ///< 当日志队列空的时候用来让日志线程等待的信号量

  pthread_t
      m_queue_mutex_holder;  ///< 用来记录队列锁的持有者，用来让其他线程判断自己持有log锁
  pthread_t
      m_flush_mutex_holder;  ///< 用来记录冲刷锁的持有者，用来让其他线程判断自己持有log锁

  std::queue<EntryPtr> m_new;  ///< 日志的记录队列
  int m_max_new;               ///< 日志队列的最大记录数

  std::string m_log_file;   ///< 日志的存放路径
  std::ofstream m_fstream;  ///< 日志文件的输出流

  int m_syslog_log;  ///< syslog级别的日志收集等级，默认为-2
  int m_stderr_log;  ///< stderr级别的日志收集等级，默认为-1

  std::string m_log_stderr_prefix;  ///< stderr的日志前缀

  char *m_log_buf;  ///< 日志的缓冲区，默认大小为65536，由宏MAX_LOG_BUF决定
  size_t m_log_buf_pos;  ///< 日志缓冲区的cursor

  bool m_stop;    ///< 日志线程是否应该退出
  int log_level;  ///< 普通日志的收集等级，默认为10

  /// 线程类入口函数entry
  /**
   * Log类继承了Thread类，是作为一个单独的线程执行的，而该线程的入口点就是entry函数。
   * entry函数直到log被调用stop为止，都会不断等待日志队列并将日志刷写到日志文件中。该
   * 函数只会被线程类所调用。
   */
  void *entry() override;

  /// 日志写入函数
  /**
   * @param what  写入源的指针
   * @param write_len 源的长度
   * @return void 本函数应该拦截并处理所有IO故障
   *
   * 该函数对日志文件的输出流的写入操作进行了包装，并应该执行一些错误处理。
   * 不过日志写入失败并不是严重的故障，因此只是通过stderr通知用户写入失败。而不会执行assert或者exit。
   */
  void _log_safe_write(const char *what, size_t write_len);

  /// 日志缓冲区刷写
  /**
   * 将logbuf全部刷入日志文件流中
   */
  void _flush_logbuf();

  /// 将日志条目刷入日志文件
  /**
   * @param q   存储日志条目的队列，通常就是log的m_new
   * 将日志条目刷入日志缓冲区，然后将日志缓冲区刷入日志文件。
   */
  void _flush(std::queue<EntryPtr> *q);

 public:
  Log();  ///< 初始化构造函数，其中申请了一片log_buf的空间，析构的时候注意释放
  ~Log()
      override;  ///< 析构函数，在析构前必须首先调用stop()，完成冲刷缓冲区、终止线程等操作

  void set_max_new(int n);  ///< 设置日志队列的最大长度
  void set_log_file(std::string fn);  ///< 设置日志文件的路径，最好是绝对路径
  void reopen_log_file();  ///< 打开或重新打开日志文件的输出流
  void set_log_stderr_prefix(const std::string &p);  ///< 设置stderr输出流的前缀

  void flush();  ///< _flush()的包装函数，会将当前队列的条目全部提交给_flush

  void set_syslog_level(int log);  ///< 设置syslog的日志收集等级，默认为-2
  void set_stderr_level(int log);  ///< 设置stderr的日志收集等级，默认为-1
  void set_log_level(
      int log);  ///< 设置普通的日志收集等级，默认为10，这意味着日志等级小于等于10的都会被收集到日志文件中

  /// 创建日志条目
  /**
   * @param level   日志等级
   * @param prefix  日志前缀
   * @return 新建日志条目的智能指针
   *
   * create_entry创建日志条目，自动传入时间戳和调用线程号。
   * 该函数通常被其他线程才用dout宏的形式调用。
   */
  EntryPtr create_entry(int level, const char *prefix = nullptr);

  void submit_entry(
      EntryPtr
          ep);  ///< 提交日志条目，该函数通常被其他线程才用dendl宏的形式调用

  void start();  ///< 启动log线程
  void stop();  ///< 终止log线程，会join直到entry函数完成最后一个flush的循环

  /// 工具函数，返回当前线程是否拥有log锁
  /**
   * is_inside_log_lock用来让线程判断自己是否拥有log锁，是否在打印log。该函数通常用在信号处理函数中，以防止死锁
   * @return 拥有锁的状态
   *    - false 本线程不持有log锁
   *    - true  本线程持有log锁
   */
  bool is_inside_log_lock();

  /// 工具函数，将时间戳格式化并追加到buffer后面
  /**
   * append_time将时间戳格式化并追加到buffer后面
   */
  static inline int append_time(const Log::log_lock::time_point &t, char *out,
                                size_t outlen);
};
}  // namespace hvs