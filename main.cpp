#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>

constexpr size_t SECTOR_SIZE = 2048;
constexpr size_t FRAME_SIZE = 2064;

static constexpr auto ECMA_267 = []() {
    std::array<uint8_t, 16 * SECTOR_SIZE> arr{};
    uint16_t sr = 0x0001;
    for (uint16_t i = 0; i < 16 * SECTOR_SIZE; ++i) {
        arr[i] = static_cast<uint8_t>(sr);
        for (uint8_t b = 0; b < 8; ++b) {
            uint16_t lsb = (sr >> 14 & 1) ^ (sr >> 10 & 1);
            sr = (sr << 1 & 0x7FFF) | lsb;
        }
    }
    return arr;
}();

int main(int argc, char* argv[]) {
    // Print help text if no inputs given
    if (argc < 2) {
        std::cout << "ECMA-267 Scrambler/Descrambler (Deterous, 2026)" << std::endl;
        std::cout << "Usage: dvd-scramble <file_path> [type]" << std::endl;
        std::cout << "[type] = \"iso\" (2048-byte sectors, default), \"raw\" (2064-byte frames), or \"nintendo\" (2064-byte sectors)" << std::endl;
        return 1;
    }

    int sector_size = SECTOR_SIZE;
    int start_byte = 0;
    int end_byte = sector_size;
    bool nintendo = false;

    // Parse inputs
    const char* path = argv[1];
    if (argc > 2) {
        char arg2 = argv[2][0];
        if (arg2 == 'r' || arg2 == 'R') {
            sector_size = FRAME_SIZE;
            start_byte = 12;
            end_byte = sector_size - 4;
        } else if (arg2 == 'n' || arg2 == 'N') {
            sector_size = FRAME_SIZE;
            start_byte = 12;
            end_byte = sector_size - 4;
            nintendo = true;
        }
            
    }

    // Open file for reading and writing
    std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        std::cout << "Error: Could not open file " << path << std::endl;
        return 1;
    }

    // Scramble file one sector at a time
    char sector[FRAME_SIZE];
    std::streampos pos = 0;
    uint32_t count = 0;
    int offset = nintendo ? (int)(7.5 * SECTOR_SIZE) : 0;
    uint8_t id = 0xFF;
    while (!file.eof()) {
        file.seekg(pos);
        file.read(sector, sector_size);
        std::streamsize num_bytes = file.gcount();

        if (num_bytes == 0)
            break;

        while (num_bytes < sector_size && !file.eof()) {
            file.read(sector + num_bytes, sector_size - num_bytes);
            num_bytes += file.gcount();
        }

        uint32_t psn = ((uint32_t)sector[1] << 16) | ((uint32_t)sector[2] << 8) | ((uint32_t)sector[3]);

        // Calculate XOR table offset (Nintendo)
        if (!nintendo)
            offset = (psn >> 4 & 0xF) * SECTOR_SIZE;
        if (nintendo && count >= 16)
            offset = ((id ^ (psn >> 4 & 0xF)) + 7.5) * SECTOR_SIZE;
        else
            offset = 7.5 * SECTOR_SIZE;

        // Calculate Nintendo disc ID checksum from first sector (when re-scrambling)
        if (nintendo && count == 0) {
            // check whether 8 consecutive bytes are zeroed as a heuristic for whether first sector is descrambled (or validate EDC)
            if (sector[1050] == 0 && sector[1051] == 0 && sector[1052] == 0 && sector[1053] == 0 && sector[1054] == 0 && sector[1055] == 0 && sector[1056] == 0 && sector[1057] == 0) {
                int sum = sector[6] + sector[7] + sector[8] + sector[9] + sector[10] + sector[11] + sector[12] + sector[13];
                id = ((sum >> 4) + sum) & 0xF;
            }
        }

        // XOR user data bytes
        for (int i = start_byte; i < end_byte; ++i)
        {
            uint32_t index = offset + i - start_byte;
            if (index >= 16 * SECTOR_SIZE)
                index -= 16 * SECTOR_SIZE - 1;
            sector[i] ^= ECMA_267[index];
        }

        // Calculate Nintendo disc ID checksum from first sector (when descrambling)
        if (nintendo && count == 0 && id == 0xFF) {
            int sum = sector[6] + sector[7] + sector[8] + sector[9] + sector[10] + sector[11] + sector[12] + sector[13];
            id = ((sum >> 4) + sum) & 0xF;
        }

        count += 1;

        if (file.eof())
            file.clear();

        file.seekp(pos);
        file.write(sector, num_bytes);
        pos += num_bytes;
    }

    file.close();
    std::cout << "Scrambled " << count << " sectors" << std::endl;

    return 0;
}

