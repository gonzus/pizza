# pizza
Generic C library written by gonzo, containing:

* A Slice data type: read-only access to an array of bytes.
* A Buffer data type: write-only access to an array of bytes.
* Date-related & time-related functions.
* A timer implementation with nanosecond (ns) resolution.
* Logging functions that can be controlled at compile- & run-time.
* UTF8 encoding & decoding (uses Slice & Buffer).
* A simple thread pool implementation (uses pthreads).
* Random number generation using [Mersenne
  Twister](https://en.wikipedia.org/wiki/Mersenne_Twister).
* Commonly used hashing functions.
* [MD5](https://en.wikipedia.org/wiki/MD5) hashing (uses Slice & Buffer).
* [Base64](https://en.wikipedia.org/wiki/Base64) encoding & decoding (uses
  Slice & Buffer).
* [URI (percent)](https://en.wikipedia.org/wiki/Percent-encoding) encoding &
  decoding (uses Slice & Buffer).
* [Blowfish](https://en.wikipedia.org/wiki/Blowfish_(cipher)) encryption &
  decryption.
* [CBC](https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation#Cipher_block_chaining_(CBC))
  encryption & decryption with Blowfish (uses Slice & Buffer).
* [Deflate](https://en.wikipedia.org/wiki/Deflate) compression & uncompression
  (uses [zlib](https://en.wikipedia.org/wiki/Zlib) + Slice & Buffer).
* Paths, inspired on [Perl's Path::Tiny](https://metacpan.org/pod/Path::Tiny).
