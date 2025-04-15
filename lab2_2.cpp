#include <iostream>
#include <fstream>

int main() {
    const char* file_name = "text.txt"; 
    uint32_t rlz_1 = 0x007b3c1a;  
    uint32_t rlz_2[4] = {0x003b2c1d, 0x7e8f9a0b, 0x5c6d7e8f, 0x1a2b3c4a};
    uint32_t bit1 = 0;
    uint32_t bit2 = 0;
    
    std::ifstream file(file_name, std::ios::binary);
    
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);  
    
    std::cout << "Количество символов: " << size << std::endl;
    int P[256]{0}, C[256]{0};
    unsigned char byte, masked_byte, psp_byte;
    
    while (file.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
        psp_byte = 0;
        for(int i = 0; i < 8; i++) {
           bit1 = ((rlz_1 >> 6) & 0x01) ^ ((rlz_1 >> 3) & 0x01);
           rlz_1 = (rlz_1 >> 1) | (bit1 << 22);  //   
           bit2 = ((rlz_2[3] >> (111 - 96)) & 0x01) ^ ((rlz_2[3] >> (119 - 96)) & 0x01);
              for(int j = 0; j < 3; j++) {
                  uint32_t carry = rlz_2[j+1] & 0x01;
                  rlz_2[j] = (rlz_2[j] >> 1) | (carry << 31);
              }
           rlz_2[3] = (rlz_2[3] >> 1) | (bit2 << 22);  
           uint32_t psp_bit = (rlz_1 ^ rlz_2[0]) & 0x01;
           psp_byte = (psp_byte << 1) | psp_bit;
        }
        masked_byte = psp_byte ^ byte;
        P[byte]++;
        C[masked_byte]++;
    }		
    
    std::cout << "номер" << "\t" << "до_маск-ния" << "\t" << "после_маск-ния" << std::endl;
    for(int i = 0; i < 256; i++) {  // Исправлено 255 на 256
        std::cout << i << "\t" << (double)P[i]/size << "\t" << "\t" << (double)C[i]/size << std::endl;
    }    
    int sum_P = 0;
    int sum_C = 0;
    for(int i = 0; i < 256; i++) {  // Исправлено 255 на 256
        sum_P += P[i];
        sum_C += C[i];
    }
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "сумма" << "\t" << sum_P << "\t\t" << sum_C << std::endl;
    
    return 0;
}
