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

static void dumpSymbol(const Symbol & sym)
{
  std::cout << "  " << sym.getName();
  if (sym.isWeakDefined())
  {
    std::cout << " (weak)";
  }
  if (sym.isThreadLocalValue())
  {
    std::cout << " (thread local)";
  }
  std::cout << std::endl;
}

static void dump(const std::string & filename,
  const std::string & arch, cpu_type_t cpuType, cpu_subtype_t cpuSubType)
{
  std::cout << "==== " << filename << " " << arch << std::endl;

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
    CpuSubTypeMatching::Exact, minOSVersion, errorMessage);

  if (errorMessage.size())
  {
    std::cout << "Failed to parse: " << errorMessage << std::endl;
    std::cout << std::endl;
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

  // Note: would be nice to show the actual name of the platform
  std::cout << "platform: " << (unsigned)file->getPlatform() << std::endl;

  std::cout << "exports: " << std::endl;
  for (const Symbol & sym : file->exports())
  {
    dumpSymbol(sym);
  }

  std::cout << std::endl;

  delete file;
}

static void dumpAsEveryArch(const std::string & filename)
{
  dump(filename, "x86_64", CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_ALL);
  dump(filename, "x86_64h", CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_H);
  dump(filename, "i386", CPU_TYPE_X86_64, CPU_SUBTYPE_I386_ALL);
}

int main(int argc, char ** argv)
{
  for (int i = 1; i < argc; i++)
  {
    dumpAsEveryArch(argv[i]);
  }
}
