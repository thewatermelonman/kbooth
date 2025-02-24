#ifndef ESC_POS_COMMANDS_H
#define ESC_POS_COMMANDS_H
#include <vector>
// ESC POS PRINTER COMMANDS

// Printer Initialization
const std::vector<unsigned char> ESC_Init = {0x1B, 0x40};

/**
 * Print Commands
 */
// Print and line feed
const std::vector<unsigned char> ESC_LF = {0x0A};

// Print and paper feed
const std::vector<unsigned char> ESC_J = {0x1b, 0x4a, 0x00};
const std::vector<unsigned char> ESC_d = {0x1b, 0x64, 0x00};

// Print self-test page
const std::vector<unsigned char> US_vt_eot = {0x1f, 0x11, 0x04};

// Beep command
const std::vector<unsigned char> ESC_B_m_n = {0x1b, 0x42, 0x00, 0x00};

// Cutter commands
const std::vector<unsigned char> GS_V_n = {0x1d, 0x56, 0x00};
const std::vector<unsigned char> GS_V_m_n = {0x1d, 0x56, 0x42, 0x00};
const std::vector<unsigned char> GS_i = {0x1b, 0x69};
const std::vector<unsigned char> GS_m = {0x1b, 0x6d};

/**
 * Character Setting Commands
 */
// Set character right spacing
const std::vector<unsigned char> ESC_SP = {0x1b, 0x20, 0x00};

// Set character font format
const std::vector<unsigned char> ESC_ExclamationMark = {0x1b, 0x21, 0x00};

// Set double height and width font
const std::vector<unsigned char> GS_ExclamationMark = {0x1d, 0x21, 0x00};

// Set reverse printing
const std::vector<unsigned char> GS_B = {0x1d, 0x42, 0x00};

// Enable/disable 90-degree rotated printing
const std::vector<unsigned char> ESC_V = {0x1b, 0x56, 0x00};

// Select font type (mainly ASCII)
const std::vector<unsigned char> ESC_M = {0x1b, 0x4d, 0x00};

// Enable/disable bold text
const std::vector<unsigned char> ESC_G = {0x1b, 0x47, 0x00};
const std::vector<unsigned char> ESC_E = {0x1b, 0x45, 0x00};

// Enable/disable inverted printing
const std::vector<unsigned char> ESC_LeftBrace = {0x1b, 0x7b, 0x00};

// Set underline height (characters)
const std::vector<unsigned char> ESC_Minus = {0x1b, 0x2d, 0x00};

// Character mode
const std::vector<unsigned char> FS_dot = {0x1c, 0x2e};

// Chinese character mode
const std::vector<unsigned char> FS_and = {0x1c, 0x26};

// Set Chinese character printing mode
const std::vector<unsigned char> FS_ExclamationMark = {0x1c, 0x21, 0x00};

// Set underline height (Chinese characters)
const std::vector<unsigned char> FS_Minus = {0x1c, 0x2d, 0x00};

// Set Chinese character left and right spacing
const std::vector<unsigned char> FS_S = {0x1c, 0x53, 0x00, 0x00};

// Select character code page
const std::vector<unsigned char> ESC_t = {0x1b, 0x74, 0x00};

/**
 * Format Setting Commands
 */
// Set default line spacing
const std::vector<unsigned char> ESC_Two = {0x1b, 0x32};

// Set line spacing
const std::vector<unsigned char> ESC_Three = {0x1b, 0x33, 0x00};

// Set alignment mode
const std::vector<unsigned char> ESC_Align = {0x1b, 0x61, 0x00};

// Set left margin
const std::vector<unsigned char> GS_LeftSp = {0x1d, 0x4c, 0x00, 0x00};

// Set absolute print position
const std::vector<unsigned char> ESC_Relative = {0x1b, 0x24, 0x00, 0x00};

// Set relative print position
const std::vector<unsigned char> ESC_Absolute = {0x1b, 0x5c, 0x00, 0x00};

// Set print area width
const std::vector<unsigned char> GS_W = {0x1d, 0x57, 0x00, 0x00};

/**
 * Status Commands
 */
// Real-time status transmission command
const std::vector<unsigned char> DLE_eot = {0x10, 0x04, 0x00};

// Real-time cash drawer command
const std::vector<unsigned char> DLE_DC4 = {0x10, 0x14, 0x00, 0x00, 0x00};

// Standard cash drawer command
const std::vector<unsigned char> ESC_p = {0x1b, 0x70, 0x00, 0x00, 0x00};

/**
 * Barcode Setting Commands
 */
// Select HRI print mode
const std::vector<unsigned char> GS_H = {0x1d, 0x48, 0x00};

// Set barcode height
//const std::vector<unsigned char> GS_h = {0x1d, 0x68, 0xa2};

// Set barcode width
const std::vector<unsigned char> GS_w = {0x1d, 0x77, 0x00};

// Set HRI font type
const std::vector<unsigned char> GS_f = {0x1d, 0x66, 0x00};

// Barcode left offset command
const std::vector<unsigned char> GS_x = {0x1d, 0x78, 0x00};

// Print barcode command
const std::vector<unsigned char> GS_k = {0x1d, 0x6b, 0x41, 0x0c};

// QR code related command
const std::vector<unsigned char> GS_k_m_v_r_nL_nH = {0x1b, 0x5a, 0x03, 0x03, 0x08, 0x00, 0x00};
#endif // ESC_POS_COMMANDS_H
