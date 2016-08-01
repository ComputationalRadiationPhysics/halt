/* This file is part of libLiFFT.
 *
 * libLiFFT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * libLiFFT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libLiFFT.  If not, see <www.gnu.org/licenses/>.
 */
 
#pragma once

namespace std {

    template< bool B, class T = void >
    using enable_if_t = typename enable_if<B,T>::type;

    template< bool B, class T, class U >
    using conditional_t = typename conditional< B, T, U >::type;

    template< class T >
    using result_of_t = typename result_of<T>::type;

    template< class T >
    using decay_t = typename decay<T>::type;

    template< class T >
    using remove_pointer_t = typename remove_pointer<T>::type;

    template< class T >
    using remove_reference_t = typename remove_reference<T>::type;

    template< class T >
    using remove_const_t = typename remove_const<T>::type;

    template< class T >
    using remove_cv_t = typename remove_cv<T>::type;

    template< class T >
    using make_unsigned_t = typename make_unsigned<T>::type;

    template< class T >
    using add_cv_t       = typename add_cv<T>::type;

    template< class T >
    using add_const_t    = typename add_const<T>::type;

    template< class T >
    using add_volatile_t = typename add_volatile<T>::type;

    template< typename T >
    struct integral_type
    {
        using type = T;
    };

    template< class T >
    constexpr const T& cmin( const T& a, const T& b )
    {
        return (b < a) ? b : a;
    }

}  // namespace std
