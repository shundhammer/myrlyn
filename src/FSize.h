/*  ---------------------------------------------------------
               __  __            _
              |  \/  |_   _ _ __| |_   _ _ __
              | |\/| | | | | '__| | | | | '_ \
              | |  | | |_| | |  | | |_| | | | |
              |_|  |_|\__, |_|  |_|\__, |_| |_|
                      |___/        |___/
    ---------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) 2024-25 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

 */


#ifndef _FSize_h_
#define _FSize_h_

#include <iosfwd>
#include <string>

// arbitrary precision integer
#include <boost/multiprecision/cpp_int.hpp>
// generate additional operators via the boost templates
#include <boost/operators.hpp>

//
//	CLASS NAME : FSize
//
/**
 * Store and operate on (file/package/partition) sizes.
 **/
class FSize :
    // generate > / * + - <= => !== operators
    boost::ordered_field_operators<FSize>,
    // generate postfix ++ --
    boost::unit_steppable<FSize>
  {

  public:

    /**
     * The Units
     **/
    enum class Unit { B, K, M, G, T, P, E, Z, Y };

  private:

    /**
     * The size (in bytes)
     * @see https://www.boost.org/doc/libs/release/libs/multiprecision/doc/html/index.html
     **/
    boost::multiprecision::cpp_int _size;

  public:

    static const boost::multiprecision::cpp_int KB;
    static const boost::multiprecision::cpp_int MB;
    static const boost::multiprecision::cpp_int GB;
    static const boost::multiprecision::cpp_int TB;
    static const boost::multiprecision::cpp_int PB;
    static const boost::multiprecision::cpp_int EB;
    // these do not fit long long anymore!
    static const boost::multiprecision::cpp_int ZB;
    static const boost::multiprecision::cpp_int YB;

    /**
     * Return ammount of bytes in Unit.
     **/
    static boost::multiprecision::cpp_int factor( const Unit unit_r ) {
      switch ( unit_r ) {
      case Unit::Y: return YB;
      case Unit::Z: return ZB;
      case Unit::E: return EB;
      case Unit::P: return PB;
      case Unit::T: return TB;
      case Unit::G: return GB;
      case Unit::M: return MB;
      case Unit::K: return KB;
      case Unit::B: break;
      }
      return 1;
    }

    /**
     * String representation of Unit.
     **/
    static const char * unit( const Unit unit_r ) {
      switch ( unit_r ) {
      case Unit::Y: return "YiB";
      case Unit::Z: return "ZiB";
      case Unit::E: return "EiB";
      case Unit::P: return "PiB";
      case Unit::T: return "TiB";
      case Unit::G: return "GiB";
      case Unit::M: return "MiB";
      case Unit::K: return "KiB";
      case Unit::B: break;
      }
      return "B";
    }

  public:

    /**
     * Construct from size in certain unit.
     * E.g. <code>FSize( 1, FSize::Unit::K )<code> makes 1024 Byte.
     **/
    FSize( const boost::multiprecision::cpp_int &size_r = 0, const Unit unit_r = Unit::B)
      : _size( boost::multiprecision::cpp_int(size_r) * factor( unit_r ) )
    {}

    /**
     * Construct from size in Byte.
     * @param size_r the initial value
     **/
    FSize( double size_r )
      : _size( size_r )
    {}

    /**
      Construct from string containing a number in given unit.
      * @param sizeStr input string - must contain *only* numbers
      * @param unit_r  optional unit, bytes by default
      * @throws std::runtime_error if the string contains any non numeric characters,
      *   even a trailing white space!
    */
    FSize( const std::string &sizeStr, const Unit unit_r = Unit::B );

    /**
     * Conversions to native data types - only explicit as it might overflow
     * If the value is out of range then the behavior depends on the boost version
     *   - 1.67 - the min/max values for the corresponding type are returned
     *   - 1.66 - returns just the lower bits
     **/
    explicit operator long long() const { return static_cast<long long>(_size); }
    explicit operator int() const { return static_cast<int>(_size); }
    explicit operator double() const { return static_cast<double>(_size); }

    operator boost::multiprecision::cpp_int() const { return _size; }
    boost::multiprecision::cpp_int in_unit(const Unit unit_r) const { return _size / factor( unit_r ); }

    // unary minus
    FSize operator-() const { return FSize(-_size); }
    FSize & operator+=( const FSize &rhs ) { _size += boost::multiprecision::cpp_int(rhs); return *this; }
    FSize & operator-=( const FSize &rhs ) { _size -= boost::multiprecision::cpp_int(rhs); return *this; }
    FSize & operator*=( const FSize &rhs ) { _size *= boost::multiprecision::cpp_int(rhs); return *this; }
    FSize & operator/=( const FSize &rhs ) { _size /= boost::multiprecision::cpp_int(rhs); return *this; }

    bool operator<( const FSize &rhs ) const { return _size < boost::multiprecision::cpp_int(rhs); }
    bool operator==( const FSize &rhs ) const { return _size == boost::multiprecision::cpp_int(rhs); }

    // ++operator (the prefix variant)
    FSize & operator++() { _size += 1; return *this; }
    // --operator (the prefix variant)
    FSize & operator--() { _size -= 1; return *this; }

    /**
     * Adjust size to multiple of <code>blocksize_r</code>
     **/
    FSize & fillBlock( FSize blocksize_r = boost::multiprecision::cpp_int(KB) );

    /**
     * Return a new size adjusted to multiple of <code>blocksize_r</code>
     **/
    FSize fullBlock( FSize blocksize_r = boost::multiprecision::cpp_int(KB) ) const
    {
        FSize ret( _size );
        return ret.fillBlock(  blocksize_r );
    }

    /**
     * Return the best unit for string representation.
     **/
    Unit bestUnit() const;

    /**
     * Used as precision argument to form(), the 'best' precision according to
     * Unist is chosen.
     **/
    static const unsigned bestPrec = (unsigned)-1;

    /**
     * Return string representation in given Unit. Parameter <code>fw</code> and
     * <code>prec</code> denote field width and precision as in a "%*.*f" printf
     * format string. Avalue of <code>bestPrec</code> automatically picks an
     * appropriate precision depending on the unit.
     * If <code>showunit</code> ist true, the string representaion
     * of Unit is <em>appended<em> separated by a single blank.
     *
     * If Unit is <b>B</b>yte, precision is set to zero.
     **/
    std::string form( const Unit unit_r, unsigned fw = 0, unsigned prec = bestPrec, bool showunit = true ) const;

    /**
     * Return string representation in bestUnit.
     **/
    std::string form( unsigned fw = 0, unsigned prec = bestPrec, bool showunit = true ) const {
      return form( bestUnit(), fw, prec, showunit );
    }

    /**
     * Default string representation (precision 1 and unit appended).
     **/
    std::string asString() const;
};

// stream operators
std::ostream& operator<<(std::ostream &ostr, const FSize&);
std::ostream& operator<<(std::ostream &ostr, const FSize::Unit);

#endif // _FSize_h_
