#include <iostream>
#include <iomanip>  
#include <cstdint>  

// Функция для подсчета количества единиц в числе для заданного количества бит
int count_ones(uint32_t num, int bits) {
    int count = 0;
    for (int i = 0; i < bits; i++) {
        if (num & (1 << i)) count++;  // Проверяем i-й бит и увеличиваем счетчик если он равен 1
    }
    return count;
}

int main() {
    uint32_t rlz_1 = 0x003b2c1d;  
    uint32_t rlz_2[4] = {0x003b2c1d, 0x7e8f9a0b, 0x5c6d7e8f, 0x1a2b3c4a}; 
    
    const uint32_t count = 1000000;  
    const int max_tau = 31;         
    const int window_size = 64;     

    uint32_t p1[2] = {0};   // Для одиночных бит (0 или 1)
    uint32_t p2[4] = {0};   // Для 2-битных комбинаций
    uint32_t p3[8] = {0};   // Для 3-битных комбинаций
    uint32_t p4[16] = {0};  // Для 4-битных комбинаций
    
    // Счетчики для комбинаций с одинаковым количеством единиц
    uint32_t equal_ones_2bit[2] = {0};  // [0] - 0 или 2 единицы, [1] - 1 единица
    uint32_t equal_ones_3bit[2] = {0};  // [0] - 0 или 3 единицы, [1] - 1 или 2 единицы
    uint32_t equal_ones_4bit[3] = {0};  // [0] - 0 или 4, [1] - 1 или 3, [2] - 2 единицы
    
    uint8_t c = 0;           // Текущая комбинация бит
    uint32_t bit_count = 0;  // Счетчик бит в текущей комбинации
    
    uint32_t psp_window = 0;  // Окно для анализа автокорреляции
    uint32_t acf_counts[max_tau+1][2] = {0};  // Счетчики для автокорреляционной функции

    for(uint32_t i = 0; i < count; i++) {
        uint32_t bit1 = (rlz_1 ^ (rlz_1 >> 3)) & 0x01;  
        rlz_1 = (rlz_1 >> 1) | (bit1 << 19);           
        uint32_t bit2 = (rlz_2[0] ^ (rlz_2[0] >> 7) ^ (rlz_2[0] >> 2)) & 0x01;  

        for(int j = 0; j < 3; j++) {
            uint32_t carry = rlz_2[j+1] & 0x01;        // Младший бит следующего регистра
            rlz_2[j] = (rlz_2[j] >> 1) | (carry << 31);  // Сдвиг с переносом
        }
        rlz_2[3] = (rlz_2[3] >> 1) | (bit2 << 22); 
        uint32_t bit_psp = (rlz_1 ^ rlz_2[0]) & 0x01;  
        psp_window = (psp_window << 1) | bit_psp;
        
        // акф
        if (i >= window_size) {
            uint64_t current_bit = psp_window & 0x01;
            for (int tau = 0; tau <= max_tau; tau++) {
                uint32_t shifted_bit = (psp_window >> tau) & 0x01;
                if (current_bit == shifted_bit) {
                    acf_counts[tau][0]++;  // совпадение битов
                } else {
                    acf_counts[tau][1]++;  // несовпадение битов
                }
            }
        }
        uint8_t b = bit_psp;
        p1[b]++;  // Подсчет одиночных бит
        
        // Формирование комбинаций бит
        c = (c << 1) | b;
        bit_count++;
        
        // Анализ 2-битных комбинаций
        if (bit_count >= 2) {
            uint8_t pattern2 = c & 0x03;  // Маска для 2 бит
            p2[pattern2]++;
            
            int ones = count_ones(pattern2, 2);
            if (ones == 0 || ones == 2) equal_ones_2bit[0]++;  // 00 или 11
            else equal_ones_2bit[1]++;                         // 01 или 10
        }
        
        // Анализ 3-битных комбинаций
        if (bit_count >= 3) {
            uint8_t pattern3 = c & 0x07;  // Маска для 3 бит
            p3[pattern3]++;
            
            int ones = count_ones(pattern3, 3);
            if (ones == 0 || ones == 3) equal_ones_3bit[0]++;  // 000 или 111
            else equal_ones_3bit[1]++;                          // остальные
        }
        
        // Анализ 4-битных комбинаций
        if (bit_count >= 4) {
            uint8_t pattern4 = c & 0x0F;  // Маска для 4 бит
            p4[pattern4]++;
            
            int ones = count_ones(pattern4, 4);
            if (ones == 0 || ones == 4) equal_ones_4bit[0]++;    // 0000 или 1111
            else if (ones == 1 || ones == 3) equal_ones_4bit[1]++; // 1 или 3 единицы
            else equal_ones_4bit[2]++;                           // 2 единицы
            
            c &= 0x07;  // Сохраняем только 3 бита для следующей итерации
            bit_count = 3;
        }
    }

    // Вывод результатов статистического анализа
    std::cout << "АНАЛИЗ ПСП (длина = " << count << " бит)\n";
    std::cout << "=================================\n\n";
    
    // Вероятности одиночных бит
    std::cout << "Вероятности одиночных бит:\n";
    std::cout << "   0: " << p1[0] << " (" << std::fixed << std::setprecision(5) 
              << static_cast<double>(p1[0]) / count << ")\n";
    std::cout << "   1: " << p1[1] << " (" << std::fixed << std::setprecision(5) 
              << static_cast<double>(p1[1]) / count << ")\n\n";
    
    // Вероятности 2-битных комбинаций
    std::cout << "Вероятности 2-битных комбинаций:\n";
    for(int i = 0; i < 4; i++) {
        std::cout << "   " << (i>>1) << (i&1) << ": " << p2[i] << " (" 
                  << std::fixed << std::setprecision(3) << static_cast<double>(p2[i]) / (count-1) << ")\n";
    }
    std::cout << "\n";
    
    // Вероятности 3-битных комбинаций
    std::cout << "Вероятности 3-битных комбинаций:\n";
    for(int i = 0; i < 8; i++) {
        std::cout << "   " << (i>>2) << ((i>>1)&1) << (i&1) << ": " << p3[i] << " (" 
                  << std::fixed << std::setprecision(3) << static_cast<double>(p3[i]) / (count-2) << ")\n";
    }
    std::cout << "\n";
    
    // Вероятности 4-битных комбинаций
    std::cout << "Вероятности 4-битных комбинаций:\n";
    for(int i = 0; i < 16; i++) {
        std::cout << "   " << (i>>3) << ((i>>2)&1) << ((i>>1)&1) << (i&1) << ": " << p4[i] << " (" 
                  << std::fixed << std::setprecision(3) << static_cast<double>(p4[i]) / (count-3) << ")\n";
    }
    std::cout << "\n";
    
    // Вероятности комбинаций с одинаковым количеством единиц
    std::cout << "ВЕРОЯТНОСТИ КОМБИНАЦИЙ С ОДИНАКОВЫМ КОЛИЧЕСТВОМ ЕДИНИЦ:\n";
    
    // 2-битные комбинации
    std::cout << "2-битные комбинации:\n";
    std::cout << "  Комбинации с 0 или 2 единицами: " << equal_ones_2bit[0] << " (" 
              << std::fixed << std::setprecision(5) << static_cast<double>(equal_ones_2bit[0]) / (count-1) << ")\n";
    std::cout << "  Комбинации с 1 единицей: " << equal_ones_2bit[1] << " (" 
              << std::fixed << std::setprecision(5) << static_cast<double>(equal_ones_2bit[1]) / (count-1) << ")\n\n";
    
    // 3-битные комбинации
    std::cout << "3-битные комбинации:\n";
    std::cout << "  Комбинации с 0 или 3 единицами: " << equal_ones_3bit[0] << " (" 
              << std::fixed << std::setprecision(5) << static_cast<double>(equal_ones_3bit[0]) / (count-2) << ")\n";
    std::cout << "  Комбинации с 1 или 2 единицами: " << equal_ones_3bit[1] << " (" 
              << std::fixed << std::setprecision(5) << static_cast<double>(equal_ones_3bit[1]) / (count-2) << ")\n\n";
    
    // 4-битные комбинации
    std::cout << "4-битные комбинации:\n";
    std::cout << "  Комбинации с 0 или 4 единицами: " << equal_ones_4bit[0] << " (" 
              << std::fixed << std::setprecision(5) << static_cast<double>(equal_ones_4bit[0]) / (count-3) << ")\n";
    std::cout << "  Комбинации с 1 или 3 единицами: " << equal_ones_4bit[1] << " (" 
              << std::fixed << std::setprecision(5) << static_cast<double>(equal_ones_4bit[1]) / (count-3) << ")\n";
    std::cout << "  Комбинации с 2 единицами: " << equal_ones_4bit[2] << " (" 
              << std::fixed << std::setprecision(5) << static_cast<double>(equal_ones_4bit[2]) / (count-3) << ")\n\n";
    
    // фак
    std::cout << "акф:\n";
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
