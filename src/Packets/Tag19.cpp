#include "Packets/Tag19.h"

namespace OpenPGP {
namespace Packet {

void Tag19::actual_read(const std::string & data, std::string::size_type & pos, const std::string::size_type & length) {
    set_hash(data.substr(pos, length));
    pos += length;
}

void Tag19::show_contents(HumanReadable & hr) const {
    hr << "SHA - 1 Hash of previous packet: " + hash;
}

std::string Tag19::actual_raw() const {
    return hash;
}

Status Tag19::actual_valid(const bool) const {
    if (hash.size() != (Hash::LENGTH.at(Hash::ID::SHA1) >> 3)) {
        return Status::INVALID_SHA1_HASH;
    }

    return Status::SUCCESS;
}

Tag19::Tag19()
    : Tag(MODIFICATION_DETECTION_CODE),
      hash()
{
    size = Hash::LENGTH.at(Hash::ID::SHA1) >> 3;
}

Tag19::Tag19(const std::string & data)
    : Tag19()
{
    read(data);
}

std::string Tag19::get_hash() const {
    return hash;
}

void Tag19::set_hash(const std::string & h) {
    hash = h;
}

Tag::Ptr Tag19::clone() const {
    return std::make_shared <Packet::Tag19> (*this);
}

}

}
