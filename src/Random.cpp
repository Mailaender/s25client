// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#include "defines.h" // IWYU pragma: keep
#include "Random.h"
#include "libutil/src/Serializer.h"
#include <boost/filesystem/fstream.hpp>

template<class T_PRNG>
Random<T_PRNG>::Random()
{
    Init(123456789);
}

template<class T_PRNG>
void Random<T_PRNG>::Init(const uint64_t& seed)
{
    ResetState(PRNG(static_cast<typename PRNG::result_type>(seed)));
}

template<class T_PRNG>
void Random<T_PRNG>::ResetState(const PRNG& newState)
{
    rng_ = newState;
    numInvocations_ = 0;
}

template<class T_PRNG>
int Random<T_PRNG>::Rand(const char* const src_name, const unsigned src_line, const unsigned obj_id, const int max)
{
    history_[numInvocations_ % history_.size()] = RandomEntry(numInvocations_, max, rng_, src_name, src_line, obj_id);
    ++numInvocations_;
    
    // Special case: [0, 0) makes 0
    return (max == 0) ? 0 : rng_(max - 1);
}

template<class T_PRNG>
unsigned Random<T_PRNG>::GetChecksum() const
{
    return CalcChecksum(rng_);
}

template<class T_PRNG>
unsigned Random<T_PRNG>::CalcChecksum(const PRNG& rng)
{
    // This is designed in a way that makes the checksum be equivalent
    // to the state of the OldLCG for compatibility with old versions
    Serializer ser;
    rng.Serialize(ser);
    unsigned checksum = 0;
    while(ser.GetBytesLeft() >= sizeof(unsigned))
        checksum += ser.PopUnsignedInt();
    while(ser.GetBytesLeft())
        checksum += ser.PopUnsignedChar();
    return checksum;
}

template<class T_PRNG>
const typename Random<T_PRNG>::PRNG& Random<T_PRNG>::GetCurrentState() const
{
    return rng_;
}

template<class T_PRNG>
std::vector<typename Random<T_PRNG>::RandomEntry> Random<T_PRNG>::GetAsyncLog()
{
    std::vector<RandomEntry> ret;

    unsigned begin, end;
    if(numInvocations_ > history_.size())
    {
        // Ringbuffer filled -> Start from next entry (which is the one written longest time ago)
        // and go one full cycle (to the entry written last)
        begin = numInvocations_;
        end = numInvocations_ + history_.size();
    } else
    {
        // Ringbuffer not filled -> Start from 0 till number of entries
        begin = 0;
        end = numInvocations_;
    }

    ret.reserve(end - begin);
    for (unsigned i = begin; i < end; ++i)
        ret.push_back(history_[i % history_.size()]);

    return ret;
}

template<class T_PRNG>
void Random<T_PRNG>::SaveLog(const std::string& filename)
{
    const std::vector<RandomEntry> log = GetAsyncLog();
    bfs::ofstream file(filename);

    for(typename std::vector<RandomEntry>::const_iterator it = log.begin(); it != log.end(); ++it)
        file << *it;
}

//////////////////////////////////////////////////////////////////////////

template<class T_PRNG>
void Random<T_PRNG>::RandomEntry::Serialize(Serializer& ser) const
{
    ser.PushUnsignedInt(counter);
    ser.PushSignedInt(max);
    rngState.Serialize(ser);
    ser.PushString(src_name);
    ser.PushUnsignedInt(src_line);
    ser.PushUnsignedInt(obj_id);
}

template<class T_PRNG>
void Random<T_PRNG>::RandomEntry::Deserialize(Serializer& ser)
{
    counter = ser.PopUnsignedInt();
    max = ser.PopSignedInt();
    rngState.Deserialize(ser);
    src_name = ser.PopString();
    src_line = ser.PopUnsignedInt();
    obj_id = ser.PopUnsignedInt();
}

template<class T_PRNG>
int Random<T_PRNG>::RandomEntry::GetValue() const
{
    PRNG tmpRng(rngState);
    return (max == 0) ? 0 : tmpRng(max - 1);
}

template<class T_PRNG>
std::ostream& Random<T_PRNG>::RandomEntry::print(std::ostream& os) const
{
    return os << counter << ":R(" << max << ")=" << GetValue() << ",z=" << rngState
        << " | " << src_name << "#" << src_line
        << "|id=" << obj_id << "\n";
}

// Instantiate the Random class with the used PRNG
template class Random<UsedPRNG>;
