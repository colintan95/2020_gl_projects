#include "gfx_utils/resource/data_source.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace gfx_utils {

DataEntry DataSource::GetEntry(const std::string& name) const {
  return data_.at(name);
}

} // namespace gfx_utils