#ifndef COMMIT_OP_H_
#define COMMIT_OP_H_

namespace bdm {

template <typename TResourceManager = ResourceManager<>>
class CommitOp {
 public:
  CommitOp() {}
  ~CommitOp() {}

  template <typename TSoContainer>
  void operator()(TSoContainer* sim_objects, uint16_t type_idx) {
    update_info_.emplace_back(sim_objects->Commit());
  }

  auto& GetUpdateInfo() const { return update_info_; }

 private:
  std::vector<std::unordered_map<uint32_t, uint32_t>> update_info_;
};

}  // namespace bdm

#endif  // COMMIT_OP_H_
