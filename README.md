# Equihash

This is an optimimzed C++ implementation of Equihash, the [memory-hard Proof-of-Work
 with fast verification](https://www.internetsociety.org/sites/default/files/blogs-media/equihash-asymmetric-proof-of-work-based-generalized-birthday-problem.pdf).

Equihash is an asymmetric proof-of-work algorithm based on a computationally
hard generalized birthday problem, which requires a lot of memory to generate
a proof, but is instant
to verify. Equihash is adapted as the PoW in [Zcash](https://z.cash/) a public implementation
of the cryptocurrency protocol [Zerocash](http://zerocash-project.org/paper).

Equihash has two parameters: **N** (width) and **K** (length), which determine the complexity 
of the underlying problem and thus the memory and time complexity of the Equihash PoW.

The time complexity is proportional to K2^{N/(K+1)}, and memory complexity to 2^{K+N/(K+1)}.

Please report bugs as issues on this repository.

## Usage

`make` builds the executable `equihash`.

### Command-line utility

`equihash` is a command-line utility to test specific Equihash instances
on your system. To show usage instructions, run
`./equihash` without arguments as
```
Usage:  ./equihash -n N -k K -s Seed
Parameters:
        N               The width of the generalized birthday problem, integer divisible by (k+1) 
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
repository is copyright (c) 2016 Dmitry Khovratovich  under
[CC0 license](https://creativecommons.org/about/cc0).

All licenses are therefore GPL-compatible.
