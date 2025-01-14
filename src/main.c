#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>

char *BASE64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// This table implements the mapping from 8-bit ascii value to 6-bit
int UNBASE64[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0-11
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 12-23
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 24-35
    -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, // 36-47
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -2, // 48-59
    -1,  0, -1, -1, -1,  0,  1,  2,  3,  4,  5,  6, // 60-71
     7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, // 72-83
    19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, // 84-95
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, // 96-107
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, // 108-119
    49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 120-131
  };

// -------------------------
// Function Signatures

void base64_encode(const char *in, const unsigned long in_len, char *out);
int base64_decode(const char *in, const unsigned long in_len, char *out);

// -------------------------
// Core Implementation

void base64_encode(const char *in, const unsigned long in_len, char *out) {
    int in_index = 0;
    int out_index = 0;

    while (in_index < in_len) {
    // Process group of 24 bit
	    
	// First 6-bit
	out[out_index] = BASE64[(in[in_index] & 0xFC) >> 2];
	out_index++;

	if ((in_index + 1) == in_len) {
	    // First Padding Case
	    out[out_index++] = BASE64[((in[in_index] & 0x3) << 4)];
	    out[out_index++] = '=';
	    out[out_index++] = '=';
	    break;
	}

	// Second 6-bit
	out[out_index] = BASE64[((in[in_index] & 0x3) << 4) | ((in[in_index+1] & 0xF0) >> 4)];
	out_index++;

	if ((in_index + 2) == in_len) {
	   // Second Padding Case
	   out[out_index++] = BASE64[((in[in_index + 1] & 0xF) << 2)];
	   out[out_index++] = '='; 
	   break;
	}
	
	// Third 6-bit
	out[out_index] = BASE64[((in[in_index+1] & 0xF) << 2) | ((in[in_index+2] & 0xC0) >> 6)];
	out_index++;

	// Fourth 6-bit
	out[out_index] = BASE64[in[in_index+2] & 0x3F];
	out_index++;

	in_index += 3;
    }

    out[out_index] = '\0';

    return;
}

int base64_decode(const char *in, const unsigned long in_len, char *out) {
    int in_index = 0;
    int out_index = 0;
    char first, second, third, fourth;

    assert(!(in_len & 0x03)); // input must be even multiple of 4

    while (in_index < in_len) {
        // Check if input is valid base64
	for (int i = 0; i < 4; i++) {
            if (((int)in[in_index + i] > 131) || UNBASE64[(int)in[in_index + i]] == -1) {
	        fprintf(stderr, "Invalid base64 char, cannot decode: %c\n", in[in_index + i]);
	        return -1;
	    }
	}

	// Extract all Bits and reconstruct original Bytes
	first   = UNBASE64[(int) in[in_index]];
	second = UNBASE64[(int) in[in_index+1]];
	third  = UNBASE64[(int) in[in_index+2]];
	fourth = UNBASE64[(int) in[in_index+3]];

	// Reconstruct First, Second and Third Byte
	out[out_index] = (first << 2) | ((second & 0x30) >> 4);
	out_index++;

	if (in[in_index + 2] != '=') {
	    out[out_index++] = ((second & 0xF) << 4) | ((third & 0x3C) >> 2);
	}

	if (in[in_index + 3] != '=') {
	    out[out_index++] = ((third & 0x3) << 6) | fourth;
	}
	
	in_index += 4;
    }

    out[out_index] = '\0';
    return 0;
}

// -------------------------
// Testing Helpers

int base64_encode_test(const char *in, const char *expected_out) {
    unsigned long in_len = strlen(in);
    unsigned long out_len = 4 * ceil(in_len / 3.0) + 1;
    char *out = malloc(out_len);

    base64_encode(in, in_len, out);

    char result = memcmp(out, expected_out, out_len);

    if (result == 0) {
        fprintf(stdout, "[ENCODE-OK] %s\n Expected: %s\n Obtained: %s\n", in, expected_out, out);
    } else {
    	fprintf(stderr, "[ENCODE-ERR] %s\n Expected: %s\n Obtained: %s\n", in, expected_out, out);
    }

    free(out);
    return result;
}

int base64_decode_test(const char *in, const char *expected_out) {
    unsigned long in_len = strlen(in);
    unsigned long padding = in[in_len-1] == '=' ? (in[in_len-1] == '=' ? 2 : 1) : 0;
    unsigned long out_len = 3 * ceil(in_len / 4.0) - padding + 1;
    char *out = malloc(out_len);

    base64_decode(in, in_len, out);

    char result = memcmp(out, expected_out, out_len);

    if (result == 0) {
      fprintf(stdout, "[encode-OK] %s\n Expected:%s\n Obtained: %s\n", in, expected_out, out);
    } else {
      fprintf(stderr, "[encode-ERR] %s\n Expected:%s\n Obtained: %s\n", in, expected_out, out);
    }

    free(out);
    return result;
}

void test(void) {
    char inputs[8][20] =  {
      "Hello World!",
      "a",
      "aa",
      "aaa",
      "aaaa",
      "aaaaa",
      "aaaaaa",
      "aaaaaaa"
    };

    char outputs[8][20] = {
      "SGVsbG8gV29ybGQh",
      "YQ==",
      "YWE=",
      "YWFh",
      "YWFhYQ==",
      "YWFhYWE=",
      "YWFhYWFh",
      "YWFhYWFhYQ=="
    };

    for (int i = 0; i < 8; i++) {
      base64_encode_test(inputs[i], outputs[i]);
      base64_decode_test(outputs[i], inputs[i]);
    }
}

int main(int argc, char **argv) {
    char *option = NULL;
    bool test_mode = false;   // -t
    bool decode_mode = false; // -d

    // Parse Arguments
    if (argc == 2) {
        option = argv[1];
    }

    if (option) {
        assert(strlen(option) == 2 && "Invalid length of options!");
        test_mode = memcmp(option, "-t", 2) == 0;
        decode_mode = memcmp(option, "-d", 2) == 0;

        if (!test_mode && !decode_mode) {
            fprintf(stderr, "[ERROR]: Options allowed are: `-t` and `-d`\n");
            return 1;
        }
  }

    while (!feof(stdin)) {
        char *input = readline("> ");
	if (!input) { break; }
        unsigned long in_len = strlen(input);
        char *output = NULL;

        if (decode_mode) {
            unsigned long padding = input[in_len-1] == '=' ? (input[in_len-1] == '=' ? 2 : 1) : 0;
            unsigned long out_len = 3 * ceil(in_len / 4.0) - padding + 1;
            output = malloc(out_len);
            base64_decode(input, in_len, output);
        } else {
            unsigned long out_len = 4 * ceil(in_len / 3.0) + 1;
            output = malloc(out_len);
            base64_encode(input, in_len, output);
	}

	printf("%s\n", output);

	free(input);
	free(output);
    }

    return 0;
}
