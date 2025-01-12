#include <stdio.h>

// We'll use unsigned char (0-255) for 8-bit values
// and unsigned int for 10-bit key
typedef unsigned char byte;
typedef unsigned int uint;

// --------- Lookup Tables for Permutations ---------
// Each number represents the position from which to take the bit (reading from right to left)
// For example, in IP[0] = 2 means "take the 2nd bit and put it in position 1"
byte IP[8] = {2, 6, 3, 1, 4, 8, 5, 7};        // Initial Permutation
byte IP_inverse[8] = {4, 1, 3, 5, 7, 2, 8, 6}; // Final Permutation

// S-boxes for substitution
// These are simple lookup tables - given row and column, they return a 2-bit value
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

// --------- Helper Functions ---------

// Get a specific bit from a number (position starts from right, 1-based)
byte get_bit(byte data, int position) {
    return (data >> (position - 1)) & 1;
}

// Set a specific bit in a number (position starts from right, 1-based)
byte set_bit(byte data, int position, byte value) {
    if (value)
        return data | (1 << (position - 1));
    else
        return data & ~(1 << (position - 1));
}

// Apply any permutation to data using a lookup table
byte permute(byte data, byte* permutation_table, int size) {
    byte result = 0;
    for (int i = 0; i < size; i++) {
        // Get bit from the position specified in permutation table
        byte bit = get_bit(data, permutation_table[i]);
        // Set this bit in the result (position i+1)
        result = set_bit(result, i + 1, bit);
    }
    return result;
}

// Split a byte into two halves (4 bits each)
void split_byte(byte data, byte* left, byte* right) {
    *left = (data >> 4) & 0x0F;   // Get upper 4 bits
    *right = data & 0x0F;         // Get lower 4 bits
}

// Combine two 4-bit values into one byte
byte combine_bytes(byte left, byte right) {
    return (left << 4) | right;
}

// The main function used in encryption (called F-function)
byte f_function(byte right_half, byte key) {
    // Expand right half from 4 to 8 bits (we'll just duplicate some bits)
    byte expanded = ((right_half & 0x0F) << 4) | (right_half & 0x0F);
    
    // XOR with key
    byte xored = expanded ^ key;
    
    // Split into two 4-bit parts for S-box lookup
    byte left, right;
    split_byte(xored, &left, &right);
    
    // Use S-boxes to convert each 4-bit part into 2-bit part
    // For simplicity, we'll use basic lookup
    byte s1_result = SBOX1[left >> 2][left & 0x03];
    byte s2_result = SBOX2[right >> 2][right & 0x03];
    
    // Combine results into 4 bits
    return (s1_result << 2) | s2_result;
}

// --------- Main Encryption Function ---------
byte encrypt(byte plaintext, uint key) {
    // Step 1: Apply initial permutation
    byte current_data = permute(plaintext, IP, 8);
    
    // Step 2: Split into left and right halves
    byte left, right;
    split_byte(current_data, &left, &right);
    
    // For simplicity, we'll use the same key for both rounds
    // In real SDES, you would generate two different subkeys
    byte round_key = key & 0xFF;  // Just use the lower 8 bits of the key
    
    // Step 3: First round
    byte f_result = f_function(right, round_key);
    byte new_right = left ^ f_result;
    byte new_left = right;
    
    // Step 4: Swap halves
    left = new_left;
    right = new_right;
    
    // Step 5: Second round
    f_result = f_function(right, round_key);
    new_right = left ^ f_result;
    new_left = right;
    
    // Step 6: Combine halves
    current_data = combine_bytes(new_left, new_right);
    
    // Step 7: Apply final permutation
    return permute(current_data, IP_inverse, 8);
}

// Example usage
int main() {
    // Example input values
    byte plaintext = 0b11010111;  // Binary: 11010111
    uint key = 0b1010000010;      // Binary: 1010000010
    
    // Encrypt
    byte ciphertext = encrypt(plaintext, key);
    
    // Print results in binary
    printf("Plaintext:  ");
    for(int i = 7; i >= 0; i--) 
        printf("%d", (plaintext >> i) & 1);
    printf("\n");
    
    printf("Key:        ");
    for(int i = 9; i >= 0; i--) 
        printf("%d", (key >> i) & 1);
    printf("\n");
    
    printf("Ciphertext: ");
    for(int i = 7; i >= 0; i--) 
        printf("%d", (ciphertext >> i) & 1);
    printf("\n");
    
    return 0;
}