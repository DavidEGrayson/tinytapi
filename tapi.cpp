#include "tapi.h"
#include "yaml.h"

using namespace tapi;

unsigned APIVersion::getMajor() noexcept {
  return 2;  // 2.0.0
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

  if (strcmp(data, "---"))
  {
    error = "Data does not start with three dashes; might be a binary.";
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
