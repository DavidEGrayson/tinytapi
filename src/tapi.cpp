#include "tapi.h"
#include "yaml.h"

#include <string.h>

using namespace tapi;

unsigned APIVersion::getMajor() noexcept {
  return 2;  // 2.0.0
}

static bool isWhiteSpace(char c)
{
  return c == '\r' || c == '\n' || c == ' ' || c == '\t';
}

static bool detectYAML(const char * data, size_t size) {
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

LinkerInterfaceFile * LinkerInterfaceFile::create(const std::string & path,
  const uint8_t * data, size_t size,
  cpu_type_t cpuType, cpu_subtype_t cpuSubType,
  CpuSubTypeMatching matchingMode, PackedVersion32 minOSVersion,
  std::string & error) noexcept {

  error.clear();

  if (path.empty()) {
    error = "The path argument is empty.";
    return NULL;
  }

  if (data == NULL) {
    error = "The data pointer is null.";
    return NULL;
  }

  if (!detectYAML((const char *)data, size)) {
    error = "File does not look like YAML; might be a binary.";
    return NULL;
  }

  yaml_parser_t parser;
  if (!error.size()) {
    int success = yaml_parser_initialize(&parser);
    if (!success) {
      error = "Failed to initialize YAML parser.";
    }
  }

  yaml_document_t doc;
  if (!error.size()) {
    yaml_parser_set_input_string(&parser, data, size);
    int success = yaml_parser_load(&parser, &doc);
    if (!success) {
      error = "Failed to initialize YAML parser.";
    }
  }

  LinkerInterfaceFile * file = NULL;
  if (!error.size()) {
    file = new LinkerInterfaceFile();
  }

  // ORIG: set patch level to 0 on minOSVersion

  error = "Parsing unimplemented!";  // TODO

  yaml_parser_delete(&parser);
  yaml_document_delete(&doc);

  if (error.size() && file) {
    delete file;
    file = NULL;
  }

  return file;
}
