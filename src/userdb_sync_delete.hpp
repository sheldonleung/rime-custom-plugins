#ifndef USERDB_SYNC_DELETE_HPP_
#define USERDB_SYNC_DELETE_HPP_

#include <rime/common.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/processor.h>

namespace rime {

class UserdbSyncDelete : public Processor {
 public:
  explicit UserdbSyncDelete(const Ticket& ticket) : Processor(ticket) {
    Context* context = engine_->context();
    update_connection_ = context->update_notifier().connect(
        [this](Context* ctx) { OnUpdate(ctx); });
  }

  virtual ~UserdbSyncDelete() { update_connection_.disconnect(); }

  ProcessResult ProcessKeyEvent(const KeyEvent& key_event) override;

 private:
  void OnUpdate(Context* ctx) {}

  connection update_connection_;
};

}  // namespace rime
#endif

