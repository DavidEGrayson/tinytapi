// Utility that uses libtapi to read linker interface files and dump all the
// information about them to the standard output.  Useful for testing libtapi
// implementations.

#include <tapi.h>

#include <string.h>

#include <iostream>
#include <fstream>
#include <vector>

using namespace tapi;

// From Apple's mach/machine.h
#define CPU_ARCH_ABI64 0x1000000
#define CPU_TYPE_I386 ((cpu_type_t)7)
#define CPU_TYPE_X86_64 ((cpu_type_t)(CPU_TYPE_I386 | CPU_ARCH_ABI64))
#define CPU_SUBTYPE_I386_ALL ((cpu_subtype_t)3)
#define CPU_SUBTYPE_X86_64_ALL CPU_SUBTYPE_I386_ALL
#define CPU_SUBTYPE_X86_64_H ((cpu_subtype_t)8)

static void dump(const std::string & filename) {
  std::cout << "---- " << filename << std::endl;

  cpu_type_t cpuType = CPU_TYPE_X86_64;
  cpu_subtype_t cpuSubType = CPU_SUBTYPE_X86_64_ALL;
  PackedVersion32 minOSVersion(0x1011);  // TODO: is that the correct format?

  std::ifstream stream(filename);
  if (!stream) {
    int error_code = errno;
    std::cerr << "Error: " << strerror(error_code) << std::endl;
    exit(1);
  }
  std::vector<uint8_t> data = {
    std::istreambuf_iterator<char>(stream),
    std::istreambuf_iterator<char>()
  };
  if (stream.fail()) {
    std::cerr << "Error: failed to read." << std::endl;
    exit(1);
  }

  bool supported = LinkerInterfaceFile::isSupported(filename,
    data.data(), data.size());
  if (!supported)
  {
    std::cout << "isSupported = false" << std::endl;
  }

  std::string errorMessage;

  LinkerInterfaceFile * file = LinkerInterfaceFile::create(filename,
    data.data(), data.size(), cpuType, cpuSubType,
    CpuSubTypeMatching::ABI_Compatible, minOSVersion, errorMessage);

  if (errorMessage.size())
  {
    std::cerr << "Failed to parse: " << errorMessage << std::endl;
    if (file != NULL)
    {
      std::cerr <<
        "Error: returned pointer is not null!" << std::endl;
      exit(1);
    }
    return;  // Continue: this isn't necessarily a bug.
  }
  if (file == NULL)
  {
    std::cerr << "Got a null pointer but no error message." << std::endl;
    exit(1);
  }

  std::cout << "install-name: " << file->getInstallName() << std::endl;

  delete file;
}

int main(int argc, char ** argv) {
  for (int i = 1; i < argc; i++) {
    dump(argv[i]);
    std::cout << std::endl;
  }
}
