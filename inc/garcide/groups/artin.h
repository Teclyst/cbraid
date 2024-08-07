/**
 * @file artin.h
 * @author Matteo Wei (matteo.wei@ens.psl.eu)
 * @brief Header file for standard braid groups (classic Garside structure).
 * @version 0.1
 * @date 2024-07-28
 *
 * @copyright Copyright (C) 2024. Distributed under the GNU General Public
 * License, version 3.
 *
 */

/*
 * GarCide Copyright (C) 2024 Matteo Wei.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License in LICENSE for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARTIN
#define ARTIN

#include "garcide/ultra_summit.h"

namespace garcide {

/**
 * @brief Namespace for standard braid groups, classic Garside structure.
 * 
 * On top of the `Underlying` class, there are functions to compute Thurston types.
 */
namespace artin {

/// A class for the underlying objects for canonical factors
/// in the Artin presentation braid group case.
/// In this case, permutations.
class Underlying {

  public:
    using Parameter = sint16;

  private:
    Parameter number_of_strands;

    std::vector<sint16> permutation_table;

    /**
     * @brief Maximum braid index.
     *
     * The greatest index that may be used for braids.
     *
     * It is used because we use `thread_local` objects to avoid some
     * allocations, and their size must be known at compile time.
     *
     * Having too big `thread_local` objects might cause some issue with thread
     * spawning.
     */
    static const sint16 MAX_NUMBER_OF_STRANDS = 256;

  public:
    static Parameter parameter_of_string(const std::string &str);

    /**
     * @brief Get the `number_of_strands`.
     *
     * Get the `number_of_strands` (which is `private`).
     *
     * @return `number_of_strands`.
     */
    Parameter get_parameter() const;

    sint16 at(size_t i) const { return permutation_table[i]; }

    /**
     * @brief Construct a new `Underlying`.
     *
     * Construct a new `Underlying`, with `n` as its
     * `number_of_strands`.
     *
     * Its `permutation_table` will have length `n`, and will be filled with
     * zeros (thus this is not a valid factor). Initialize it with `identity`,
     * `delta`, or another similar method.
     *
     * @param n The `number_of_strands` of the factor (also the length of
     * its `permutation_table`).
     */
    Underlying(Parameter n);

    /**
     * @brief Extraction from string.
     *
     * Reads the string `str`, starting at position `pos`, and sets `this` to
     * the corresponding atom.
     *
     * Letting `Z = -? ([1 - 9] [0 - 9]* | 0)` be the language of integers,
     * accepted strings are those represented by regular expression `Z`, under
     * the additional hypothesis that the integer they represent is in [`1`,
     * `Parameter`[.
     *
     * @param str The string to extract from.
     * @param pos The position to start from.
     * @exception InvalidStringError Thrown when there is no subword starting
     * from `pos` that matches `Z`, or if there is one, if the corresponding
     * integer does not belong to [`1`, `Parameter`[.
     */
    void of_string(const std::string &str, size_t &pos);

    sint16 lattice_height() const;

    /**
     * @brief Prints internal representation to `os`.
     *
     * Prints the factor's `permutation_table` to output stream `os`, typically for debugging
     * purposes.
     *
     * @param os The output stream it prints to.
     */
    void debug(IndentedOStream &os) const;

    /**
     * @brief Prints the factor to `os`.
     *
     * Prints the factor to output stream `os` as a product of atoms.
     *
     * @param os The output stream it prints to.
     */
    void print(IndentedOStream &os) const;

    // Set to the identity element (here the identity).
    void identity();

    // Set to delta.
    void delta();

    /**
     * @brief Computes the left meet of `*this` and `b`.
     *
     * @param b Second argument.
     * @return The left meet of `*this` and `b`.
     */
    Underlying left_meet(const Underlying &b) const;

    /**
     * @brief Computes the right meet of `*this` and `b`.
     *
     * @param b Second argument.
     * @return The right meet of `*this` and `b`.
     */
    Underlying right_meet(const Underlying &b) const;

    // Equality check.
    // We check wether the underlying permutation table are (pointwise) equal.
    bool compare(const Underlying &b) const;

    // product under the hypothesis that it is still simple.
    Underlying product(const Underlying &b) const;

    // Under the assumption a <= b, a.left_complement(b) computes
    // The factor c such that ac = b.
    Underlying left_complement(const Underlying &b) const;

    Underlying right_complement(const Underlying &b) const;

    // Generate a random factor.
    void randomize();

    // List of atoms.
    std::vector<Underlying> atoms() const;

    // Conjugate by delta^k.
    // Used to speed up calculations compared to the default implementation.
    void delta_conjugate_mut(sint16 k);

    size_t hash() const;

    /**
     * @brief Computes the tableau associated with a factor.
     *
     * Computes the tableau associated with `this` and store it in `tab`.
     *
     * This was directly copied (mutatis mutandis) from Juan Gonzalez-Meneses'
     * code.
     *
     * @param tab A matrix where the tableau is to be stored.
     */
    void tableau(sint16 **&tab) const;

  private:
    // Computes the factor corresponding to the inverse permutation.
    // Used to simplify complement operation.
    Underlying inverse() const;

    // Subroutine called by left_meet() and right_meet().
    static void MeetSub(const sint16 *a, const sint16 *b, sint16 *r, sint16 s,
                        sint16 t);
};

typedef FactorTemplate<Underlying> Factor;

typedef BraidTemplate<Factor> Braid;

/**
 * @brief Enum for Thurston types.
 *
 * An enum whose elements represent the three Thurston types.
 */
enum class ThurstonType { Periodic, Reducible, PseudoAsonov };

/**
 * @brief Determines if a braid preserves a family of circles.
 *
 * This was directly copied (mutatis mutandis) from Juan Gonzalez-Meneses' code.
 *
 * @param b The braid to be tested.
 * @return Whether `b` preserves a family of circles.
 */
bool preserves_circles(const Braid &b);

/**
 * @brief Computes the Thurston type of a braid whose USS was already computed.
 *
 * This was directly copied (mutatis mutandis) from Juan Gonzalez-Meneses' code.
 *
 * @param b The braid whose Thurston type is to be computed.
 * @param uss `b`'s USS.
 * @return `b`'s Thurston type.
 */
ThurstonType thurston_type(const Braid &b,
                           const ultra_summit::UltraSummitSet<Braid> &uss);

/**
 * @brief Computes the Thurston type of a braid.
 *
 * This was directly copied (mutatis mutandis) from Juan Gonzalez-Meneses' code.
 *
 * @param b The braid whose Thurston type is to be computed.
 * @return `b`'s Thurston type.
 */
ThurstonType thurston_type(const Braid &b);

} // namespace artin

/**
 * @brief Inserts a `ThurstonType` value in the output stream.
 *
 * @param type A ThurstonType value.
 * @return A reference to `*this`, so that `<<` may be chained.
 */
template <>
IndentedOStream &IndentedOStream::operator<< <artin::ThurstonType>(
    const artin::ThurstonType &type);

} // namespace garcide

#endif