# pizza
Generic C library written by gonzo, containing:

* Macros / functions to allocate / free memory and do some basic sanity checks.
* A Slice data type: read-only access to an array of bytes.
* A Buffer data type: write-only access to an array of bytes.
* Date-related functions.
* Logging functions that can be controlled at compile- and run-time.
* UTF8 encoding and decoding (uses Slice and Buffer).
