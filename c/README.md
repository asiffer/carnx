## Adding a counter

To add a specific counter several files must be adapted:

* In `common.h` the counter must be added in the `enum Counter`
definition (no matter the position but before `__END_OF_COUNTERS__`)
* In `common.c`, a new entry must be added in the function `reverse_lookup(int c, char* name)`. The returned string should
have the same name as the `enum Counter`. For instance if the new counter is defined as `FTP` in `enum Counter`), its name should be `"FTP"`
* In `kernel.c` the way the counter is incremented must be implemented

After these stages, the code can be re-compiled by invoking `make`.
