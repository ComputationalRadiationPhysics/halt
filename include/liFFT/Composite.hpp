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

namespace LiFFT {

    /**
     * Functor that is composed of multiple functors
     * The last functor is applied first, then the next is applied to the result and so on
     */
    template < class... T_Funcs >
    struct Composite;

    template< class T_Func >
    struct Composite< T_Func >: T_Func{};

    template< class T_Func, class... T_Funcs >
    struct Composite< T_Func, T_Funcs... >{

        T_Func f1;
        Composite< T_Funcs... > f2;

        template< typename... T >
        auto
        operator()(T&&... args)
        -> decltype( f1( f2( std::forward<T>(args)... ) ) )
        {
            return f1( f2( std::forward<T>(args)... ) );
        }
    };


}  // namespace LiFFT
