/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2010 Unavowed <unavowed@vexillium.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <input_stream.h>
#include <cstring>

/*
 * Reads a non-integer little-endian value of the same size as the integer
 * type AliasT.
 */
template <typename T, typename AliasT>
bool read_le_alias(InputStream *is, T *valp)
{
   static_assert(sizeof(T) == sizeof(AliasT), "read_le_alias requires types that have same underlying storage size");

   AliasT aliasValue;
   if (!read_le_integer<AliasT>(is, &aliasValue))
      return false;

   std::memcpy(valp, &aliasValue, sizeof(aliasValue));
   return true;
}

template <>
bool read_le<float>(InputStream *is, float *valp)
{
   return read_le_alias<float, uint32_t>(is, valp);
}

template <>
bool read_le<double>(InputStream *is, double *valp)
{
   return read_le_alias<double, uint64_t>(is, valp);
}
