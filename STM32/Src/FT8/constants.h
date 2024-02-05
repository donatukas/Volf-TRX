#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <stdint.h>

#define true 1
#define false 0

// Define LDPC parameters
#define FT8_LDPC_N (174)                        ///< Number of bits in the encoded message (payload with LDPC checksum bits)
#define FT8_LDPC_K (91)                         ///< Number of payload bits (including CRC)
#define FT8_LDPC_M (83)                         ///< Number of LDPC checksum bits (FT8_LDPC_N - FT8_LDPC_K)
#define FT8_LDPC_N_BYTES ((FT8_LDPC_N + 7) / 8) ///< Number of whole bytes needed to store 174 bits (full message)
#define FT8_LDPC_K_BYTES ((FT8_LDPC_K + 7) / 8) ///< Number of whole bytes needed to store 91 bits (payload + CRC only)

extern int K_BYTES;
extern uint8_t tones[79];

// Costas 7x7 tone pattern
extern const uint8_t kCostas_map[7];

// Gray code map
extern const uint8_t kGray_map[8];

// Parity generator matrix for (174,91) LDPC code, stored in bitpacked format (MSB first)
// extern const uint8_t kGenerator[M][K_BYTES];
extern uint8_t kGenerator[83][12];

void initalize_constants(void);

/// Column order (permutation) in which the bits in codeword are stored
/// (Not really used in FT8 v2 - instead the Nm, Mn and generator matrices are already permuted)
extern const uint8_t kFT8_LDPC_column_order[FT8_LDPC_N];

/// LDPC(174,91) parity check matrix, containing 83 rows,
/// each row describes one parity check,
/// each number is an index into the codeword (1-origin).
/// The codeword bits mentioned in each row must xor to zero.
/// From WSJT-X's ldpc_174_91_c_reordered_parity.f90.
extern const uint8_t kFT8_LDPC_Nm[FT8_LDPC_M][7];

/// Mn from WSJT-X's bpdecode174.f90. Each row corresponds to a codeword bit.
/// The numbers indicate which three parity checks (rows in Nm) refer to the codeword bit.
/// The numbers use 1 as the origin (first entry).
extern const uint8_t kFT8_LDPC_Mn[FT8_LDPC_N][3];

/// Number of rows (columns in C/C++) in the array Nm.
extern const uint8_t kFT8_LDPC_num_rows[FT8_LDPC_M];

#endif /* CONSTANTS_H_ */
