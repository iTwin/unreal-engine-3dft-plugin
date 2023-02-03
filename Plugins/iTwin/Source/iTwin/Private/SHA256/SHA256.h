#pragma once

class SHA256 {

public:
	SHA256();
	
	void update(const uint8_t * data, size_t length);
	
	uint8_t *digest();

private:
	uint8_t  m_data[64];
	uint32_t m_blocklen;
	uint64_t m_bitlen;
	uint32_t m_state[8]; //A, B, C, D, E, F, G, H

	void transform();
	void pad();
	void revert(uint8_t * hash);
};
