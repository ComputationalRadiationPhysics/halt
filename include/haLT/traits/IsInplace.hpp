/* This file is part of HaLT.
 *
 * HaLT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * HaLT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with HaLT.  If not, see <www.gnu.org/licenses/>.
 */
 
#pragma once

namespace haLT {
namespace traits {

    /**
     * Evaluates to true type if the given type is an In-Place Indicator type
     * That is: Input data ptr should be uses as Output data ptr
     * and data format matches input data format except the type (complex or real)
     */
    template< class T >
    struct IsInplace: std::false_type{};

}  // namespace traits
}  // namespace haLT