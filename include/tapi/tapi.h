#pragma once

#include <stdint.h>
#include <vector>
#include <string>

using cpu_type_t = int;
using cpu_subtype_t = int;

namespace tapi {

// Internally used class; ideally this wouldn't even be here.
struct StubData;

class APIVersion {
public:
  static unsigned getMajor() noexcept;
};

class Version {
public:
  static std::string getFullVersionAsString() noexcept;
};

class PackedVersion32 {
  uint32_t version;
public:
  PackedVersion32() = default;
  PackedVersion32(uint32_t v) : version(v) {}
  PackedVersion32(unsigned major, unsigned minor, unsigned patch) :
    version((major << 16) | (minor << 8) | patch) {}
  operator unsigned() const noexcept { return version; }

  unsigned int getMajor() const noexcept { return version >> 16; }
  unsigned int getMinor() const noexcept { return version >> 8 & 0xFF; }
  unsigned int getPatch() const noexcept { return version & 0xFF; }

  void setPatch(unsigned int v) noexcept
  {
    version = (version & 0xFFFFFF00) | (v & 0xFF);
  }
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
public:
  std::string name;
  bool weak = false;
  bool threadLocal = false;
  Symbol(const std::string & name) : name(name) { }
  const std::string & getName() const noexcept { return name; }
  bool isWeakDefined() const noexcept { return weak; }
  bool isThreadLocalValue() const noexcept { return threadLocal; }
};

class LinkerInterfaceFile {
  LinkerInterfaceFile() = default;

  Platform platform;
  std::string installName;
  std::vector<Symbol> exportList;
  PackedVersion32 currentVersion, compatVersion;

  void init(const StubData &, cpu_type_t, cpu_subtype_t,
    CpuSubTypeMatching, PackedVersion32 minOSVersion,
    std::string & errorMessage);

public:
  /** Let's hope we don't need any of these:
  LinkerInterfaceFile(const LinkerInterfaceFile &) noexcept = delete;
  LinkerInterfaceFile & operator=(const LinkerInterfaceFile &) noexcept = delete;

  LinkerInterfaceFile(LinkerInterfaceFile &&) noexcept;
  LinkerInterfaceFile & operator=(LinkerInterfaceFile &&) noexcept;

  ~LinkerInterfaceFile() noexcept;
  **/

  static LinkerInterfaceFile * create(const std::string & path,
    const uint8_t * data, size_t size, cpu_type_t, cpu_subtype_t,
    CpuSubTypeMatching, PackedVersion32 minOSVersion,
    std::string & errorMessage) noexcept;

  static bool isSupported(const std::string & path,
    const uint8_t * data, size_t size) noexcept;

  static bool shouldPreferTextBasedStubFile(const std::string & path)
    noexcept;

  static bool areEquivalent(const std::string & tbdPath,
    const std::string & dylibPath) noexcept;

  const std::string & getInstallName() const noexcept
  {
    return installName;
  }

  bool isInstallNameVersionSpecific() const noexcept
  {
    return false;
  }

  Platform getPlatform() const noexcept
  {
    return platform;
  }

  PackedVersion32 getCurrentVersion() const noexcept
  {
    return currentVersion;
  }

  PackedVersion32 getCompatibilityVersion() const noexcept
  {
    return compatVersion;
  }

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

  const std::vector<Symbol> & exports() const noexcept { return exportList; }

  const std::vector<Symbol> & undefineds() const noexcept;
};

}  // end namespace tapi
