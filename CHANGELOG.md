#### wut-tools 1.1.0
- elf2rpl: Added a new feature, `__rplwrap`. This will rename any symbol named `__rplwrap_<name>` (where `<name>` is any string) to just `<name>`. If a `<name>` already exists that would conflict with the new symbol, it is renamed to `__rplwrap_name`.
- rplimportgen: Add support for `:TEXT_WRAP` and `:DATA_WRAP` sections. Every symbol in these sections will be prefixed with `__rplwrap_`. This is useful for cases where Cafe functions conflict with libc functions, and should not be used outside of libc or wut internals.

#### wut-tools 1.0.0
