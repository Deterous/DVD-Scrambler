# DVD Scrambler / Descrambler
Unconditionally applies ECMA-267 scrambling to an entire DVD disc image file.

Run the function again to descramble, it is a reversible function.

Usage:

- (De)scramble ISO (2048-byte sectors): `./dvd-scramble.exe dump.iso`
- (De)scramble raw DVD frames (2064-byte sectors): `./dvd-scramble.exe dump.bin raw`
- (De)scramble Nintendo disc dump (2064-byte sectors): `./dvd-scramble.exe dump.bin nintendo`

**Note**: This will overwrite the input file. Running the same command again will give you back the original file.

## Technical Note

Nintendo descrambling is deterministic, done without any brute forcing of seeds. In the data frame, Nintendo stores the 2048-byte user data beginning from the usual offset of the CPR_MAI, and instead stores the CPR_MAI immediately before the EDC. Scrambling is very similar to standard DVD (ECMA-267) scrambling, with the following changes:

The first ECC block is descrambled with an offset of 7.5 sectors into the XOR table (i.e. starting after 1024 bytes of LSFR bit shifting when using the 7th seed).
The first 8 bytes of descrambled user data of the first sector are summed, then the result's two least significant nibbles are summed.
The least significant nibble of the result is a constant value that is XOR'd with the standard DVD table index, and the 7.5 sector offset is also applied, resulting in a pseudo-random indexing of the standard XOR table that changes between discs.
e.g. if the first 8 user data bytes are `52 4D 43 4B 30 31 00 00`, they sum to `0x18E` (`0001 1000 1110`), and summing the lower two nibbles (dropping the carry), `1000 + 1110 = 0110` gives a value of 6. This value of 6 is XOR'd with the standard DVD index value (the 2nd lowest nibble of the PSN value), and this is used to index the XOR table instead (with the 7.5 sector offset).

The C++ code shows an example implementation, which is easier to understand than my explanation. This research was done for [redumper](https://github.com/superg/redumper/) which is capable of dumping & descrambling Nintendo (GameCube and Wii) discs.
