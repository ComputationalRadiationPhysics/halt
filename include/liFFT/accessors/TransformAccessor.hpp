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
namespace accessors {

    /**
     * Accessor that transforms all elements on access using the specified functor
     */
    template< class T_BaseAccessor, class T_Func >
    struct TransformAccessor
    {
    private:
        using BaseAccessor = T_BaseAccessor;
        using Func = T_Func;

        BaseAccessor m_acc;
        Func m_func;
    public:

        TransformAccessor(){}
        template< class T, class U >
        explicit TransformAccessor(T&& baseAccessor, U&& func = T_Func()): m_acc(std::forward<T>(baseAccessor)), m_func(std::forward<U>(func)){}

        template< class T_Index, class T_Data >
        auto
        operator()(const T_Index& idx, const T_Data& data) const
        -> decltype(m_func(m_acc(idx, data)))
        {
            return m_func(m_acc(idx, data));
        }
    };

    /**
     * Creates a transform accessor for the given function and accessor
     * This will transform all values returned by the base accessor with the function
     *
     * @param acc Base accessor
     * @param func Transform functor
     * @return TransformAccessor
     */
    template< class T_BaseAccessor, class T_Func >
    TransformAccessor< T_BaseAccessor, T_Func >
    makeTransformAccessor(T_BaseAccessor&& acc, T_Func&& func)
    {
        return TransformAccessor< T_BaseAccessor, T_Func >(std::forward<T_BaseAccessor>(acc), std::forward<T_Func>(func));
    }

    /**
     * Creates a transform accessor for the given function when applied to the given container using its default accessor
     * @param func Transform functor
     * @param      Container instance
     * @return TransformAccessor
     */
    template< class T, class T_Func >
    TransformAccessor< traits::IdentityAccessor_t<T>, T_Func >
    makeTransformAccessorFor(T_Func&& func, const T& = T())
    {
        return TransformAccessor< traits::IdentityAccessor_t<T>, T_Func >(traits::IdentityAccessor_t<T>(), std::forward<T_Func>(func));
    }

}  // namespace accessors
}  // namespace LiFFT
