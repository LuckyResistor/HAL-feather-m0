//
// String
// ---------------------------------------------------------------------------
// (c)2018 by Lucky Resistor. See LICENSE for details.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "String.hpp"


#include <cstring>
#include <cstdlib>


namespace lr {


String::String() noexcept
    : _length(0), _capacity(0), _data(nullptr)
{
}


String::~String() noexcept
{
    if (_data != nullptr) {
        free(_data);
    }
}


String::String(const char *str) noexcept
    : _data(nullptr)
{
    _length = static_cast<Size>(std::strlen(str));
    reserve(_length);
    std::strcpy(_data, str);
}


String::String(const String &other) noexcept
    : _length(other._length), _capacity(other._length), _data(nullptr)
{
    reserve(other.getLength());
    std::memcpy(_data, other._data, _length+1);
}


String::String(String &&other) noexcept
    : _length(other._length), _capacity(other._length), _data(other._data)
{
    other._data = nullptr;
    other._length = 0;
    other._capacity = 0;
}


String& String::operator=(const String &other) noexcept
{
    if (_data != nullptr) {
        free(_data);
        _data = nullptr;
        _capacity = 0;
    }
    _length = other._length;
    reserve(_length);
    std::memcpy(_data, other._data, _length+1);
    return *this;
}


String& String::operator=(const char *str) noexcept
{
    if (_data != nullptr) {
        free(_data);
        _data = nullptr;
        _capacity = 0;
    }
    const Size strLen = std::strlen(str);
    _length = strLen;
    reserve(_length);
    std::strcpy(_data, str);
    return *this;
}


String& String::operator=(String &&other) noexcept
{
    _length = other._length;
    _capacity = other._length;
    _data = other._data;
    other._data = nullptr;
    other._length = 0;
    other._capacity = 0;
    return *this;
}


bool String::operator==(const String &other) const noexcept
{
    if (_data == nullptr && other._data == nullptr) {
        return true;
    } else if (_data == nullptr || other._data == nullptr) {
        return false;
    }
    return std::strcmp(_data, other._data) == 0;
}


bool String::operator==(const char *str) const noexcept
{
    if (_data == nullptr && str == nullptr) {
        return true;
    } else if (_data == nullptr || str == nullptr) {
        return false;
    }
    return std::strcmp(_data, str) == 0;
}


bool String::operator!=(const String &other) const noexcept
{
    return !operator==(other);
}


bool String::operator!=(const char *str) const noexcept
{
    return !operator==(str);
}


bool String::operator<(const String &other) const noexcept
{
    if (_data == nullptr && other._data != nullptr) {
        return true;
    } else if (_data == nullptr || other._data == nullptr) {
        return false;
    }
    return std::strcmp(_data, other._data) < 0;
}


bool String::operator<(const char *str) const noexcept
{
    if (_data == nullptr && str != nullptr) {
        return true;
    } else if (_data == nullptr || str == nullptr) {
        return false;
    }
    return std::strcmp(_data, str) < 0;
}


bool String::operator<=(const String &other) const noexcept
{
    return operator==(other)||operator<(other);
}


bool String::operator<=(const char *str) const noexcept
{
    return operator==(str)||operator<(str);
}


bool String::operator>(const String &other) const noexcept
{
    if (_data != nullptr && other._data == nullptr) {
        return true;
    } else if (_data == nullptr || other._data == nullptr) {
        return false;
    }
    return std::strcmp(_data, other._data) > 0;
}


bool String::operator>(const char *str) const noexcept
{
    if (_data != nullptr && str == nullptr) {
        return true;
    } else if (_data == nullptr || str == nullptr) {
        return false;
    }
    return std::strcmp(_data, str) > 0;
}


bool String::operator>=(const String &other) const noexcept
{
    return operator==(other)||operator>(other);
}


bool String::operator>=(const char *str) const noexcept
{
    return operator==(str)||operator>(str);
}


void String::append(const String &other) noexcept
{
    if (other.isEmpty()) {
        return;
    }
    const Size newLength = _length + other.getLength();
    reserve(newLength);
    std::strcpy(_data + _length, other._data);
    _length = newLength;
}


void String::append(const char *str) noexcept
{
    if (str == nullptr) {
        return;
    }
    const Size strLen = std::strlen(str);
    if (strLen == 0) {
        return;
    }
    const Size newLength = _length + strLen;
    reserve(newLength);
    std::strcpy(_data + _length, str);
    _length = newLength;
}


void String::append(char c) noexcept
{
    const Size newLength = _length + 1;
    reserve(newLength);
    _data[_length] = c;
    _data[_length+1] = '\0';
    _length = newLength;
}


bool String::isEmpty() const noexcept
{
    return _length == 0;
}


String::Size String::getLength() const noexcept
{
    return _length;
}


String::Size String::getCapacity() const noexcept
{
    return _capacity;
}


const char* String::getData() const noexcept
{
    return _data;
}


void String::reserve(Size capacity) noexcept
{
    if (capacity < 32) {
        // For strings below 32 bytes, reserve in 8 byte steps.
        capacity = ((capacity/4)+1)*4;
    } else if (capacity < 128) {
        // For strings below 128 bytes, reserve in 16 byte steps.
        capacity = ((capacity/16)+1)*16;
    } else {
        // For larger strings, reserve in 32 byte steps.
        capacity = ((capacity/32)+1)*32;
    }
    if (_capacity >= capacity) {
        return;
    }
    // Change or allocate the memory block.
    _data = static_cast<char*>(std::realloc(_data, capacity+1));
    // If this operation shortens the string, adjust the length and add a zero byte.
    if (capacity < _length) {
        _data[capacity] = '\0';
        _length = capacity;
    }
    _capacity = capacity;
}


void String::squeeze() noexcept
{
    if (_length != _capacity) {
        _capacity = _length;
        _data = static_cast<char*>(std::realloc(_data, _capacity+1));
    }
}


}

