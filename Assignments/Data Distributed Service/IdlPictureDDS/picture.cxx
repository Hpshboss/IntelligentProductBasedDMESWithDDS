// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file picture.cpp
 * This source file contains the definition of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifdef _WIN32
// Remove linker warning LNK4221 on Visual Studio
namespace { char dummy; }
#endif

#include "picture.h"
#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>

picture::picture()
{
    // m_index com.eprosima.idl.parser.typecode.PrimitiveTypeCode@a7e666
    m_index = 0;
    // m_size com.eprosima.idl.parser.typecode.PrimitiveTypeCode@68bbe345
    m_size = 0;
    // m_note com.eprosima.idl.parser.typecode.StringTypeCode@30b8a058
    m_note ="";
    // m_buffer com.eprosima.idl.parser.typecode.SequenceTypeCode@7494e528


}

picture::~picture()
{




}

picture::picture(const picture &x)
{
    m_index = x.m_index;
    m_size = x.m_size;
    m_note = x.m_note;
    m_buffer = x.m_buffer;
}

picture::picture(picture &&x)
{
    m_index = x.m_index;
    m_size = x.m_size;
    m_note = std::move(x.m_note);
    m_buffer = std::move(x.m_buffer);
}

picture& picture::operator=(const picture &x)
{

    m_index = x.m_index;
    m_size = x.m_size;
    m_note = x.m_note;
    m_buffer = x.m_buffer;

    return *this;
}

picture& picture::operator=(picture &&x)
{

    m_index = x.m_index;
    m_size = x.m_size;
    m_note = std::move(x.m_note);
    m_buffer = std::move(x.m_buffer);

    return *this;
}

size_t picture::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255 + 1;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    current_alignment += (5000000 * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);




    return current_alignment - initial_alignment;
}

size_t picture::getCdrSerializedSize(const picture& data, size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.note().size() + 1;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    if (data.buffer().size() > 0)
    {
        current_alignment += (data.buffer().size() * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
    }




    return current_alignment - initial_alignment;
}

void picture::serialize(eprosima::fastcdr::Cdr &scdr) const
{

    scdr << m_index;
    scdr << m_size;
    scdr << m_note;
    scdr << m_buffer;
}

void picture::deserialize(eprosima::fastcdr::Cdr &dcdr)
{

    dcdr >> m_index;
    dcdr >> m_size;
    dcdr >> m_note;
    dcdr >> m_buffer;
}

/*!
 * @brief This function sets a value in member index
 * @param _index New value for member index
 */
void picture::index(uint32_t _index)
{
m_index = _index;
}

/*!
 * @brief This function returns the value of member index
 * @return Value of member index
 */
uint32_t picture::index() const
{
    return m_index;
}

/*!
 * @brief This function returns a reference to member index
 * @return Reference to member index
 */
uint32_t& picture::index()
{
    return m_index;
}

/*!
 * @brief This function sets a value in member size
 * @param _size New value for member size
 */
void picture::size(uint32_t _size)
{
m_size = _size;
}

/*!
 * @brief This function returns the value of member size
 * @return Value of member size
 */
uint32_t picture::size() const
{
    return m_size;
}

/*!
 * @brief This function returns a reference to member size
 * @return Reference to member size
 */
uint32_t& picture::size()
{
    return m_size;
}

/*!
 * @brief This function copies the value in member note
 * @param _note New value to be copied in member note
 */
void picture::note(const std::string &_note)
{
m_note = _note;
}

/*!
 * @brief This function moves the value in member note
 * @param _note New value to be moved in member note
 */
void picture::note(std::string &&_note)
{
m_note = std::move(_note);
}

/*!
 * @brief This function returns a constant reference to member note
 * @return Constant reference to member note
 */
const std::string& picture::note() const
{
    return m_note;
}

/*!
 * @brief This function returns a reference to member note
 * @return Reference to member note
 */
std::string& picture::note()
{
    return m_note;
}
/*!
 * @brief This function copies the value in member buffer
 * @param _buffer New value to be copied in member buffer
 */
void picture::buffer(const std::vector<uint8_t> &_buffer)
{
m_buffer = _buffer;
}

/*!
 * @brief This function moves the value in member buffer
 * @param _buffer New value to be moved in member buffer
 */
void picture::buffer(std::vector<uint8_t> &&_buffer)
{
m_buffer = std::move(_buffer);
}

/*!
 * @brief This function returns a constant reference to member buffer
 * @return Constant reference to member buffer
 */
const std::vector<uint8_t>& picture::buffer() const
{
    return m_buffer;
}

/*!
 * @brief This function returns a reference to member buffer
 * @return Reference to member buffer
 */
std::vector<uint8_t>& picture::buffer()
{
    return m_buffer;
}

size_t picture::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;







    return current_align;
}

bool picture::isKeyDefined()
{
   return false;
}

void picture::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
     
     
     
     
}
