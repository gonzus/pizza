# pizza
Generic C library written by gonzo, containing:

* A Slice data type: read-only access to an array of bytes.
* A Buffer data type: write-only access to an array of bytes.
* Date-related and time-related functions.
* A timer implementation with nanosecond (ns) resolution.
* Logging functions that can be controlled at compile- and run-time.
* UTF8 encoding and decoding (uses Slice and Buffer).
* A simple thread pool implementation (uses pthreads).
* Random number generation using Mersenne Twister.
* Commonly used hashing functions.
* MD5 hashing (uses Slice and Buffer).
* Base64 encoding and decoding (uses Slice and Buffer).
* URI (percent) encoding and decoding (uses Slice and Buffer).
* Blowfish encryption and decryption.
* CBC descryption using Blowfish (uses Slice and Buffer).
* Deflate compression and uncompression (uses zlib + Slice and Buffer).
