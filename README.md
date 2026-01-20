# DVD Scrambler / Descrambler
Unconditionally applies ECMA-267 scrambling to an entire DVD disc image file.

Run the function again to descramble, it is a reversible function.

Usage:

- (De)scramble ISO (2048-byte sectors): `./dvd-scramble dump.iso`
- (De)scramble raw DVD frames (2064-byte sectors): `./dvd-scramble.exe dump.bin raw`
- (De)scramble Nintendo disc dump (2064-byte sectors): `./dvd-scramble.exe dump.raw nintendo`

**Note**: This will overwrite the input file. Running the same command again will give you back the original file.

## Technical Note

Nintendo descrambling is deterministic, done without any brute forcing of seeds. Nintendo stores the user data beginning from the usual offset of the CPR_MAI. Scrambling is very similar to standard DVD (ECMA-267) scrambling, with the following changes:

The first ECC block is descrambled with an offset of 7.5 sectors into the XOR table.  The first 8 bytes of descrambled user data of the first sector are summed, then the result's two least significant nibbles are summed. The least significant nibble of the result is a constant value that is XOR'd with the standard DVD table index, and the 7.5 sector offset is also applied, resulting in a pseudo-random indexing of the standard XOR table that changes between discs. The C++ code shows an example implementation. This research was done for [redumper](https://github.com/superg/redumper/) which is capable of dumping & descrambling Nintendo (GameCube and Wii) discs.

