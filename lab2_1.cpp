#include <iostream>
#include <iomanip>
#include <cstdint>

int main() {
    uint32_t rlz_1 = 0x003b2c1d;
    uint32_t rlz_2[4] = {0x003b2c1d, 0x7e8f9a0b, 0x5c6d7e8f, 0x1a2b3c4a};
    
    const uint32_t count = 1000000;
    const int max_tau = 31;
    const int window_size = 64;

    // Счетчики для анализа битов
    uint32_t p1[2] = {0};
    uint32_t p2[4] = {0};
    uint32_t p3[8] = {0};
    uint32_t p4[16] = {0};
    
    uint8_t c = 0;
    uint32_t bit_count = 0;
    
    uint32_t psp_window = 0;
    uint32_t acf_counts[max_tau+1][2] = {0};

    for(uint32_t i = 0; i < count; i++) {
        // Генерация бита первого ЛРС
        uint32_t bit1 = (rlz_1 ^ (rlz_1 >> 3)) & 0x01;
        rlz_1 = (rlz_1 >> 1) | (bit1 << 19);

        // Генерация бита второго ЛРС
        uint32_t bit2 = (rlz_2[0] ^ (rlz_2[0] >> 7) ^ (rlz_2[0] >> 2)) & 0x01;
        
        // Сдвиг регистров второго ЛРС
        for(int j = 0; j < 3; j++) {
            uint32_t carry = rlz_2[j+1] & 0x01;
            rlz_2[j] = (rlz_2[j] >> 1) | (carry << 31);
        }
        rlz_2[3] = (rlz_2[3] >> 1) | (bit2 << 22);

        uint32_t bit_psp = (rlz_1 ^ rlz_2[0]) & 0x01;
        
        // Обновление окна
        psp_window = (psp_window << 1) | bit_psp;
        
        if (i >= window_size) {
            uint64_t current_bit = psp_window & 0x01;
            for (int tau = 0; tau <= max_tau; tau++) {
                uint32_t shifted_bit = (psp_window >> tau) & 0x01;
                if (current_bit == shifted_bit) {
                    acf_counts[tau][0]++;
                } else {
                    acf_counts[tau][1]++;
                }
            }
        }

        uint8_t b = bit_psp;
        p1[b]++;
        
        c = (c << 1) | b;
        bit_count++;
        
        if (bit_count >= 2) p2[c & 0x03]++;
        if (bit_count >= 3) p3[c & 0x07]++;
        if (bit_count >= 4) {
            p4[c & 0x0F]++;
            c &= 0x07;
            bit_count = 3;
        }
    }
    std::cout << "АНАЛИЗ ПСП (длина = " << count << " бит)\n";
    std::cout << "=================================\n";
    std::cout<<std::endl;
    std::cout << "   0: " << p1[0] << " (" << std::fixed << std::setprecision(5) << static_cast<double>(p1[0]) / count << ")\n";
    std::cout << "   1: " << p1[1] << " (" << std::fixed << std::setprecision(5) << static_cast<double>(p1[1]) / count << ")\n\n";
    for(int i = 0; i < 4; i++) {
        std::cout << "   " << (i>>1) << (i&1) << ": " << p2[i] << " (" 
                  << std::fixed << std::setprecision(3) << static_cast<double>(p2[i]) / (count-1) << ")\n";
    }
    std::cout<<std::endl;
    for(int i = 0; i < 8; i++) {
        std::cout << "   " << (i>>2) << ((i>>1)&1) << (i&1) << ": " << p3[i] << " (" 
                  << std::fixed << std::setprecision(3) << static_cast<double>(p3[i]) / (count-2) << ")\n";
    }
    std::cout<<std::endl;
    for(int i = 0; i < 8; i++) {
        std::cout << "   " << (i>>3) << ((i>>2)&1) << ((i>>1)&1) << (i&1) << ": " << p4[i] << " (" 
                  << std::fixed << std::setprecision(3) << static_cast<double>(p4[i]) / (count-3) << ")\n";
    }
    std::cout<<std::endl;
    for (int tau = 0; tau <= max_tau; tau++) {
        uint32_t total = acf_counts[tau][0] + acf_counts[tau][1];
        if (total > 0) {
            double acf = (static_cast<double>(acf_counts[tau][0]) - acf_counts[tau][1]) / total;
            std::cout << "   tau=" << std::setw(2) << tau << ": " 
                      << std::fixed << std::setprecision(6) << acf << std::endl;
        }
    }

    return 0;
}
