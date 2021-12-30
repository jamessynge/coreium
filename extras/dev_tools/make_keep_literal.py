#!/usr/bin/env python3
"""Outputs KeepLiteral template functions for progmem_string_data.h."""


def OutputOneSize(keep_count: int, discard_count: int) -> None:
  print(f'// Keep {keep_count} characters, discard {discard_count} nulls.')

  var_names = [f'C{n}' for n in range(1, keep_count + 1)]
  template_params = ', '.join([f'char {v}' for v in var_names] + ['char... X'])
  input_fragments = ', '.join([f'StringFragment<{v}>' for v in var_names] +
                              ['StringFragment<X>...'])
  output_params = ', '.join(var_names)

  print(f"""template <{template_params}>
auto KeepLiteral(DiscardCount<{discard_count}>, {input_fragments})
-> StringFragment<{output_params}>;
""")


def main():
  for keep_count in range(1, 16):
    OutputOneSize(keep_count, 16 - keep_count)


if __name__ == '__main__':
  main()
