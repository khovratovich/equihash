# Equihash

This is an optimized C++ implementation of Equihash, the [memory-hard Proof-of-Work
 with fast verification](https://www.internetsociety.org/sites/default/files/blogs-media/equihash-asymmetric-proof-of-work-based-generalized-birthday-problem.pdf). Equihash is designed by Alex Biryukov and Dmitry Khovratovich, cryptographers at the [University of Luxembourg](https://www.cryptolux.org/index.php/Home).

Equihash is an asymmetric proof-of-work algorithm based on a computationally
hard generalized birthday problem, which requires a lot of memory to generate
a proof, but is instant
to verify. Equihash is adapted as the PoW in [Zcash](https://z.cash/) a public implementation
of the cryptocurrency protocol [Zerocash](http://zerocash-project.org/paper). It is possible to use Equihash in TLS as a [client puzzle](https://tools.ietf.org/html/draft-nygren-tls-client-puzzles-00).

Equihash has two parameters: **N** (width in bits) and **K** (length), which determine the complexity 
of the underlying problem and thus the memory and time complexity of the Equihash PoW. The underlying hash function is Blake2b, but any collision-resistant hash function would work too.

The time complexity is proportional to K2^{N/(K+1)}, and memory complexity to 2^{K+N/(K+1)}. The proof size is 2^{K}(1+N/(K+1))+192 bits. Verification requires 2^K hashes and XORs. 

Please report bugs as issues on this repository.

## Recommended parameters (N,K)

For cryptocurrencies: (100/110/120,4), (108/114/120/126,5). 

For client puzzles: (60/70/80/90,4), (90/96/102,5).

## Usage

`make` builds the executable `equihash`.

### Command-line utility

`equihash` is a command-line utility to test specific Equihash instances
on your system. To show usage instructions, run
`./equihash` without arguments as
```
Usage:  ./equihash -n N -k K -s Seed
Parameters:
        N               The width (number of bits) of the generalized birthday problem, integer divisible by (K+1) 
        K               The length of the generalized birthday problem, small integer
        Seed            Seed for the problem, to distinguish between solutions. Integer.
```
For example, to compute Equihash using N=120 and k=5, consuming at least 32 MB of RAM
```
$ ./equihash -n 120 -k 5 -s 3
```

## Alternative implementations


* [Zcash](https://github.com/zcash/zcash/) by [@str4d](https://github.com/str4d)

## Intellectual property

The Equihash code in this
repository is copyright (c) 2016 Dmitry Khovratovich (University of Luxembourg)  under
[CC0 license](https://creativecommons.org/about/cc0).

The license is GPL-compatible.
