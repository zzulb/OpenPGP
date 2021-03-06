#include "Packets/Tag3.h"

namespace OpenPGP {
namespace Packet {

void Tag3::actual_read(const std::string & data, std::string::size_type & pos, const std::string::size_type & length) {
    set_version(data[pos + 0]);  // 4
    set_sym(data[pos + 1]);

    if (data[pos + 2] == S2K::ID::SIMPLE_S2K) {
        s2k = std::make_shared <S2K::S2K0> ();
    }
    else if (data[pos + 2] == S2K::ID::SALTED_S2K) {
        s2k = std::make_shared <S2K::S2K1> ();
    }
    else if (data[pos + 2] == 2) {
        throw std::runtime_error("S2K with ID 2 is reserved.");
    }
    else if (data[pos + 2] == S2K::ID::ITERATED_AND_SALTED_S2K) {
        s2k = std::make_shared <S2K::S2K3> ();
    }
    else{
        throw std::runtime_error("Unknown S2K ID encountered: " + std::to_string(data[0]));
    }

    const std::string::size_type orig_pos = pos;
    pos += 2; // include S2K type
    s2k -> read(data, pos);

    if (pos < (orig_pos + length)) {
        esk = std::make_shared <std::string> (data.substr(pos, orig_pos + length - pos));
    }

    pos = orig_pos + length;
}

void Tag3::show_contents(HumanReadable & hr) const {
    hr << "Version: " + std::to_string(version)
       << "Symmetric Key Algorithm: " + get_mapped(Sym::NAME, sym) + " (sym " + std::to_string(sym) + ")";
    if (s2k) {
        s2k -> show(hr);
    }
    else {
        hr << "";
    }
    if (esk) {
        hr << "Encrypted Session Key: " + hexlify(*esk);
    }
}

std::string Tag3::actual_raw() const {
    return std::string(1, version) + std::string(1, sym) + (s2k?s2k -> write():"") + (esk?*esk:"");
}

Status Tag3::actual_valid(const bool) const {
    if (version != 4) {
        return Status::INVALID_VERSION;
    }

    if (!Sym::valid(sym)) {
        return Status::INVALID_SYMMETRIC_ENCRYPTION_ALGORITHM;
    }

    if (!s2k) {
        return Status::MISSING_S2K;
    }

    return s2k->valid();
}

Tag3::Tag3()
    : Tag(SYMMETRIC_KEY_ENCRYPTED_SESSION_KEY, 4),
      sym(),
      s2k(nullptr),
      esk(nullptr)
{}

Tag3::Tag3(const Tag3 & copy)
    : Tag(copy),
      sym(copy.sym),
      s2k(copy.s2k?copy.s2k -> clone():nullptr),
      esk(copy.get_esk_clone())
{}

Tag3::Tag3(const std::string & data)
    : Tag3()
{
    read(data);
}

Tag3::~Tag3() {}

uint8_t Tag3::get_sym() const {
    return sym;
}

S2K::S2K::Ptr Tag3::get_s2k() const {
    return s2k;
}

S2K::S2K::Ptr Tag3::get_s2k_clone() const {
    return s2k -> clone();
}

std::shared_ptr <std::string> Tag3::get_esk() const {
    return esk;
}

std::shared_ptr <std::string> Tag3::get_esk_clone() const {
    return esk?std::make_shared <std::string> (*esk):nullptr;
}

std::string Tag3::get_session_key(const std::string & pass) const {
    std::string out = s2k -> run(pass, Sym::KEY_LENGTH.at(sym) >> 3);
    if (esk) {
        out = use_normal_CFB_decrypt(sym, *esk, out, std::string(Sym::BLOCK_LENGTH.at(sym) >> 3, 0));
    }
    else{
        out = std::string(1, sym) + out;
    }
    return out; // first octet is symmetric key algorithm. rest is session key
}

void Tag3::set_sym(const uint8_t s) {
    sym = s;
}

void Tag3::set_s2k(const S2K::S2K::Ptr & s) {
    if (!s) {
        throw std::runtime_error("Error: No S2K provided.\n");
    }

    if ((s -> get_type() != S2K::ID::SALTED_S2K)             &&
        (s -> get_type() != S2K::ID::ITERATED_AND_SALTED_S2K)) {
        throw std::runtime_error("Error: S2K must have a salt value.");
    }

    s2k = s -> clone();
}

void Tag3::set_esk(std::string * s) {
    if (s) {
        set_esk(*s);
    }
}

void Tag3::set_esk(const std::string & s) {
    esk = std::make_shared <std::string> (s);
}

void Tag3::set_session_key(const std::string & pass, const std::string & sk) {
    //sk should be [1 octet symmetric key algorithm] + [session key(s)]
    esk.reset();
    if (s2k && (sk.size() > 1)) {
        esk = std::make_shared <std::string> (use_normal_CFB_encrypt(sym, sk, s2k -> run(pass, Sym::KEY_LENGTH.at(sym) >> 3), std::string(Sym::BLOCK_LENGTH.at(sym) >> 3, 0)));
    }
}

Tag::Ptr Tag3::clone() const {
    Ptr out = std::make_shared <Packet::Tag3> (*this);
    out -> sym = sym;
    out -> s2k = s2k?s2k -> clone():nullptr;
    out -> esk = esk?std::make_shared <std::string> (*esk):nullptr;
    return out;
}

Tag3 & Tag3::operator=(const Tag3 & tag3) {
    Tag::operator=(tag3);
    sym = tag3.sym;
    s2k = tag3.s2k -> clone();
    esk = std::make_shared <std::string> (*tag3.esk);
    return *this;
}

}
}
