// Information about the architectures we support.

// Note: This is pretty minimal right now; there are a bunch of ARM
// architectures we might want to support in
// tapi/include/tapi/Core/Architecture.def or
// cctools/ld64/src/abstract/MachOFileAbstraction.hpp (archInfoArray[])

// From Apple's mach/machine.h
#define CPU_ARCH_ABI64 0x1000000
#define CPU_TYPE_I386 ((cpu_type_t)7)
#define CPU_TYPE_X86_64 ((cpu_type_t)(CPU_TYPE_I386 | CPU_ARCH_ABI64))
#define CPU_SUBTYPE_I386_ALL ((cpu_subtype_t)3)
#define CPU_SUBTYPE_X86_64_ALL CPU_SUBTYPE_I386_ALL
#define CPU_SUBTYPE_X86_64_H ((cpu_subtype_t)8)

struct ArchInfo
{
  const char * name;
  cpu_type_t cpuType;
  cpu_subtype_t cpuSubType;
};

// These serve as indices in the archInfoArray.
enum class Architecture
{
  None = 0,
  x86_64,
  x86_64h,
  i386,
};

static const ArchInfo archInfoArray[] = {
  { "none", 0, 0 },
  { "x86_64", CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_ALL },
  { "x86_64h", CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_H },
  { "i386", CPU_TYPE_I386, CPU_SUBTYPE_I386_ALL },
};

static const size_t archCount =
  sizeof(archInfoArray)/sizeof(archInfoArray[0]);

static const ArchInfo & getArchInfo(Architecture arch)
{
  size_t index = (size_t)arch;
  if (index >= archCount)
  {
    index = 0;
  }
  return archInfoArray[index];
}
static Architecture getCpuArch(cpu_type_t cpuType, cpu_subtype_t cpuSubType)
{
  for (size_t i = 1; i < archCount; i++)
  {
    Architecture arch = (Architecture)i;
    auto info = getArchInfo(arch);
    if (info.cpuType == cpuType && info.cpuSubType == cpuSubType)
    {
      return arch;
    }
  }
  return Architecture::None;
}

static Architecture getArchByName(const std::string & name)
{
  for (size_t i = 1; i < archCount; i++)
  {
    Architecture arch = (Architecture)i;
    auto info = getArchInfo(arch);
    if (info.name == name)
    {
      return arch;
    }
  }
  return Architecture::None;
}

static Architecture pickArchitecture(Architecture arch,
  bool enforceCpuSubType, const std::vector<Architecture> & list)
{
  for (Architecture a : list)
  {
    if (arch == a) { return a; }
  }
  if (!enforceCpuSubType)
  {
    for (Architecture a : list)
    {
      if (getArchInfo(arch).cpuType == getArchInfo(a).cpuType)
      {
        return a;
      }
    }
  }
  return Architecture::None;
}
