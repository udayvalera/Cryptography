#include <stdio.h>

typedef unsigned char byte;
typedef unsigned int uint;

byte IP[8] = {2, 6, 3, 1, 4, 8, 5, 7};
byte IP_inverse[8] = {4, 1, 3, 5, 7, 2, 8, 6};

byte SBOX1[4][4] = {
    {1, 0, 3, 2},
    {3, 2, 1, 0},
    {0, 2, 1, 3},
    {3, 1, 0, 2}
};

byte SBOX2[4][4] = {
    {0, 1, 2, 3},
    {2, 3, 1, 0},
    {3, 0, 1, 2},
    {2, 1, 0, 3}
};

byte get_bit(byte data, int position) {
    return (data >> (position - 1)) & 1;
}

byte set_bit(byte data, int position, byte value) {
    if (value)
        return data | (1 << (position - 1));
    else
        return data & ~(1 << (position - 1));
}

byte permute(byte data, byte* permutation_table, int size) {
    byte result = 0;
    for (int i = 0; i < size; i++) {
        byte bit = get_bit(data, permutation_table[i]);
        result = set_bit(result, i + 1, bit);
    }
    return result;
}

void split_byte(byte data, byte* left, byte* right) {
    *left = (data >> 4) & 0x0F;
    *right = data & 0x0F;
}

byte combine_bytes(byte left, byte right) {
    return (left << 4) | right;
}

byte f_function(byte right_half, byte key) {
    byte expanded = ((right_half & 0x0F) << 4) | (right_half & 0x0F);
    byte xored = expanded ^ key;
    byte left, right;
    split_byte(xored, &left, &right);
    byte s1_result = SBOX1[left >> 2][left & 0x03];
    byte s2_result = SBOX2[right >> 2][right & 0x03];
    return (s1_result << 2) | s2_result;
}

byte encrypt(byte plaintext, uint key) {
    byte current_data = permute(plaintext, IP, 8);
    byte left, right;
    split_byte(current_data, &left, &right);
    byte round_key = key & 0xFF;
    byte f_result = f_function(right, round_key);
    byte new_right = left ^ f_result;
    byte new_left = right;
    left = new_left;
    right = new_right;
    f_result = f_function(right, round_key);
    new_right = left ^ f_result;
    new_left = right;
    current_data = combine_bytes(new_left, new_right);
    return permute(current_data, IP_inverse, 8);
}

int main() {
    byte plaintext = 0b11010111;
    byte target_ciphertext = encrypt(plaintext, 0b1010000010);

    printf("Target Ciphertext: ");
    for(int i = 7; i >= 0; i--) 
        printf("%d", (target_ciphertext >> i) & 1);
    printf("\n");

    printf("Brute-forcing keys...\n");
    for (uint candidate_key = 0; candidate_key < 1024; candidate_key++) {
        byte candidate_cipher = encrypt(plaintext, candidate_key);
        if (candidate_cipher == target_ciphertext) {
            printf("Found key: ");
            for(int i = 9; i >= 0; i--) 
                printf("%d", (candidate_key >> i) & 1);
            printf("\n");
            break; // Stop iterating after finding the first valid key
        }
    }
    
    return 0;
}