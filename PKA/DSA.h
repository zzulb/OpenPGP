/*
Input data should already be formatted and ready for hashing
*/

#include <vector>
#include <iostream>

#include "../common/cryptomath.h"
#include "../common/integer.h"
#include "../Hashes/Hashes.h"
#include "../RNG/RNG.h"
#include "../usehash.h"

#ifndef __DSA__
#define __DSA__
//  {p, q, g, y}
std::vector <integer> new_DSA_public(uint32_t L = 1024, uint32_t N = 160);
integer DSA_keygen(std::vector <integer> & pub);
std::vector <integer> DSA_sign(std::string & data, const std::vector <integer> & pub, const std::vector <integer> & pri);
bool DSA_verify(std::string & data, const std::vector <integer> & sig,const std::vector <integer> & pub);
#endif
