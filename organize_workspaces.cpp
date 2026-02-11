// cd ~/projects/hypr-workspaces-tools
// g++ -O2 -std=c++20 -o organize_workspaces organize_workspaces.cpp -ljsoncpp
// ./organize_workspaces
// Intended for use with a Firefox extension that prefixes each window title
// with "[N]" where N is the target Hyprland workspace number.
//
// Example title: "[2] ACC 214 notes - Mozilla Firefox"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include <jsoncpp/json/json.h>

static std::string exec_cmd(const char* cmd) {
  std::array<char, 256> buffer{};
  std::string result;

  std::unique_ptr<FILE, int (*)(FILE*)> pipe(popen(cmd, "r"), pclose);
  if (!pipe) throw std::runtime_error("popen() failed");

  while (fgets(buffer.data(), (int)buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

static bool parse_workspace_prefix(const std::string& title, int& ws_out) {
  // Expect: "[" digits "]" ...
  if (title.size() < 3) return false;
  if (title[0] != '[') return false;

  const size_t close = title.find(']');
  if (close == std::string::npos) return false;
  if (close <= 1) return false; // "[]" or "[ ]" not allowed

  // Validate all characters between '[' and ']' are digits
  for (size_t i = 1; i < close; i++) {
    const char c = title[i];
    if (c < '0' || c > '9') return false;
  }

  try {
    ws_out = std::stoi(title.substr(1, close - 1));
  } catch (...) {
    return false;
  }

  // Optional: reject workspace 0 or negative
  if (ws_out <= 0) return false;

  return true;
}

int main() {
  const std::string jsonString = exec_cmd("hyprctl clients -j 2>/dev/null");

  Json::Value root;
  Json::CharReaderBuilder builder;
  std::string errs;

  {
    std::istringstream iss(jsonString);
    if (!Json::parseFromStream(builder, iss, &root, &errs)) {
      throw std::runtime_error("failed to parse json: " + errs);
    }
  }

  if (!root.isArray()) return 0;

  std::ostringstream batch;
  bool any = false;

  for (const auto& item : root) {
    if (!item.isObject()) continue;
    const std::string klass = item.get("class", "").asString();
    if (klass != "firefox") continue;

    const std::string title = item.get("title", "").asString();
    const std::string addr  = item.get("address", "").asString();
    if (addr.empty()) continue;

    int ws = 0;
    if (!parse_workspace_prefix(title, ws)) continue;

    // Build batch: dispatch movetoworkspacesilent <ws>,address:<addr>;
    batch << "dispatch movetoworkspacesilent " << ws << ",address:" << addr << "; ";
    any = true;
  }

  if (!any) return 0;

  // Use single quotes around batch to avoid needing to escape most characters.
  // (Batch only contains our own "dispatch ..." strings; no window titles included.)
  std::string cmd = "hyprctl --batch '" + batch.str() + "' >/dev/null 2>&1";
  std::system(cmd.c_str());

  return 0;
}
