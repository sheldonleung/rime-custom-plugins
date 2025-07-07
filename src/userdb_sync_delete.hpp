#ifndef USERDB_SYNC_DELETE_HPP_
#define USERDB_SYNC_DELETE_HPP_

#include <rime/common.h>
#include <rime/processor.h>

namespace rime {

class UserdbSyncDelete : public Processor {
 public:
  explicit UserdbSyncDelete(const Ticket& ticket) : Processor(ticket) {};

  ProcessResult ProcessKeyEvent(const KeyEvent& key_event) override;
};

}  // namespace rime
#endif

