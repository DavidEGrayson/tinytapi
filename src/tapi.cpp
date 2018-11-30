// API of this library
#include "tapi.h"

// External libraries
#include "yaml.h"

// Standard external libraries
#include <string.h>

// Components of this compilation unit
#include "arch.h"

using namespace tapi;

unsigned APIVersion::getMajor() noexcept
{
  return 2;  // 2.0.0
}

static bool isWhiteSpace(char c)
{
  return c == '\r' || c == '\n' || c == ' ' || c == '\t';
}

static bool detectYAML(const char * data, size_t size)
{
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

bool LinkerInterfaceFile::isSupported(const std::string & path,
  const uint8_t * data, size_t size) noexcept
{
  (void)path;
  return detectYAML((const char *)data, size);
}

bool LinkerInterfaceFile::shouldPreferTextBasedStubFile(
  const std::string & path) noexcept
{
  (void)path;
  // Note: The original library would actually load the file and check
  // "isInstallAPI()".  Not sure how important it is to do all that work so
  // let's just return true for now.
  return true;
}

static bool areEquivalent(const std::string & tbdPath,
  const std::string & dylibPath) noexcept
{
  (void)tbdPath; (void)dylibPath;
  // Note: The original library would load both files and check to see if they
  // have some UUIDs in common.  I don't think the TBD files in the macOS
  // SDK have UUIDs so this would always just return false.
  return false;
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

  Architecture arch = getCpuArch(cpuType, cpuSubType);
  if (arch == Architecture::None)
  {
    error = "Unrecognized architecture.";
    return nullptrptr;
  }

  if (!detectYAML((const char *)data, size))
  {
    error = "File does not look like YAML; might be a binary.";
    return nullptr;
  }

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

  LinkerInterfaceFile * file = nullptr;
  if (!error.size())
  {
    file = new LinkerInterfaceFile();
  }

  // ORIG: set patch level to 0 on minOSVersion

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
    for (yaml_node_pair_t * pair = root->data.mapping.pairs.start;
      pair < root->data.mapping.pairs.top; pair++)
    {
      yaml_node_t * key = yaml_document_get_node(&doc, pair->key);
      yaml_node_t * value = yaml_document_get_node(&doc, pair->value);

      if (key->type != YAML_SCALAR_NODE)
      {
        error = "Root mapping has a non-scalar key.";
        break;
      }

      std::string key_str((const char *)key->data.scalar.value,
        key->data.scalar.length);

      if (key_str == "install-name")
      {
        if (value->type != YAML_SCALAR_NODE)
        {
          error = "Install name is not a scalar.";
          break;
        }
        file->installName = { (const char *)value->data.scalar.value,
          value->data.scalar.length };
      }
    }
  }

  yaml_parser_delete(&parser);
  yaml_document_delete(&doc);

  if (error.size() && file)
  {
    delete file;
    file = nullptr;
  }

  return file;
}
