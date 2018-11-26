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
  operator unsigned() const noexcept { return version; }
};

enum class ObjCConstraint : unsigned {
  None = 0,
  Retain_Release = 1,
  Retain_Release_For_Simulator = 2,
  Retain_Release_Or_GC = 3,
  GC = 4,
};

enum class Platform : unsigned  {
  Unknown = 0,
  OSX = 1,
  iOS = 2,
  watchOS = 3,
  tvOS = 4,
  bridgeOS = 5,
};

enum class CpuSubTypeMatching : unsigned {
  ABI_Compatible = 0,
  Exact = 1,
};

class Symbol {
  std::string name;
public:
  const std::string & getName() const noexcept { return name; }
  bool isWeakDefined() const noexcept;
  bool isThreadLocalValue() const noexcept;
};

class LinkerInterfaceFile {
  LinkerInterfaceFile() = default;
public:
  /** Let's hope we don't need any of these:
  LinkerInterfaceFile(const LinkerInterfaceFile &) noexcept = delete;
  LinkerInterfaceFile & operator=(const LinkerInterfaceFile &) noexcept = delete;

  LinkerInterfaceFile(LinkerInterfaceFile &&) noexcept;
  LinkerInterfaceFile & operator=(LinkerInterfaceFile &&) noexcept;

  ~LinkerInterfaceFile() noexcept;
  **/

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

  const std::string & getInstallName() const noexcept;
  bool isInstallNameVersionSpecific() const noexcept;
  Platform getPlatform() const noexcept;
  PackedVersion32 getCurrentVersion() const noexcept;
  PackedVersion32 getCompatibilityVersion() const noexcept;
  unsigned getSwiftVersion() const noexcept;
  ObjCConstraint getObjCConstraint() const noexcept;
  const std::string & getParentFrameworkName() const noexcept;
  bool isApplicationExtensionSafe() const noexcept;
  bool hasTwoLevelNamespace() const noexcept;

  bool hasAllowableClients() const noexcept;
  const std::vector<std::string> & allowableClients() const noexcept;

  bool hasReexportedLibraries() const noexcept;
  const std::vector<std::string> & reexportedLibraries() const noexcept;

  bool hasWeakDefinedExports() const noexcept;

  const std::vector<std::string> & ignoreExports() const noexcept;

  const std::vector<Symbol> & exports() const noexcept;

  const std::vector<Symbol> & undefineds() const noexcept;
};

}  // end namespace tapi
