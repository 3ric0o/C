#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/*
 * Jenkins one-at-a-time hash function
 * Parameters:
 * - key: pointer to the input data
 * - len: length of the input data in bytes
 * Returns: 32-bit hash value
 */

uint32_t jenkins_hash(const void* key, size_t len)
// key: A pointer to the data to be hashed (can be any type)
// len: The length of the data in bytes
// The function returns a 32-bit unsigned integer

{
    const uint8_t* data = (const uint8_t*)key;
    // Converts the input pointer to a byte pointer
    // This allows us to process the input data one byte at a time
    // uint8_t ensures we're working with unsigned 8-bit integers

    uint32_t hash = 0;
    size_t i;

    // Phase 1: Process each byte of input
    for (i = 0; i < len; ++i) {
        hash += data[i];           // Step 1: Add byte to hash
        hash += (hash << 10);      // Step 2: Add hash shifted left by 10
        hash ^= (hash >> 6);       // Step 3: XOR with hash shifted right by 6
    }

    // Phase 2: Avalanche effect - final mixing
    hash += (hash << 3);           // Mix 1: Add hash shifted left by 3
    hash ^= (hash >> 11);          // Mix 2: XOR with hash shifted right by 11
    hash += (hash << 15);          // Mix 3: Add hash shifted left by 15

    return hash;
}


// Function to verify credentials
bool verify_credentials(const char* input_username, const char* input_password,
                        uint32_t stored_username_hash, uint32_t stored_password_hash){

    // Calculate hashes of input credentials
    uint32_t input_username_hash = jenkins_hash(input_username, strlen(input_username));
    uint32_t input_password_hash = jenkins_hash(input_password, strlen(input_password));

    // Compare the hashes
    return (input_username_hash == stored_username_hash && input_password_hash == stored_password_hash);
}


int main()
{
    // Store the correct credentials' hashes (this would normally be in a database)
    const char* CORRECT_USERNAME = "Erik";
    const char* CORRECT_PASSWORD = "HashMap123";

    uint32_t stored_username_hash = jenkins_hash(CORRECT_USERNAME, strlen(CORRECT_USERNAME));
    uint32_t stored_password_hash = jenkins_hash(CORRECT_PASSWORD, strlen(CORRECT_PASSWORD));

    // Simulating login attempts
    char input_username[50];
    char input_password[50];

    printf("Enter username: ");
    scanf("%49s", input_username);
    printf("Enter password: ");
    scanf("%49s", input_password);

    // Verify the credentials
    if (verify_credentials(input_username, input_password,stored_username_hash, stored_password_hash))
    {
        printf("Login successful!\n");
    }
    else
    {
        printf("Invalid username or password!\n");
    }

    // For demonstration purposes, print the hashes
    printf("\nDEBUG Information:\n");
    printf("Stored Username Hash: %u\n", stored_username_hash);
    printf("Input Username Hash: %u\n",jenkins_hash(input_username, strlen(input_username)));
    printf("Stored Password Hash: %u\n", stored_password_hash);
    printf("Input Password Hash: %u\n",jenkins_hash(input_password, strlen(input_password)));

    return 0;
}

/* TLDR of a Jenkins one-at-a-time hash function

                Avalanche Effect:
 * Small input changes create significant output changes
 * Changing one bit in input typically changes about half the bits in output

                Distribution:
 * Produces well-distributed hash values
 * Good for hash tables and checksums

                Performance:
 * Fast computation with simple integer operations
 * No large lookup tables needed
 * Cache-friendly due to sequential memory access

                Weaknesses:
 * Not cryptographically secure
 * Should not be used for security-critical applications
 * Better alternatives exist for cryptographic purposes (like SHA-256)

            Best Use Cases:
 * Hash tables
 * Data integrity checks
 * Quick string comparison
 * Caching systems
 * Non-cryptographic data fingerprinting

*/
