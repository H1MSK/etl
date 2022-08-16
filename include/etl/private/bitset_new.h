///\file

/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl
https://www.etlcpp.com

Copyright(c) 2014 John Wellbelove

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef ETL_BITSET_NEW_INCLUDED
#define ETL_BITSET_NEW_INCLUDED

#include <string.h>
#include <stddef.h>
#include <stdint.h>

#include "../platform.h"
#include "../algorithm.h"
#include "../iterator.h"
#include "../integral_limits.h"
#include "../algorithm.h"
#include "../nullptr.h"
#include "../log.h"
#include "../exception.h"
#include "../integral_limits.h"
#include "../binary.h"
#include "../char_traits.h"
#include "../static_assert.h"
#include "../error_handler.h"
#include "../span.h"
#include "../string.h"

#include "minmax_push.h"

#if defined(ETL_COMPILER_KEIL)
#pragma diag_suppress 1300
#endif

#if ETL_USING_CPP11
  #define ETL_STR(x)  x
  #define ETL_STRL(x) L##x
  #define ETL_STRu(x) u##x
  #define ETL_STRU(x) U##x
#else
  #define ETL_STR(x)  x
  #define ETL_STRL(x) x
  #define ETL_STRu(x) x
  #define ETL_STRU(x) x
#endif

//*****************************************************************************
///\defgroup bitset bitset
/// Similar to std::bitset but without requiring std::string.
///\ingroup containers
//*****************************************************************************

namespace etl
{
  //***************************************************************************
  /// Exception base for bitset
  ///\ingroup bitset
  //***************************************************************************
  class bitset_exception : public etl::exception
  {
  public:

    bitset_exception(string_type reason_, string_type file_name_, numeric_type line_number_)
      : exception(reason_, file_name_, line_number_)
    {
    }
  };

  //***************************************************************************
  /// Bitset null pointer exception.
  ///\ingroup bitset
  //***************************************************************************
  class bitset_nullptr : public bitset_exception
  {
  public:

    bitset_nullptr(string_type file_name_, numeric_type line_number_)
      : bitset_exception(ETL_ERROR_TEXT("bitset:null pointer", ETL_BITSET_FILE_ID"A"), file_name_, line_number_)
    {
    }
  };

  //***************************************************************************
  /// Bitset type_too_small exception.
  ///\ingroup bitset
  //***************************************************************************
  class bitset_type_too_small : public bitset_exception
  {
  public:

    bitset_type_too_small(string_type file_name_, numeric_type line_number_)
      : bitset_exception(ETL_ERROR_TEXT("bitset:type_too_small", ETL_BITSET_FILE_ID"B"), file_name_, line_number_)
    {
    }
  };

  //***************************************************************************
  /// Bitset overflow exception.
  ///\ingroup bitset
  //***************************************************************************
  class bitset_overflow : public bitset_exception
  {
  public:

    bitset_overflow(string_type file_name_, numeric_type line_number_)
      : bitset_exception(ETL_ERROR_TEXT("bitset:overflow", ETL_BITSET_FILE_ID"C"), file_name_, line_number_)
    {
    }
  };

  //*************************************************************************
  /// The base class for etl::bitset
  ///\ingroup bitset
  //*************************************************************************
  template <typename TElement = void>
  class bitset_impl
  {
  public:

    typedef typename etl::make_unsigned<TElement>::type element_type;    
    typedef element_type*       pointer;
    typedef const element_type* const_pointer;

    static ETL_CONSTANT size_t Bits_Per_Element = etl::integral_limits<element_type>::bits;
    static ETL_CONSTANT element_type All_Set    = etl::integral_limits<typename etl::make_unsigned<element_type>::type>::max;
    static ETL_CONSTANT element_type All_Clear  = element_type(0);

    enum
    {
      npos = etl::integral_limits<size_t>::max
    };

    //*************************************************************************
    /// Count the number of bits set.
    //*************************************************************************
    ETL_CONSTEXPR14 size_t count(const_pointer pbuffer, size_t Number_Of_Elements) const
    {
      size_t n = 0UL;

      for (size_t i = 0UL; i < Number_Of_Elements; ++i)
      {
        n += etl::count_bits(pbuffer[i]);
      }

      return n;
    }

    //*************************************************************************
    /// Tests a bit at a position.
    /// Positions greater than the number of configured bits will return <b>false</b>.
    //*************************************************************************
    ETL_CONSTEXPR14 bool test(const_pointer pbuffer, size_t Number_Of_Elements, size_t position) const
    {
      size_t       index = 0U;
      element_type mask  = element_type(0);

      if (Number_Of_Elements == 0)
      {
        return false;
      }
      else if (Number_Of_Elements == 1)
      {
        index = 0;
        mask = element_type(1) << position;
      }
      else
      {
        index = position >> etl::log2<Bits_Per_Element>::value;
        mask = element_type(1) << (position & (Bits_Per_Element - 1));
      }

      return (pbuffer[index] & mask) != 0;
    }


    //*************************************************************************
    /// Set the bit at the position.
    //*************************************************************************
    ETL_CONSTEXPR14 void set(pointer pbuffer, size_t Number_Of_Elements, size_t position, bool value = true)
    {
      size_t    index = 0;
      element_type bit = 0;

      if (Number_Of_Elements == 0) 
      {
        return;
      }
      else if (Number_Of_Elements == 1)
      {
        index = 0;
        bit = element_type(1) << position;
      }
      else
      {
        index = position >> etl::log2<Bits_Per_Element>::value;
        bit = element_type(1) << (position & (Bits_Per_Element - 1));
      }

      if (value)
      {
        pbuffer[index] |= bit;
      }
      else
      {
        pbuffer[index] &= ~bit;
      }
    }

    //*************************************************************************
    /// Set from a string.
    //*************************************************************************
    void from_string(pointer pbuffer, size_t number_of_elements, size_t total_bits, const char* text)
    {
      size_t string_length = etl::strlen(text);
      size_t element_index = etl::min(number_of_elements - 1U, (string_length / Bits_Per_Element));     

      // Only reset elements we need to.
      while (element_index != number_of_elements)
      {
        pbuffer[element_index++] = All_Clear;
      }

      // Build from the string.
      size_t i = etl::min(total_bits, string_length);

      while (i > 0)
      {
        set(pbuffer, number_of_elements, --i, *text++ == ETL_STR('1'));
      }
    }

    //*************************************************************************
    /// Set from a wide string.
    //*************************************************************************
    void from_string(pointer pbuffer, size_t number_of_elements, size_t total_bits, const wchar_t* text)
    {
      size_t string_length = etl::strlen(text);
      size_t element_index = etl::min(number_of_elements - 1U, (string_length / Bits_Per_Element));

      // Only reset elements we need to.
      while (element_index != number_of_elements)
      {
        pbuffer[element_index++] = All_Clear;
      }

      // Build from the string.
      size_t i = etl::min(total_bits, string_length);

      while (i > 0)
      {
        set(pbuffer, number_of_elements, --i, *text++ == ETL_STRL('1'));
      }
    }

    //*************************************************************************
    /// Set from a u16 string.
    //*************************************************************************
    void from_string(pointer pbuffer, size_t number_of_elements, size_t total_bits, const char16_t* text)
    {
      size_t string_length = etl::strlen(text);
      size_t element_index = etl::min(number_of_elements - 1U, (string_length / Bits_Per_Element));

      // Only reset elements we need to.
      while (element_index != number_of_elements)
      {
        pbuffer[element_index++] = All_Clear;
      }

      // Build from the string.
      size_t i = etl::min(total_bits, string_length);

      while (i > 0)
      {
        set(pbuffer, number_of_elements, --i, *text++ == ETL_STRu('1'));
      }
    }

    //*************************************************************************
    /// Set from a u32 string.
    //*************************************************************************
    void from_string(pointer pbuffer, size_t number_of_elements, size_t total_bits, const char32_t* text)
    {
      size_t string_length = etl::strlen(text);
      size_t element_index = etl::min(number_of_elements - 1U, (string_length / Bits_Per_Element));

      // Only reset elements we need to.
      while (element_index != number_of_elements)
      {
        pbuffer[element_index++] = All_Clear;
      }

      // Build from the string.
      size_t i = etl::min(total_bits, string_length);

      while (i > 0)
      {
        set(pbuffer, number_of_elements, --i, *text++ == ETL_STRU('1'));
      }
    }

    //*************************************************************************
    /// Set from a string.
    //*************************************************************************
    ETL_CONSTEXPR14 void set(pointer pbuffer, size_t number_of_elements, size_t total_bits, const char* text)
    {
      from_string(pbuffer, number_of_elements, total_bits, text);
    }

    //*************************************************************************
    /// Set from a wstring.
    //*************************************************************************
    ETL_CONSTEXPR14 void set(pointer pbuffer, size_t number_of_elements, size_t total_bits, const wchar_t* text)
    {
      from_string(pbuffer, number_of_elements, total_bits, text);
    }

    //*************************************************************************
    /// Set from a u16string.
    //*************************************************************************
    ETL_CONSTEXPR14 void set(pointer pbuffer, size_t number_of_elements, size_t total_bits, const char16_t* text)
    {
      from_string(pbuffer, number_of_elements, total_bits, text);
    }

    //*************************************************************************
    /// Set from a u32string.
    //*************************************************************************
    ETL_CONSTEXPR14 void set(pointer pbuffer, size_t number_of_elements, size_t total_bits, const char32_t* text)
    {
      from_string(pbuffer, number_of_elements, total_bits, text);
    }

    //*************************************************************************
    /// Get as a value.
    //*************************************************************************
    template <typename T>
    ETL_CONSTEXPR14
    typename etl::enable_if<etl::is_integral<T>::value, T>::type
      value(const_pointer pbuffer, size_t number_of_elements) const
    {
      T v = T(0);

      const bool OK = (sizeof(T) * CHAR_BIT) >= (number_of_elements * Bits_Per_Element);

      ETL_ASSERT_AND_RETURN_VALUE(OK, ETL_ERROR(etl::bitset_type_too_small), T(0));

      if (OK)
      {
        uint_least8_t shift = 0U;

        for (size_t i = 0UL; i < number_of_elements; ++i)
        {
          v |= T(typename etl::make_unsigned<T>::type(pbuffer[i]) << shift);
          shift += uint_least8_t(Bits_Per_Element);
        }
      }

      return v;
    }

    //*************************************************************************
    /// Reset the bit at the position.
    //*************************************************************************
    ETL_CONSTEXPR14 void reset(pointer pbuffer, size_t number_of_elements, size_t position)
    {
      size_t       index = 0U;
      element_type bit = element_type(0);

      if (number_of_elements == 0)
      {
        return;
      }
      else if (number_of_elements == 1)
      {
        index = 0;
        bit = element_type(1) << position;
      }
      else
      {
        index = position >> etl::log2<Bits_Per_Element>::value;
        bit = element_type(1) << (position & (Bits_Per_Element - 1));
      }

      pbuffer[index] &= ~bit;
    }

    //*************************************************************************
    /// Flip all of the bits.
    //*************************************************************************
    ETL_CONSTEXPR14 void flip(pointer pbuffer, size_t number_of_elements)
    {
      for (size_t i = 0UL; i < number_of_elements; ++i)
      {
        pbuffer[i] = ~pbuffer[i];
      }
    }

    //*************************************************************************
    /// Flip the bit at the position.
    //*************************************************************************
    ETL_CONSTEXPR14 void flip(pointer pbuffer, size_t number_of_elements, size_t total_bits, size_t position)
    {
      if (position < total_bits)
      {
        size_t    index = 0U;
        element_type bit = element_type(0);
        
        if (number_of_elements == 0)
        {
          return;
        }
        else if (number_of_elements == 1)
        {
          index = 0;
          bit = element_type(1) << position;
        }
        else
        {
          index = position >> log2<Bits_Per_Element>::value;
          bit = element_type(1) << (position & (Bits_Per_Element - 1));
        }

        pbuffer[index] ^= bit;
      }
    }

    //*************************************************************************
    // Are all the bits sets?
    //*************************************************************************
    ETL_CONSTEXPR14 bool all(const_pointer pbuffer, size_t number_of_elements, element_type top_mask) const ETL_NOEXCEPT
    {
      if (number_of_elements == 0UL)
      {
        return true;
      }

      // All but the last.
      for (size_t i = 0UL; i < (number_of_elements - 1U); ++i)
      {
        if (pbuffer[i] != All_Set)
        {
          return false;
        }
      }

      // The last.
      if (pbuffer[number_of_elements - 1U] != (All_Set & top_mask))
      {
        return false;
      }

      return true;
    }

    //*************************************************************************
    /// Are none of the bits set?
    //*************************************************************************
    ETL_CONSTEXPR14 bool none(const_pointer pbuffer, size_t number_of_elements) const ETL_NOEXCEPT
    {
      for (size_t i = 0UL; i < number_of_elements; ++i)
      {
        if (pbuffer[i] != 0)
        {
          return false;
        }
      }

      return true;
    }

    //*************************************************************************
    /// Finds the first bit in the specified state.
    ///\param state The state to search for.
    ///\returns The position of the bit or Number_Of_Elements if none were found.
    //*************************************************************************
    ETL_CONSTEXPR14 size_t find_first(const_pointer pbuffer, size_t number_of_elements, size_t total_bits, bool state) const
    {
      return find_next(pbuffer, number_of_elements, total_bits, state, 0);
    }

    //*************************************************************************
    /// Finds the next bit in the specified state.
    ///\param state    The state to search for.
    ///\param position The position to start from.
    ///\returns The position of the bit or npos if none were found.
    //*************************************************************************
    ETL_CONSTEXPR14 size_t find_next(const_pointer pbuffer, size_t number_of_elements, size_t total_bits, bool state, size_t position) const
    {
      // Where to start.
      size_t index = position >> log2<Bits_Per_Element>::value;
      size_t bit   = position;

      element_type mask = 1 << bit;

      // For each element in the bitset...
      while (index < number_of_elements)
      {
        element_type value = pbuffer[index];

        // Needs checking?
        if ((state && (value != All_Clear)) ||
            (!state && (value != All_Set)))
        {
          // For each bit in the element...
          while ((bit < Bits_Per_Element) && (position < total_bits))
          {
            // Equal to the required state?
            if (((value & mask) != 0) == state)
            {
              return position;
            }

            // Move on to the next bit.
            mask <<= 1;
            ++position;
            ++bit;
          }
        }
        else
        {
          position += (Bits_Per_Element - bit);
        }

        // Start at the beginning for all other elements.
        bit = 0;
        mask = 1;

        ++index;
      }

      return npos;
    }

    //*************************************************************************
    /// shift_left_equals
    //*************************************************************************
    ETL_CONSTEXPR14 void shift_left_equals(pointer pbuffer, size_t number_of_elements, size_t total_bits, size_t shift)
    {
      if ((shift % Bits_Per_Element) == 0U)
      {
        const size_t element_shift = shift / Bits_Per_Element;
        etl::copy_backward(pbuffer, pbuffer + number_of_elements - element_shift, pbuffer + number_of_elements);
        etl::fill_n(pbuffer, element_shift, All_Clear);
      }
      else
      {
        // The place where the elements are split when shifting.
        const size_t split_position = Bits_Per_Element - (shift % Bits_Per_Element);

        // Where we are shifting from.
        int src_index = int(number_of_elements - (shift / Bits_Per_Element) - 1U);

        // Where we are shifting to.
        int dst_index = int(number_of_elements - 1U);

        // Shift control constants.
        const size_t lsb_shift = Bits_Per_Element - split_position;
        const size_t msb_shift = split_position;

        const element_type lsb_mask         = element_type(etl::integral_limits<element_type>::max >> (Bits_Per_Element - split_position));
        const element_type msb_mask         = etl::integral_limits<element_type>::max - lsb_mask;
        const element_type lsb_shifted_mask = element_type(lsb_mask << lsb_shift);
          
        // First lsb.
        element_type lsb = element_type((pbuffer[src_index] & lsb_mask) << lsb_shift);
        pbuffer[dst_index] = lsb;
        --src_index;

        // Now do the shifting.
        while (src_index >= 0)
        {
          // Shift msb.
          element_type msb = element_type((pbuffer[src_index] & msb_mask) >> msb_shift);
          pbuffer[dst_index] = pbuffer[dst_index] | msb;
          --dst_index;

          // Shift lsb.
          element_type lsb = element_type((pbuffer[src_index] & lsb_mask) << lsb_shift);
          pbuffer[dst_index] = lsb;
          --src_index;
        }

        // Clear the remaining bits.
        // First lsb.
        pbuffer[dst_index] &= lsb_shifted_mask;
        --dst_index;

        // The other remaining bytes on the right.
        while (dst_index >= 0)
        {
          pbuffer[dst_index] = 0;
          --dst_index;
        }
      }
    }

    //*************************************************************************
    /// shift_right_equals
    //*************************************************************************
    ETL_CONSTEXPR14 void shift_right_equals(pointer pbuffer, size_t number_of_elements, size_t total_bits, size_t shift)
    {
      if ((shift % Bits_Per_Element) == 0U)
      {
        const size_t element_shift = (shift / Bits_Per_Element);
        pointer pzeros_begin = etl::copy(pbuffer + element_shift, pbuffer + number_of_elements, pbuffer);      
        etl::fill_n(pzeros_begin, element_shift, All_Clear);
      }
      else
      {
        // The place where the elements are split when shifting.
        const size_t split_position = shift % Bits_Per_Element;

        // Where we are shifting from.
        int src_index = int(shift / Bits_Per_Element);

        // Where we are shifting to.
        int dst_index = 0;

        // Shift control constants.
        const size_t lsb_shift = Bits_Per_Element - split_position;
        const size_t msb_shift = split_position;

        const element_type lsb_mask         = element_type(etl::integral_limits<element_type>::max >> (Bits_Per_Element - split_position));
        const element_type msb_mask         = etl::integral_limits<element_type>::max - lsb_mask;
        const element_type msb_shifted_mask = element_type(msb_mask >> msb_shift);

        // Now do the shifting.
        while (src_index < int(number_of_elements - 1))
        {
          // Shift msb.
          element_type msb = element_type((pbuffer[src_index] & msb_mask) >> msb_shift);
          ++src_index;
            
          // Shift lsb.
          element_type lsb = element_type((pbuffer[src_index] & lsb_mask) << lsb_shift);
            
          // Combine them.
          pbuffer[dst_index] = lsb | msb;
          ++dst_index;
        }

        // Final msb.
        element_type msb = element_type((pbuffer[src_index] & msb_mask) >> msb_shift);
        pbuffer[dst_index] = msb;

        // Clear the remaining bits.
        // First msb.
        pbuffer[dst_index] &= msb_shifted_mask;
        ++dst_index;

        // The other remaining bytes.
        while (dst_index < int(number_of_elements))
        {
          pbuffer[dst_index] = 0;
          ++dst_index;
        }
      }
    }

    //*************************************************************************
    /// and_equals
    //*************************************************************************
    ETL_CONSTEXPR14 void and_equals(pointer pbuffer, const_pointer pbuffer2, size_t number_of_elements)
    {
      for (size_t i = 0U; i < number_of_elements; ++i)
      {
        pbuffer[i] &= pbuffer2[i];
      }
    }

    //*************************************************************************
    /// or_equals
    //*************************************************************************
    ETL_CONSTEXPR14 void or_equals(pointer pbuffer, const_pointer pbuffer2, size_t number_of_elements)
    {
      for (size_t i = 0U; i < number_of_elements; ++i)
      {
        pbuffer[i] |= pbuffer2[i];
      }
    }

    //*************************************************************************
    /// xor_equals
    //*************************************************************************
    ETL_CONSTEXPR14 void xor_equals(pointer pbuffer, const_pointer pbuffer2, size_t number_of_elements)
    {
      for (size_t i = 0U; i < number_of_elements; ++i)
      {
        pbuffer[i] ^= pbuffer2[i];
      }
    }

    //*************************************************************************
    /// Initialise from an unsigned long long.
    //*************************************************************************
    ETL_CONSTEXPR14 void initialise(pointer pbuffer, size_t number_of_elements, unsigned long long value)
    {
      const size_t Shift = (etl::integral_limits<unsigned long long>::bits <= (int)Bits_Per_Element) ? 0 : Bits_Per_Element;

      // Can we do it in one hit?
      if (Shift == 0)
      {
        pbuffer[0] = element_type(value);
      }
      else
      {
        size_t i = 0UL;

        // Set the non-zero elements.
        while ((value != 0) && (i != number_of_elements))
        {
          pbuffer[i++] = value & All_Set;
          value = value >> Shift;
        }

        // Clear the remaining elements.
        while (i != number_of_elements)
        {
          pbuffer[i++] = All_Clear;
        }
      }
    }
    //*************************************************************************
    ETL_CONSTEXPR14 void copy_elements(const_pointer sb, pointer db, size_t count)
    {
      while (count != 0)
      {
        *db = *sb;
        ++db;
        ++sb;
        --count;
      }
    }
  };

  namespace private_bitset
  {
    //*************************************************************************
    // Selects an implicit element type based on the required number of bits.
    template <size_t Active_Bits, bool Using_8Bit_Types, bool Using_64Bit_Types>
    struct select_element_type;

    //*************************************************************************
    template <size_t Active_Bits>
    struct select_element_type<Active_Bits, 1, 1>
    {
      typedef typename etl::conditional<Active_Bits == 0U, void,
      typename etl::conditional<Active_Bits <= 8U, uint8_t,
      typename etl::conditional<Active_Bits <= 16U, uint16_t,
      typename etl::conditional<Active_Bits <= 32U, uint32_t,
      typename etl::conditional<Active_Bits <= 64U, uint64_t, char>::type>::type>::type>::type>::type type;
    };

    //*************************************************************************
    template <size_t Active_Bits>
    struct select_element_type<Active_Bits, 0, 1>
    {
      typedef typename etl::conditional<Active_Bits <= 0U, void,
      typename etl::conditional<Active_Bits <= 16U, uint16_t,
      typename etl::conditional<Active_Bits <= 32U, uint32_t,
      typename etl::conditional<Active_Bits <= 64U, uint64_t, char>::type>::type>::type>::type type;
    };

    //*************************************************************************
    template <size_t Active_Bits>
    struct select_element_type<Active_Bits, 1, 0>
    {
      typedef typename etl::conditional<Active_Bits <= 0U, void,
      typename etl::conditional<Active_Bits <= 8U, uint8_t,
      typename etl::conditional<Active_Bits <= 16U, uint16_t,
      typename etl::conditional<Active_Bits <= 32U, uint32_t, char>::type>::type>::type>::type type;
    };
    
    //*************************************************************************
    template <size_t Active_Bits>
    struct select_element_type<Active_Bits, 0, 0>
    {
      typedef typename etl::conditional<Active_Bits <= 0U, void,
      typename etl::conditional<Active_Bits <= 16U, uint16_t,
      typename etl::conditional<Active_Bits <= 32U, uint32_t, char>::type>::type>::type type;
    };
  }

  //*************************************************************************
  /// Specialisation for void type.
  //*************************************************************************
  template <>
  class bitset_impl<void>
  {
  public:

    enum
    {
      npos = etl::integral_limits<size_t>::max
    };
  };

  //*************************************************************************
  /// The class emulates an array of bool elements, but optimized for space allocation.
  /// Will accommodate any number of bits.
  ///\ingroup bitset
  //*************************************************************************
  template <size_t Active_Bits = 0U, 
            typename TElement = typename private_bitset::select_element_type<Active_Bits, ETL_USING_8BIT_TYPES, ETL_USING_64BIT_TYPES>::type,
            bool IsSingleElement  = (etl::integral_limits<TElement>::bits >= Active_Bits),
            bool Has_Element_Mask = IsSingleElement && (etl::integral_limits<TElement>::bits != Active_Bits)>
  class bitset
  {
  public:

    typedef typename etl::bitset_impl<TElement>::element_type  element_type;
    typedef typename etl::bitset_impl<TElement>::pointer       pointer;
    typedef typename etl::bitset_impl<TElement>::const_pointer const_pointer;
    
    typedef etl::span<element_type>       span_type;
    typedef etl::span<const element_type> const_span_type;
   
    static ETL_CONSTANT size_t       Bits_Per_Element   = etl::bitset_impl<element_type>::Bits_Per_Element;
    static ETL_CONSTANT size_t       Number_Of_Elements = (Active_Bits % Bits_Per_Element == 0) ? Active_Bits / Bits_Per_Element : Active_Bits / Bits_Per_Element + 1;
    static ETL_CONSTANT size_t       Allocated_Bits     = Number_Of_Elements * Bits_Per_Element;
    static ETL_CONSTANT size_t       Top_Mask_Shift     = ((Bits_Per_Element - (Allocated_Bits - Active_Bits)) % Bits_Per_Element);
    static ETL_CONSTANT element_type All_Set            = etl::bitset_impl<element_type>::All_Set;
    static ETL_CONSTANT element_type All_Clear          = etl::bitset_impl<element_type>::All_Clear;
    static ETL_CONSTANT element_type Top_Mask           = element_type(Top_Mask_Shift == 0 ? All_Set : ~(All_Set << Top_Mask_Shift));

    static ETL_CONSTANT size_t ALLOCATED_BITS = Allocated_Bits; ///< For backward compatibility.

    //*************************************************************************
    /// The reference type returned.
    //*************************************************************************
    class bit_reference
    {
    public:

      friend class bitset;

      //*******************************
      /// Conversion operator.
      //*******************************
      ETL_CONSTEXPR14 operator bool() const
      {
        return p_bitset->test(position);
      }

      //*******************************
      /// Assignment operator.
      //*******************************
      ETL_CONSTEXPR14 bit_reference& operator = (bool b)
      {
        p_bitset->set(position, b);
        return *this;
      }

      //*******************************
      /// Assignment operator.
      //*******************************
      ETL_CONSTEXPR14 bit_reference& operator = (const bit_reference& r)
      {
        p_bitset->set(position, bool(r));
        return *this;
      }

      //*******************************
      /// Flip the bit.
      //*******************************
      ETL_CONSTEXPR14 bit_reference& flip()
      {
        p_bitset->flip(position);
        return *this;
      }

      //*******************************
      /// Return the logical inverse of the bit.
      //*******************************
      ETL_CONSTEXPR14 bool operator~() const
      {
        return !p_bitset->test(position);
      }

    private:

      //*******************************
      /// Default constructor.
      //*******************************
      ETL_CONSTEXPR14 bit_reference()
        : p_bitset(ETL_NULLPTR)
        , position(0)
      {
      }

      //*******************************
      /// Constructor.
      //*******************************
      ETL_CONSTEXPR14 bit_reference(bitset<Active_Bits, TElement>& r_bitset, size_t position_)
        : p_bitset(&r_bitset)
        , position(position_)
      {
      }

      bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>* p_bitset; ///< The bitset.
      size_t position; ///< The position in the bitset.
    };

    //*************************************************************************
    /// Default constructor.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset() ETL_NOEXCEPT
      : buffer()
    {
      reset();
    }

    //*************************************************************************
    /// Copy constructor.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset(const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& other) ETL_NOEXCEPT
      : buffer()
    {
      ibitset.copy_elements(other.buffer, buffer, Number_Of_Elements);
    }

    //*************************************************************************
    /// Construct from a value.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset(unsigned long long value) ETL_NOEXCEPT
      : buffer()
    {
      ibitset.initialise(buffer, Number_Of_Elements, value);
      clear_unused_bits_in_msb();
    }

    //*************************************************************************
    /// Construct from a string.
    //*************************************************************************
    bitset(const char* text)
      : buffer()
    {
      ibitset.set(buffer, Number_Of_Elements, Active_Bits, text);
      clear_unused_bits_in_msb();
    }

    //*************************************************************************
    /// Construct from a string.
    //*************************************************************************
    bitset(const wchar_t* text)
      : buffer()
    {
      ibitset.set(buffer, Number_Of_Elements, Active_Bits, text);
      clear_unused_bits_in_msb();
    }

    //*************************************************************************
    /// Construct from a string.
    //*************************************************************************
    bitset(const char16_t* text)
      : buffer()
    {
      ibitset.set(buffer, Number_Of_Elements, Active_Bits, text);
      clear_unused_bits_in_msb();
    }

    //*************************************************************************
    /// Construct from a string.
    //*************************************************************************
    bitset(const char32_t* text)
      : buffer()
    {
      ibitset.set(buffer, Number_Of_Elements, Active_Bits, text);
      clear_unused_bits_in_msb();
    }

    //*************************************************************************
    /// Set all of the bits.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& set()
    {
      etl::fill_n(buffer, ETL_OR_STD::size(buffer), All_Set);
      clear_unused_bits_in_msb();

      return *this;
    }

    //*************************************************************************
    /// Set the bit at the position.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& set(size_t position, bool value = true) ETL_NOEXCEPT
    {
      ibitset.set(buffer, Number_Of_Elements, position, value);
      clear_unused_bits_in_msb();
      
      return *this;
    }

    //*************************************************************************
    /// Set from a string.
    //*************************************************************************
    bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& set(const char* text)
    {
      ETL_ASSERT_AND_RETURN_VALUE(text != 0, ETL_ERROR(bitset_nullptr), *this);
      ibitset.set(buffer, Number_Of_Elements, Active_Bits, text);

      return *this;
    }

    //*************************************************************************
    /// Set from a string.
    //*************************************************************************
    bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& set(const wchar_t* text)
    {
      ETL_ASSERT_AND_RETURN_VALUE(text != 0, ETL_ERROR(bitset_nullptr), *this);
      ibitset.set(buffer, Number_Of_Elements, Active_Bits, text);

      return *this;
    }

    //*************************************************************************
    /// Set from a string.
    //*************************************************************************
    bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& set(const char16_t* text)
    {
      ETL_ASSERT_AND_RETURN_VALUE(text != 0, ETL_ERROR(bitset_nullptr), *this);
      ibitset.set(buffer, Number_Of_Elements, Active_Bits, text);

      return *this;
    }

    //*************************************************************************
    /// Set from a string.
    //*************************************************************************
    bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& set(const char32_t* text)
    {
      ETL_ASSERT_AND_RETURN_VALUE(text != 0, ETL_ERROR(bitset_nullptr), *this);
      ibitset.set(buffer, Number_Of_Elements, Active_Bits, text);

      return *this;
    }

    //*************************************************************************
    /// Set from a string.
    //*************************************************************************
    bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& from_string(const char* text)
    {
      ibitset.from_string(buffer, Number_Of_Elements, Active_Bits, text);

      return *this;
    }

    //*************************************************************************
    /// Set from a wide string.
    //*************************************************************************
    bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& from_string(const wchar_t* text)
    {
      ibitset.from_string(buffer, Number_Of_Elements, Active_Bits, text);

      return *this;
    }

    //*************************************************************************
    /// Set from a u16 string.
    //*************************************************************************
    bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& from_string(const char16_t* text)
    {
      ibitset.from_string(buffer, Number_Of_Elements, Active_Bits, text);

      return *this;
    }

    //*************************************************************************
    /// Set from a u32 string.
    //*************************************************************************
    bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& from_string(const char32_t* text)
    {
      ibitset.from_string(buffer, Number_Of_Elements, Active_Bits, text);

      return *this;
    }

    //*************************************************************************
    /// Get as an integral value.
    //*************************************************************************
    template <typename T>
    ETL_CONSTEXPR14
      typename etl::enable_if<etl::is_integral<T>::value, T>::type
      value() const
    {
      ETL_STATIC_ASSERT(etl::is_integral<T>::value, "Only integral types are supported");
      ETL_STATIC_ASSERT((sizeof(T) * CHAR_BIT) >= (Number_Of_Elements * Bits_Per_Element), "Type too small");

      return ibitset.value<T>(buffer, Number_Of_Elements);
    }

    //*************************************************************************
    /// Get as an unsigned long.
    //*************************************************************************
    ETL_CONSTEXPR14 unsigned long to_ulong() const
    {
      return value<unsigned long>();
    }

    //*************************************************************************
    /// Get as an unsigned long long.
    //*************************************************************************
    ETL_CONSTEXPR14 unsigned long long to_ullong() const
    {
      return value<unsigned long long>();
    }
    //*************************************************************************
    /// Reset all of the bits.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& reset() ETL_NOEXCEPT
    {
      etl::fill_n(buffer, Number_Of_Elements, All_Clear);

      return *this;
    }

    //*************************************************************************
    /// Reset the bit at the position.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& reset(size_t position) ETL_NOEXCEPT
    {
      ibitset.reset(buffer, Number_Of_Elements, position);
      return *this;
    }

    //*************************************************************************
    /// Tests a bit at a position.
    /// Positions greater than the number of configured bits will return <b>false</b>.
    //*************************************************************************
    ETL_CONSTEXPR14 bool test(size_t position) const
    {
      return ibitset.test(buffer, Number_Of_Elements, position);
    }
 
    //*************************************************************************
    /// The number of bits in the bitset.
    //*************************************************************************
    ETL_CONSTEXPR size_t size() const ETL_NOEXCEPT
    {
      return Active_Bits;
    }

    //*************************************************************************
    /// Count the number of bits set.
    //*************************************************************************
    ETL_CONSTEXPR14 size_t count() const ETL_NOEXCEPT
    {
      return ibitset.count(buffer, Number_Of_Elements);
    }

    //*************************************************************************
    // Are all the bits sets?
    //*************************************************************************
    ETL_CONSTEXPR14 bool all() const ETL_NOEXCEPT
    {
      return ibitset.all(buffer, Number_Of_Elements, Top_Mask);
    }

    //*************************************************************************
    /// Are none of the bits set?
    //*************************************************************************
    ETL_CONSTEXPR14 bool none() const ETL_NOEXCEPT
    {
      return ibitset.none(buffer, Number_Of_Elements);
    }

    //*************************************************************************
    /// Are any of the bits set?
    //*************************************************************************
    ETL_CONSTEXPR14 bool any() const ETL_NOEXCEPT
    {
      return !none();
    }
 
    //*************************************************************************
    /// Flip all of the bits.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& flip()
    {
      ibitset.flip(buffer, Number_Of_Elements);
      clear_unused_bits_in_msb();

      return *this;
    }

    //*************************************************************************
    /// Flip the bit at the position.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& flip(size_t position)
    {
      ibitset.flip(buffer, Number_Of_Elements, Active_Bits, position);

      return *this;
    }

    //*************************************************************************
    /// Read [] operator.
    //*************************************************************************
    ETL_CONSTEXPR14 bool operator[] (size_t position) const
    {
      return ibitset.test(buffer, Number_Of_Elements, position);
    }

    //*************************************************************************
    /// Write [] operator.
    //*************************************************************************
    ETL_CONSTEXPR14 bit_reference operator [] (size_t position)
    {
      return bit_reference(*this, position);
    }

    //*************************************************************************
    /// Returns a string representing the bitset.
    //*************************************************************************
#if ETL_USING_CPP11
    template <typename TString = etl::string<Active_Bits>>
#else
    template <typename TString>
#endif
    TString to_string(typename TString::value_type zero = typename TString::value_type('0'), 
                      typename TString::value_type one  = typename TString::value_type('1')) const
    {
      TString result;

      result.resize(Active_Bits, '\0');

      ETL_ASSERT_AND_RETURN_VALUE((result.size() == Active_Bits), ETL_ERROR(etl::bitset_overflow), result);

      for (size_t i = Active_Bits; i > 0; --i) 
      {
        result[Active_Bits - i] = test(i - 1) ? one : zero;
      }
      
      return result;
    }

    //*************************************************************************
    /// Finds the first bit in the specified state.
    ///\param state The state to search for.
    ///\returns The position of the bit or Number_Of_Elements if none were found.
    //*************************************************************************
    ETL_CONSTEXPR14 size_t find_first(bool state) const
    {
      return ibitset.find_next(buffer, Number_Of_Elements, Active_Bits, state, 0);
    }

    //*************************************************************************
    /// Finds the next bit in the specified state.
    ///\param state    The state to search for.
    ///\param position The position to start from.
    ///\returns The position of the bit or npos if none were found.
    //*************************************************************************
    ETL_CONSTEXPR14 size_t find_next(bool state, size_t position) const
    {
      return ibitset.find_next(buffer, Number_Of_Elements, Active_Bits, state, position);
    }

    //*************************************************************************
    /// operator &
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask> operator &(const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& other) const
    {
      etl::bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask> temp(*this);

      temp &= other;

      return temp;
    }

    //*************************************************************************
    /// operator &=
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& operator &=(const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& other)
    {
      ibitset.and_equals(&buffer[0], &other.buffer[0], Number_Of_Elements);

      return *this;
    }

    //*************************************************************************
    /// operator |
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask> operator |(const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& other) const
    {
      etl::bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask> temp(*this);

      temp |= other;

      return temp;
    }

    //*************************************************************************
    /// operator |=
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& operator |=(const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& other)
    {
      ibitset.or_equals(&buffer[0], &other.buffer[0], Number_Of_Elements);

      return *this;
    }

    //*************************************************************************
    /// operator ^
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask> operator ^(const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& other) const
    {
      etl::bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask> temp(*this);

      temp ^= other;

      return temp;
    }

    //*************************************************************************
    /// operator ^=
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& operator ^=(const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& other)
    {
      ibitset.xor_equals(&buffer[0], &other.buffer[0], Number_Of_Elements);

      return *this;
    }

    //*************************************************************************
    /// operator ~
    //*************************************************************************
    bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask> operator ~() const
    {
      etl::bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask> temp(*this);

      temp.flip();

      return temp;
    }

    //*************************************************************************
    /// operator <<
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask> operator <<(size_t shift) const
    {
      etl::bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask> temp(*this);

      temp <<= shift;

      return temp;
    }

    //*************************************************************************
    /// operator <<=
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& operator <<=(size_t shift)
    {
      if (shift >= Active_Bits)
      {
        reset();
      }
      else
      {
        ibitset.shift_left_equals(buffer, Number_Of_Elements, Active_Bits, shift);

        // Truncate any bits shifted to the left.
        clear_unused_bits_in_msb();
      }
      
      return *this;
    }

    //*************************************************************************
    /// operator >>
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask> operator >>(size_t shift) const
    {
      bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask> temp(*this);

      temp >>= shift;

      return temp;
    }

    //*************************************************************************
    /// operator >>=
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& operator >>=(size_t shift)
    {
      if (shift >= Active_Bits)
      {
        reset();
      }
      else
      {
        ibitset.shift_right_equals(buffer, Number_Of_Elements, Active_Bits, shift);
      }

      return *this;
    }

    //*************************************************************************
    /// operator ==
    //*************************************************************************
    friend ETL_CONSTEXPR14 bool operator ==(const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& lhs, const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& rhs)
    {
      return etl::equal(lhs.buffer, 
                        lhs.buffer + lhs.Number_Of_Elements,
                        rhs.buffer);
    }

    //*************************************************************************
    /// swap
    //*************************************************************************
    ETL_CONSTEXPR14 void swap(etl::bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& other)
    {
      pointer p1 = buffer;
      pointer p1_end = buffer + Number_Of_Elements;
      pointer p2 = other.buffer;
      
      while (p1 != p1_end)
      {
        element_type temp = *p1;
        *p1 = *p2;
        *p2 = temp;
        ++p1;
        ++p2;
      }
    }

    //*************************************************************************
    /// span
    /// Returns a span of the underlying buffer.
    //*************************************************************************
    ETL_CONSTEXPR14 span_type span() ETL_NOEXCEPT
    {
      return span_type(buffer, buffer + Number_Of_Elements);
    }

    //*************************************************************************
    /// span
    /// Returns a const span of the underlying buffer.
    //*************************************************************************
    ETL_CONSTEXPR14 const_span_type span() const ETL_NOEXCEPT
    {
      return const_span_type(buffer, buffer + Number_Of_Elements);
    }

  private:

    //*************************************************************************
    /// Correct the unused top bits after bit manipulation.
    //*************************************************************************
    ETL_CONSTEXPR14 void clear_unused_bits_in_msb() ETL_NOEXCEPT
    {
      buffer[Number_Of_Elements - 1U] &= Top_Mask;
    }

    etl::bitset_impl<TElement> ibitset;

    element_type buffer[Number_Of_Elements > 0U ? Number_Of_Elements : 1U];
  };

  //***************************************************************************
  /// Specialisation for 0 bits.
  //***************************************************************************
  template <>
  class bitset<0U, void>
  {
  public:

    enum
    {
      npos = etl::integral_limits<size_t>::max
    };
  };

  //***************************************************************************
  /// Specialisation for when there is only one element.
  //***************************************************************************
  template <size_t Active_Bits, typename TElement, bool Has_Element_Mask>
  class bitset<Active_Bits, TElement, true, Has_Element_Mask>
  {
  public:

    typedef TElement        element_type;
    typedef TElement*       pointer;
    typedef const TElement* const_pointer;

    typedef etl::span<element_type>       span_type;
    typedef etl::span<const element_type> const_span_type;

    static ETL_CONSTANT size_t       Bits_Per_Element   = etl::integral_limits<element_type>::bits;
    static ETL_CONSTANT size_t       Number_Of_Elements = 1U;
    static ETL_CONSTANT size_t       Allocated_Bits     = Bits_Per_Element;
    static ETL_CONSTANT size_t       Top_Mask_Shift     = ((Bits_Per_Element - (Allocated_Bits - Active_Bits)) == Bits_Per_Element) ? 0U : (Bits_Per_Element - (Allocated_Bits - Active_Bits));
    static ETL_CONSTANT element_type All_Set            = etl::integral_limits<typename etl::make_unsigned<element_type>::type>::max;
    static ETL_CONSTANT element_type All_Clear          = element_type(0);
    static ETL_CONSTANT element_type Top_Mask           = element_type(~(All_Set << Top_Mask_Shift));

    //*************************************************************************
    /// The reference type returned.
    //*************************************************************************
    class bit_reference
    {
    public:

      friend class bitset;

      //*******************************
      /// Conversion operator.
      //*******************************
      ETL_CONSTEXPR14 operator bool() const
      {
        return p_bitset->test(position);
      }

      //*******************************
      /// Assignment operator.
      //*******************************
      ETL_CONSTEXPR14 bit_reference& operator = (bool b)
      {
        p_bitset->set(position, b);
        return *this;
      }

      //*******************************
      /// Assignment operator.
      //*******************************
      ETL_CONSTEXPR14 bit_reference& operator = (const bit_reference& r)
      {
        p_bitset->set(position, bool(r));
        return *this;
      }

      //*******************************
      /// Flip the bit.
      //*******************************
      ETL_CONSTEXPR14 bit_reference& flip()
      {
        p_bitset->flip(position);
        return *this;
      }

      //*******************************
      /// Return the logical inverse of the bit.
      //*******************************
      ETL_CONSTEXPR14 bool operator~() const
      {
        return !p_bitset->test(position);
      }

    private:

      //*******************************
      /// Default constructor.
      //*******************************
      ETL_CONSTEXPR14 bit_reference()
        : p_bitset(ETL_NULLPTR)
        , position(0)
      {
      }

      //*******************************
      /// Constructor.
      //*******************************
      ETL_CONSTEXPR14 bit_reference(bitset<Active_Bits, TElement, true, Has_Element_Mask>& r_bitset, size_t position_)
        : p_bitset(&r_bitset)
        , position(position_)
      {
      }

      bitset<Active_Bits, TElement, true, Has_Element_Mask>* p_bitset; ///< The bitset.
      size_t position; ///< The position in the bitset.
    };

    //*************************************************************************
    /// Default constructor.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset() ETL_NOEXCEPT
      : buffer(All_Clear)
    {
    }

    //*************************************************************************
    /// Copy constructor.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset(const bitset<Active_Bits, TElement, true, Has_Element_Mask>& other) ETL_NOEXCEPT
      : buffer(other.buffer)
    {
    }

    //*************************************************************************
    /// Construct from a value.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset(unsigned long long value) ETL_NOEXCEPT
      : buffer(element_type(value))
    {
    }

    //*************************************************************************
    /// Construct from a string.
    //*************************************************************************
    bitset(const char* text)
      : buffer(All_Clear)
    {
      set(text);
      clear_unused_bits_in_msb();
    }

    //*************************************************************************
    /// Construct from a string.
    //*************************************************************************
    bitset(const wchar_t* text)
      : buffer(All_Clear)
    {
      set(text);
      clear_unused_bits_in_msb();
    }

    //*************************************************************************
    /// Construct from a string.
    //*************************************************************************
    bitset(const char16_t* text)
      : buffer(All_Clear)
    {
      set(text);
      clear_unused_bits_in_msb();
    }

    //*************************************************************************
    /// Construct from a string.
    //*************************************************************************
    bitset(const char32_t* text)
      : buffer(All_Clear)
    {
      set(buffer, Number_Of_Elements, Active_Bits, text);
      clear_unused_bits_in_msb();
    }

    //*************************************************************************
    /// Set all of the bits.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, true, Has_Element_Mask>& set()
    {
      buffer = All_Set;
      clear_unused_bits_in_msb();

      return *this;
    }

    //*************************************************************************
    /// Set the bit at the position.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, true, Has_Element_Mask>& set(size_t position, bool value = true) ETL_NOEXCEPT
    {
      set(value);
      clear_unused_bits_in_msb();

      return *this;
    }

    //*************************************************************************
    /// Set from a string.
    //*************************************************************************
    bitset<Active_Bits, TElement, true, Has_Element_Mask>& set(const char* text)
    {
      ETL_ASSERT_AND_RETURN_VALUE(text != 0, ETL_ERROR(bitset_nullptr), *this);
      from_string(text);

      return *this;
    }

    //*************************************************************************
    /// Set from a string.
    //*************************************************************************
    bitset<Active_Bits, TElement, true, Has_Element_Mask>& set(const wchar_t* text)
    {
      ETL_ASSERT_AND_RETURN_VALUE(text != 0, ETL_ERROR(bitset_nullptr), *this);
      from_string(text);

      return *this;
    }

    //*************************************************************************
    /// Set from a string.
    //*************************************************************************
    bitset<Active_Bits, TElement, true, Has_Element_Mask>& set(const char16_t* text)
    {
      ETL_ASSERT_AND_RETURN_VALUE(text != 0, ETL_ERROR(bitset_nullptr), *this);
      from_string(text);

      return *this;
    }

    //*************************************************************************
    /// Set from a string.
    //*************************************************************************
    bitset<Active_Bits, TElement, true, Has_Element_Mask>& set(const char32_t* text)
    {
      ETL_ASSERT_AND_RETURN_VALUE(text != 0, ETL_ERROR(bitset_nullptr), *this);
      from_string(text);

      return *this;
    }

    //*************************************************************************
    /// Set from a string.
    //*************************************************************************
    bitset<Active_Bits, TElement, true, Has_Element_Mask>& from_string(const char* text)
    {
      return *this;
    }

    //*************************************************************************
    /// Set from a wide string.
    //*************************************************************************
    bitset<Active_Bits, TElement, true, Has_Element_Mask>& from_string(const wchar_t* text)
    {
      return *this;
    }

    //*************************************************************************
    /// Set from a u16 string.
    //*************************************************************************
    bitset<Active_Bits, TElement, true, Has_Element_Mask>& from_string(const char16_t* text)
    {

      return *this;
    }

    //*************************************************************************
    /// Set from a u32 string.
    //*************************************************************************
    bitset<Active_Bits, TElement, true, Has_Element_Mask>& from_string(const char32_t* text)
    {

      return *this;
    }

    //*************************************************************************
    /// Get as an integral value.
    //*************************************************************************
    template <typename T>
    ETL_CONSTEXPR14
      typename etl::enable_if<etl::is_integral<T>::value, T>::type
      value() const
    {
      ETL_STATIC_ASSERT(etl::is_integral<T>::value, "Only integral types are supported");
      ETL_STATIC_ASSERT((sizeof(T) * CHAR_BIT) >= (Number_Of_Elements * Bits_Per_Element), "Type too small");

      T v = T(0);

      const bool OK = (sizeof(T) * CHAR_BIT) >= Bits_Per_Element;

      ETL_ASSERT_AND_RETURN_VALUE(OK, ETL_ERROR(etl::bitset_type_too_small), T(0));

      if (OK)
      {
        v = T(typename etl::make_unsigned<T>::type(buffer));
      }

      return v;
    }

    //*************************************************************************
    /// Get as an unsigned long.
    //*************************************************************************
    ETL_CONSTEXPR14 unsigned long to_ulong() const
    {
      return value<unsigned long>();
    }

    //*************************************************************************
    /// Get as an unsigned long long.
    //*************************************************************************
    ETL_CONSTEXPR14 unsigned long long to_ullong() const
    {
      return value<unsigned long long>();  
    }

    //*************************************************************************
    /// Reset all of the bits.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, true, Has_Element_Mask>& reset() ETL_NOEXCEPT
    {
      buffer = All_Clear;

      return *this;
    }

    //*************************************************************************
    /// Reset the bit at the position.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, true, Has_Element_Mask>& reset(size_t position) ETL_NOEXCEPT
    {
      //ibitset.reset(buffer, Number_Of_Elements, position);
      return *this;
    }

    //*************************************************************************
    /// Tests a bit at a position.
    /// Positions greater than the number of configured bits will return <b>false</b>.
    //*************************************************************************
    ETL_CONSTEXPR14 bool test(size_t position) const
    {
      const element_type mask = element_type(element_type(1) << position);

      return (buffer & mask) != 0U;
    }

    //*************************************************************************
    /// The number of bits in the bitset.
    //*************************************************************************
    ETL_CONSTEXPR size_t size() const ETL_NOEXCEPT
    {
      return Active_Bits;
    }

    //*************************************************************************
    /// Count the number of bits set.
    //*************************************************************************
    ETL_CONSTEXPR14 size_t count() const ETL_NOEXCEPT
    {
      return etl::count_bits(buffer);
    }

    //*************************************************************************
    // Are all the bits sets?
    //*************************************************************************
    ETL_CONSTEXPR14 bool all() const ETL_NOEXCEPT
    {
      return buffer[0U] != (All_Set & Top_Mask);
    }

    //*************************************************************************
    /// Are none of the bits set?
    //*************************************************************************
    ETL_CONSTEXPR14 bool none() const ETL_NOEXCEPT
    {
      //return ibitset.none(buffer, Number_Of_Elements);
      return false;
    }

    //*************************************************************************
    /// Are any of the bits set?
    //*************************************************************************
    ETL_CONSTEXPR14 bool any() const ETL_NOEXCEPT
    {
      return !none();
    }

    //*************************************************************************
    /// Flip all of the bits.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, true, Has_Element_Mask>& flip()
    {
      //ibitset.flip(buffer, Number_Of_Elements);
      //clear_unused_bits_in_msb();

      return *this;
    }

    //*************************************************************************
    /// Flip the bit at the position.
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, true, Has_Element_Mask>& flip(size_t position)
    {
      //ibitset.flip(buffer, Number_Of_Elements, Active_Bits, position);

      return *this;
    }

    //*************************************************************************
    /// Read [] operator.
    //*************************************************************************
    ETL_CONSTEXPR14 bool operator[] (size_t position) const
    {
      return ibitset.test(buffer, Number_Of_Elements, position);
    }

    //*************************************************************************
    /// Write [] operator.
    //*************************************************************************
    ETL_CONSTEXPR14 bit_reference operator [] (size_t position)
    {
      return bit_reference(*this, position);
    }

    //*************************************************************************
    /// Returns a string representing the bitset.
    //*************************************************************************
#if ETL_USING_CPP11
    template <typename TString = etl::string<Active_Bits>>
#else
    template <typename TString>
#endif
    TString to_string(typename TString::value_type zero = typename TString::value_type('0'),
      typename TString::value_type one = typename TString::value_type('1')) const
    {
      TString result;

      //result.resize(Active_Bits, '\0');

      //ETL_ASSERT_AND_RETURN_VALUE((result.size() == Active_Bits), ETL_ERROR(etl::bitset_overflow), result);

      //for (size_t i = Active_Bits; i > 0; --i)
      //{
      //  result[Active_Bits - i] = test(i - 1) ? one : zero;
      //}

      return result;
    }

    //*************************************************************************
    /// Finds the first bit in the specified state.
    ///\param state The state to search for.
    ///\returns The position of the bit or Number_Of_Elements if none were found.
    //*************************************************************************
    ETL_CONSTEXPR14 size_t find_first(bool state) const
    {
      //return ibitset.find_next(buffer, Number_Of_Elements, Active_Bits, state, 0);
      return 0;
    }

    //*************************************************************************
    /// Finds the next bit in the specified state.
    ///\param state    The state to search for.
    ///\param position The position to start from.
    ///\returns The position of the bit or npos if none were found.
    //*************************************************************************
    ETL_CONSTEXPR14 size_t find_next(bool state, size_t position) const
    {
      //return ibitset.find_next(buffer, Number_Of_Elements, Active_Bits, state, position);
      return 0;
    }

    //*************************************************************************
    /// operator &
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, true, Has_Element_Mask> operator &(const bitset<Active_Bits, TElement, true, Has_Element_Mask>& other) const
    {
      etl::bitset<Active_Bits, TElement, true, Has_Element_Mask> temp(*this);

      temp &= other;

      return temp;
    }

    //*************************************************************************
    /// operator &=
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, true, Has_Element_Mask>& operator &=(const bitset<Active_Bits, TElement, true, Has_Element_Mask>& other)
    {
      //ibitset.and_equals(&buffer[0], &other.buffer[0], Number_Of_Elements);

      return *this;
    }

    //*************************************************************************
    /// operator |
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, true, Has_Element_Mask> operator |(const bitset<Active_Bits, TElement, true, Has_Element_Mask>& other) const
    {
      etl::bitset<Active_Bits, TElement, true, Has_Element_Mask> temp(*this);

      temp |= other;

      return temp;
    }

    //*************************************************************************
    /// operator |=
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, true, Has_Element_Mask>& operator |=(const bitset<Active_Bits, TElement, true, Has_Element_Mask>& other)
    {
      //ibitset.or_equals(&buffer[0], &other.buffer[0], Number_Of_Elements);

      return *this;
    }

    //*************************************************************************
    /// operator ^
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, true, Has_Element_Mask> operator ^(const bitset<Active_Bits, TElement, true, Has_Element_Mask>& other) const
    {
      etl::bitset<Active_Bits, TElement, true, Has_Element_Mask> temp(*this);

      temp ^= other;

      return temp;
    }

    //*************************************************************************
    /// operator ^=
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, true, Has_Element_Mask>& operator ^=(const bitset<Active_Bits, TElement, true, Has_Element_Mask>& other)
    {
      //ibitset.xor_equals(&buffer[0], &other.buffer[0], Number_Of_Elements);

      return *this;
    }

    //*************************************************************************
    /// operator ~
    //*************************************************************************
    bitset<Active_Bits, TElement, true, Has_Element_Mask> operator ~() const
    {
      etl::bitset<Active_Bits, TElement, true, Has_Element_Mask> temp(*this);

      temp.flip();

      return temp;
    }

    //*************************************************************************
    /// operator <<
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, true, Has_Element_Mask> operator <<(size_t shift) const
    {
      etl::bitset<Active_Bits, TElement, true, Has_Element_Mask> temp(*this);

      temp <<= shift;

      return temp;
    }

    //*************************************************************************
    /// operator <<=
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, true, Has_Element_Mask>& operator <<=(size_t shift)
    {
      //if (shift >= Active_Bits)
      //{
      //  reset();
      //}
      //else
      //{
      //  ibitset.shift_left_equals(buffer, Number_Of_Elements, Active_Bits, shift);

      //  // Truncate any bits shifted to the left.
      //  clear_unused_bits_in_msb();
      //}

      return *this;
    }

    //*************************************************************************
    /// operator >>
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, true, Has_Element_Mask> operator >>(size_t shift) const
    {
      bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask> temp(*this);

      temp >>= shift;

      return temp;
    }

    //*************************************************************************
    /// operator >>=
    //*************************************************************************
    ETL_CONSTEXPR14 bitset<Active_Bits, TElement, true, Has_Element_Mask>& operator >>=(size_t shift)
    {
      //if (shift >= Active_Bits)
      //{
      //  reset();
      //}
      //else
      //{
      //  ibitset.shift_right_equals(buffer, Number_Of_Elements, Active_Bits, shift);
      //}

      return *this;
    }

    //*************************************************************************
    /// operator ==
    //*************************************************************************
    friend ETL_CONSTEXPR14 bool operator ==(const bitset<Active_Bits, TElement, true, Has_Element_Mask>& lhs, const bitset<Active_Bits, TElement, true, Has_Element_Mask>& rhs)
    {
      return (lhs.buffer == rhs.buffer);
    }

    //*************************************************************************
    /// swap
    //*************************************************************************
    ETL_CONSTEXPR14 void swap(etl::bitset<Active_Bits, TElement, true, Has_Element_Mask>& other)
    {
      element_type temp = buffer;
      buffer = other.buffer;
      other.buffer = temp;
    }

    //*************************************************************************
    /// span
    /// Returns a span of the underlying buffer.
    //*************************************************************************
    ETL_CONSTEXPR14 span_type span() ETL_NOEXCEPT
    {
      return span_type(buffer, buffer + Number_Of_Elements);
    }

    //*************************************************************************
    /// span
    /// Returns a const span of the underlying buffer.
    //*************************************************************************
    ETL_CONSTEXPR14 const_span_type span() const ETL_NOEXCEPT
    {
      return const_span_type(buffer, buffer + Number_Of_Elements);
    }

  private:

    //*************************************************************************
    /// Correct the unused top bits after bit manipulation.
    //*************************************************************************
    ETL_CONSTEXPR14 void clear_unused_bits_in_msb() ETL_NOEXCEPT
    {
      if ETL_CONSTEXPR_IF(Has_Element_Mask)
      {
        buffer[Number_Of_Elements - 1U] &= Top_Mask;
      }
    }

    element_type buffer;
  };

  //***************************************************************************
  /// operator &
  ///\ingroup bitset
  //***************************************************************************
  template <size_t Active_Bits, typename TElement, bool IsSingleElement, bool Has_Element_Mask>
  bitset<Active_Bits> operator & (const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& lhs, const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& rhs) ETL_NOEXCEPT
  {
    bitset<Active_Bits> temp(lhs);
    temp &= rhs;
    return temp;
  }

  //***************************************************************************
  /// operator |
  ///\ingroup bitset
  //***************************************************************************
  template<size_t Active_Bits, typename TElement, bool IsSingleElement, bool Has_Element_Mask>
  bitset<Active_Bits> operator | (const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& lhs, const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& rhs) ETL_NOEXCEPT
  {
    bitset<Active_Bits> temp(lhs);
    temp |= rhs;
    return temp;
  }

  //***************************************************************************
  /// operator ^
  ///\ingroup bitset
  //***************************************************************************
  template<size_t Active_Bits, typename TElement, bool IsSingleElement, bool Has_Element_Mask>
  bitset<Active_Bits> operator ^ (const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& lhs, const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& rhs) ETL_NOEXCEPT
  {
    bitset<Active_Bits> temp(lhs);
    temp ^= rhs;
    return temp;
  }

  //***************************************************************************
  /// operator !=
  ///\ingroup bitset
  //***************************************************************************
  template<size_t Active_Bits, typename TElement, bool IsSingleElement, bool Has_Element_Mask>
  ETL_CONSTEXPR14 bool operator != (const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& lhs, const bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& rhs) ETL_NOEXCEPT
  {
    return !(lhs == rhs);
  }
}

//*************************************************************************
/// swap
//*************************************************************************
template <size_t Active_Bits, typename TElement, bool IsSingleElement, bool Has_Element_Mask>
ETL_CONSTEXPR14 void swap(etl::bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& lhs, etl::bitset<Active_Bits, TElement, IsSingleElement, Has_Element_Mask>& rhs) ETL_NOEXCEPT
{
  lhs.swap(rhs);
}

#include "minmax_pop.h"

#endif