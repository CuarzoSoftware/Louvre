#ifndef SESSIONLOCKMANAGER_H
#define SESSIONLOCKMANAGER_H

#include <LSessionLockManager.h>

using namespace Louvre;

class SessionLockManager final : public LSessionLockManager {
 public:
  SessionLockManager(const void *params) : LSessionLockManager(params) {}

  bool lockRequest(LClient *) override { return true; }
  void stateChanged() override;
};

#endif  // SESSIONLOCKMANAGER_H
