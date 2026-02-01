#include <carbon.h>

int main(int argc, char **argv) {
  const auto self = CARBON_SHIFT_ARGS(argc, argv);
  if (argc != 2) {
    CBN_ERROR("usage: %s [IN_FILE] [OUT_FILE]", self);
    return 1;
  }
  const auto in_file = CARBON_SHIFT_ARGS(argc, argv);
  const auto out_file = CARBON_SHIFT_ARGS(argc, argv);
  if (!cbn::SKAP::Create(in_file, out_file)) return 1;
  return 0;
}
