# pizza
Generic C library written by gonzo, containing:

* A Slice data type: read-only access to an array of bytes.
* A Buffer data type: write-only access to an array of bytes.
* Date-related and time-related functions.
* A timer implementation with nanosecond (ns) resolution.
* Logging functions that can be controlled at compile- and run-time.
* UTF8 encoding and decoding (uses Slice and Buffer).
* A simple thread pool implementation.
* Random number generation using Mersenne Twister.
* Hashing functions.
