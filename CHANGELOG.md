#### wut-tools 1.1.1
- Buildfix from 1.1.0, no major changes.

#### wut-tools 1.1.0
- elf2rpl: Added a new feature, `__rplwrap`. This will rename any symbol named `__rplwrap_<name>` (where `<name>` is any string) to just `<name>`. If a `<name>` already exists that would conflict with the new symbol, it is renamed to `__rplwrap_name`.
- rplimportgen: Add support for `:TEXT_WRAP` and `:DATA_WRAP` sections. Every symbol in these sections will be prefixed with `__rplwrap_`. This is useful for cases where Cafe functions conflict with libc functions, and should not be used outside of libc or wut internals.
- No known loader, including decaf, readrpl and the Cafe system loader.elf, actually uses or checks the crc32 and count parameters on an import section. To allow import garbage-collection, these have been hardcoded to dummy values.
- rplimportgen now places each imported function into a dedicated section, -ffunction-sections style. This allows ld to garbage-collect unused imports, but also requires an updated linker script that only ships with wut 1.0.0-beta9 and later.

#### wut-tools 1.0.0
