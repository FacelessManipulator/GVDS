/*
 * @Author: Hanjie,Zhou
 * @Date: 2020-02-20 00:37:52
 * @Last Modified by:   Hanjie,Zhou
 * @Last Modified time: 2020-02-20 00:37:52
 */
#pragma once

enum TransactionStatus {
  UNINITILIZED = 1,
  PREPARED,
  SUBMITED,
  FORGOT,
  ABORTED
};

// the basic distributed transaction with two phase commit shold impl. the
// following interface.
class BasicTransaction {
 public:
  virtual int prepare() = 0;
  virtual int submit() = 0;
  virtual int forget() = 0;
  virtual int abort() = 0;

 public:
  TransactionStatus status;
};

// GVDS trasaction need provide the filesystem interface for fuse request.
class GVDSTransaction {
 public:
  int lookup();
};