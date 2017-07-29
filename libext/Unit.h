#pragma once

namespace libext
{
struct Unit
{
    template <class T>
    using Lift = std::conditional<std::is_same<T, void>::value, Unit, T>;
    template <class T>
    using Drop = std::conditional<std::is_same<T, Unit>::value, void, T>;

    bool operator ==(const Unit&) const { return true; }
    bool operator !=(const Unit&) const { return false; }
};

}
