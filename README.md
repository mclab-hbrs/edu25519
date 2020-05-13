# Edu25519
This is an implementation of Curve25519 from Daniel Bernstein.
It uses the most tricks introduced in the original paper, mainly the radix ceil(25.5i) storing that makes reduction very fast.

This code was written as a clean, readable, self-contained implementation for **educational purposes only**. 
It has not been optimized for speed or security, but for readability!

## Build
To build the library and example program, do the usual cmake dance:

```
mkdir build && cd build
cmake .. && make
./example
```

## Sources
The comments in the code frequently mention the main sources by their index:

1. [D. J. Bernstein. Curve25519: new Diffie-Hellman speed records. Proceedings of PKC 2006](https://cr.yp.to/ecdh/curve25519-20060209.pdf)
2. [Hankerson, Darrel, Alfred J. Menezes, and Scott Vanstone. Guide to elliptic curve cryptography. Springer Science & Business Media, 2006.](https://dl.acm.org/doi/book/10.5555/940321)
3. [RFC 7748 Elliptic Curves for Security](https://tools.ietf.org/html/rfc7748)
4. [Arduino Cryptography Library](https://github.com/rweather/arduinolibs/blob/b9fefc6a76bb7ade3fa82ac0e358445c138a6a28/libraries/Crypto/Curve25519.cpp#L369)
