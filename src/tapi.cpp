// API of this library
#include "tapi/tapi.h"

// External libraries
#include "yaml.h"

// Standard external libraries
#include <string.h>
#include <iostream>  // TODO: remove

// Components of this compilation unit
#include "arch.h"

using namespace tapi;

struct ExportItem
{
  std::vector<Architecture> archs;
  std::vector<std::string> symbols, weak_symbols, objc_classes, objc_ivars;
};

struct tapi::StubData
{
  std::string filename;
  std::vector<Architecture> archs;
  Platform platform;
  std::string installName;
  std::vector<ExportItem> exports;
};

unsigned APIVersion::getMajor() noexcept
{
  return 2;  // 2.0.0
}

static bool isWhiteSpace(char c)
{
  return c == '\r' || c == '\n' || c == ' ' || c == '\t';
}

static bool detectYAML(const uint8_t * udata, size_t size)
{
  const char * data = (const char *)udata;

  // Ignore whitespace at the end.
  while (size && isWhiteSpace(data[size - 1])) { size--; }

  auto startsWith = [&](const std::string & str) -> bool {
    return size >= str.size() && std::string(data, str.size()) == str;
  };

  auto endsWith = [&](const std::string & str) -> bool {
    return size >= str.size() &&
      std::string(data + size - str.size(), str.size()) == str;
  };

  return startsWith("---") && endsWith("...");
}

static std::string convertYAMLString(const yaml_node_t * node)
{
  if (node->type != YAML_SCALAR_NODE) { return ""; }
  return { (const char *)node->data.scalar.value,
    node->data.scalar.length };
}

static Platform convertYAMLPlatform(const yaml_node_t * node)
{
  std::string str = convertYAMLString(node);
  if (str == "macosx") { return Platform::OSX; }
  else if (str == "ios") { return Platform::iOS; }
  else if (str == "watchos") { return Platform::watchOS; }
  else if (str == "tvos") { return Platform::tvOS; }
  else if (str == "bridgeos") { return Platform::bridgeOS; }
  return Platform::Unknown;
}

static std::vector<std::string> convertYAMLStringList(
  yaml_document_t * doc, yaml_node_t * node)
{
  if (node->type != YAML_SEQUENCE_NODE) { return {}; }
  std::vector<std::string> list;
  yaml_node_item_t * start = node->data.sequence.items.start;
  yaml_node_item_t * top = node->data.sequence.items.top;
  for (yaml_node_item_t * i = start; i < top; i++)
  {
    yaml_node_t * child = yaml_document_get_node(doc, *i);
    list.push_back(convertYAMLString(child));
  }
  return list;
}

static std::vector<Architecture> convertYAMLArchList(
  yaml_document_t * doc, yaml_node_t * node)
{
  std::vector<Architecture> list;
  for (const std::string & name : convertYAMLStringList(doc, node))
  {
    Architecture arch = getArchByName(name);
    if (arch != Architecture::None) { list.push_back(arch); }
  }
  return list;
}

static ExportItem convertYAMLExportItem(
  yaml_document_t * doc, yaml_node_t * node)
{
  ExportItem item;
  if (node->type != YAML_MAPPING_NODE) { return item; }
  yaml_node_pair_t * start = node->data.mapping.pairs.start;
  yaml_node_pair_t * top = node->data.mapping.pairs.top;
  for (yaml_node_pair_t * pair = start; pair < top; pair++)
  {
    yaml_node_t * key_node = yaml_document_get_node(doc, pair->key);
    yaml_node_t * value_node = yaml_document_get_node(doc, pair->value);

    if (key_node->type != YAML_SCALAR_NODE) { continue; }

    std::string key = convertYAMLString(key_node);

    if (key == "archs")
    {
      item.archs = convertYAMLArchList(doc, value_node);
    }
    else if (key == "symbols")
    {
      item.symbols = convertYAMLStringList(doc, value_node);
    }
    else if (key == "weak-def-symbols")
    {
      item.weak_symbols = convertYAMLStringList(doc, value_node);
    }
    else if (key == "objc-classes")
    {
      item.objc_classes = convertYAMLStringList(doc, value_node);
    }
    else if (key == "objc-ivars")
    {
      item.objc_ivars = convertYAMLStringList(doc, value_node);
    }
    // TODO: re-exports
  }
  return item;
}

static std::vector<ExportItem> convertYAMLExportList(
  yaml_document_t * doc, yaml_node_t * node)
{
  if (node->type != YAML_SEQUENCE_NODE) { return {}; }
  std::vector<ExportItem> list;
  yaml_node_item_t * start = node->data.sequence.items.start;
  yaml_node_item_t * top = node->data.sequence.items.top;
  for (yaml_node_item_t * i = start; i < top; i++)
  {
    yaml_node_t * node = yaml_document_get_node(doc, *i);
    list.push_back(convertYAMLExportItem(doc, node));
  }
  return list;
}

static StubData parseYAML(const uint8_t * data, size_t size,
  std::string & error)
{
  StubData r;

  yaml_parser_t parser;
  if (!error.size())
  {
    int success = yaml_parser_initialize(&parser);
    if (!success)
    {
      error = "Failed to initialize YAML parser.";
    }
  }

  yaml_document_t doc;
  if (!error.size())
  {
    yaml_parser_set_input_string(&parser, data, size);
    int success = yaml_parser_load(&parser, &doc);
    if (!success)
    {
      error = "Failed to initialize YAML parser.";
    }
  }

  // Get the root node and make sure it is a mapping.
  yaml_node_t * root;
  if (!error.size())
  {
    root = yaml_document_get_root_node(&doc);
    if (root->type != YAML_MAPPING_NODE)
    {
      error = "YAML root node is not a mapping.";
    }
  }

  if (!error.size())
  {
    yaml_node_pair_t * start = root->data.mapping.pairs.start;
    yaml_node_pair_t * top = root->data.mapping.pairs.top;
    for (yaml_node_pair_t * pair = start; pair < top; pair++)
    {
      yaml_node_t * key_node = yaml_document_get_node(&doc, pair->key);
      yaml_node_t * value_node = yaml_document_get_node(&doc, pair->value);

      if (key_node->type != YAML_SCALAR_NODE)
      {
        error = "Root mapping has a non-scalar key.";
        break;
      }

      std::string key = convertYAMLString(key_node);

      if (key == "platform")
      {
        r.platform = convertYAMLPlatform(value_node);
      }
      else if (key == "install-name")
      {
        r.installName = convertYAMLString(value_node);
      }
      else if (key == "archs")
      {
        r.archs = convertYAMLArchList(&doc, value_node);
      }
      else if (key == "exports")
      {
        r.exports = convertYAMLExportList(&doc, value_node);
      }
      // TODO: objc-constraints
      // TODO: current-version
      // TODO: compatibility-version
      // TODO: uuids
    }
  }

  yaml_parser_delete(&parser);
  yaml_document_delete(&doc);

  return r;
}

bool LinkerInterfaceFile::isSupported(const std::string & path,
  const uint8_t * data, size_t size) noexcept
{
  (void)path;
  return detectYAML(data, size);
}

bool LinkerInterfaceFile::shouldPreferTextBasedStubFile(
  const std::string & path) noexcept
{
  (void)path;
  // Note: The original library would actually load the file and check
  // "isInstallAPI()".  Not sure how important it is to do all that work so
  // let's just return false for now to match the observed behavior of
  // Apple TAPI.
  return false;
}

bool LinkerInterfaceFile::areEquivalent(const std::string & tbdPath,
  const std::string & dylibPath) noexcept
{
  (void)tbdPath; (void)dylibPath;
  // TODO: Load both files and check to see if they have some UUIDs in
  // common or something.
  return false;
}

void LinkerInterfaceFile::init(const StubData & d,
  cpu_type_t cpuType, cpu_subtype_t cpuSubType,
  CpuSubTypeMatching matchingMode, PackedVersion32 minOSVersion,
  std::string & error)
{
  platform = d.platform;
  installName = d.installName;

  Architecture cpuArch = getCpuArch(cpuType, cpuSubType);
  if (cpuArch == Architecture::None)
  {
    error = "Unrecognized desired architecture.";
    return;
  }

  bool enforceCpuSubType = matchingMode == CpuSubTypeMatching::Exact;
  Architecture selectedArch = pickArchitecture(
    cpuArch, enforceCpuSubType, d.archs);
  if (selectedArch == Architecture::None)
  {
    error = "missing required architecture " +
      std::string(getArchInfo(cpuArch).name) + " in file " +
      d.filename;
    return;
  }

  for (const ExportItem & item : d.exports)
  {
    bool hasSelectedArch = false;
    for (const Architecture & arch : item.archs)
    {
      if (arch == selectedArch)
      {
        hasSelectedArch = true;
        break;
      }
    }
    if (!hasSelectedArch) { continue; }

    for (const std::string & name : item.symbols)
    {
      exportList.push_back(name);
    }

    for (const std::string & name : item.weak_symbols)
    {
      Symbol sym(name);
      sym.weak = true;
      exportList.push_back(sym);
    }

    for (const std::string & name : item.objc_classes)
    {
      exportList.push_back("_OBJC_CLASS_$_" + name);
      exportList.push_back("_OBJC_METACLASS_$_" + name);
    }

    for (const std::string & name : item.objc_ivars)
    {
      exportList.push_back("_OBJC_IVAR_$_" + name);
    }
  }

  auto it = exportList.begin();
  while (it != exportList.end())
  {
    const Symbol & sym = *it;
    if (sym.name.substr(0, 11) == "$ld$hide$os")
    {
      // TODO: probably should parse the OS version and symbol number and
      // hide the corresponding symbol if minOSVersion is less.
      it = exportList.erase(it);
    }
    else
    {
      ++it;
    }
  }

  minOSVersion.setPatch(0);
}

LinkerInterfaceFile * LinkerInterfaceFile::create(const std::string & path,
  const uint8_t * data, size_t size,
  cpu_type_t cpuType, cpu_subtype_t cpuSubType,
  CpuSubTypeMatching matchingMode, PackedVersion32 minOSVersion,
  std::string & error) noexcept
{
  error.clear();

  if (path.empty())
  {
    error = "The path argument is empty.";
    return nullptr;
  }

  if (data == nullptr)
  {
    error = "The data pointer is nullptr.";
    return nullptr;
  }

  StubData d;

  if (detectYAML(data, size))
  {
    d = parseYAML(data, size, error);
  }
  else
  {
    error = "File does not look like YAML; might be a binary.";
    return nullptr;
  }
  d.filename = path;

  if (error.size()) { return nullptr; }

  LinkerInterfaceFile * file = new LinkerInterfaceFile();
  file->init(d, cpuType, cpuSubType, matchingMode, minOSVersion, error);

  if (error.size())
  {
    delete file;
    return nullptr;
  }

  return file;
}
