#pragma once

#include <stdint.h>
#include <vector>
#include <string>

using cpu_type_t = int;
using cpu_subtype_t = int;

namespace tapi {

class APIVersion {
public:
  static unsigned getMajor() noexcept;
};

class Version {
public:
  static std::string getFullVersionAsString();
};

class PackedVersion32 {
  uint32_t version;
public:
  PackedVersion32() = default;
  PackedVersion32(uint32_t v) : version(v) {}
};

enum class ObjCConstraint {
  None,
  Retain_Release,
  Retain_Release_For_Simulator,
  Retain_Release_Or_GC,
  GC,
};

enum class Platform {
  Unknown,
  OSX,
  iOS,
  watchOS,
  tvOS
};

enum class CpuSubTypeMatching {
  Exact,
  ABI_Compatible
};

class Symbol {
  std::string name;
public:
  const std::string & getName() const noexcept { return name; }
  bool isWeakDefined() const noexcept;
  bool isThreadLocalValue() const noexcept;
};

class LinkerInterfaceFile {
public:
  static LinkerInterfaceFile * create(const std::string & path,
    const uint8_t * data, size_t size,
    cpu_type_t cpuType, cpu_subtype_t cpuSubType,
    CpuSubTypeMatching matchingMode, PackedVersion32 minOSVersion,
    std::string & errorMessage) noexcept;

  static bool isSupported(const std::string & path,
    const uint8_t * data, size_t size) noexcept;

  static bool shouldPreferTextBasedStubFile(const std::string & path)
    noexcept;

  static bool areEquivalent(const std::string & tbdPath,
    const std::string & dylibPath) noexcept;

  const std::vector<Symbol> & exports() const noexcept;
};

}  // end namespace tapi
