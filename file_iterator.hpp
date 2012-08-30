/** \brief Byte iterator for access to big files without mmap */

#include <cstring>
#include <iostream>
#include <fstream>

#include <boost/scoped_array.hpp>

class file_iterator {

    std::istream *stream;
    std::streamoff pos;

    std::streamoff size;

    static const size_t block_size = 4096;
    boost::scoped_array<char> block;
    std::streamoff block_begin;

    void _read()
    {
        block_begin = pos - ( block_size >> 1 );
        std::streamoff neg = ( block_begin < 0 ) ? -block_begin : 0; // if begin is negative
        std::streamoff over = size - block_begin - block_size; over = ( over >= 0 ) ? 0 : -over; // if end over size
        stream->seekg( block_begin + neg, std::ios_base::beg );
        stream->read( block.get() + neg, block_size - neg - over );
    }

public:

    typedef std::random_access_iterator_tag iterator_category;
    typedef char value_type;
    typedef std::streamoff difference_type;
    typedef char* pointer;
    typedef char& reference;

    file_iterator( std::istream * stream, std::streamoff pos ) : stream(stream), pos(pos), block( new char[block_size] ) { stream->seekg( 0, std::ios_base::end ); size = stream->tellg(); _read(); }
    file_iterator() : stream(0), pos(0), size(0), block_begin(0) {}
    file_iterator( const file_iterator &other )
    {
    block.reset( new char[block_size] );
        *this = other;
    }
    file_iterator& operator=( const file_iterator &other )
    {
        if( this != &other )
        {
            stream = other.stream;
            pos = other.pos;
            size = other.size;
            if( block.get() && other.block.get() )
                memcpy( block.get(), other.block.get(), block_size );
            block_begin = other.block_begin;
        }
    }

    file_iterator & operator++() { ++pos; return *this; }
    file_iterator operator++( int ) { return file_iterator( stream, pos++ ); }
    file_iterator & operator--() { --pos; return *this; }
    file_iterator operator--( int ) { return file_iterator( stream, pos-- ); }
    file_iterator & operator+=( const file_iterator &other ) { pos += other.pos; return *this; }
    file_iterator & operator+=( difference_type diff ) { pos += diff; return *this; }
    file_iterator & operator-=( const file_iterator &other ) { pos -= other.pos; return *this; }
    file_iterator & operator-=( difference_type diff ) { pos -= diff; return *this; }
    difference_type operator-( const file_iterator &other ) { return pos - other.pos;  }
    bool operator!=( const file_iterator &other ) const { return pos != other.pos; }
    bool operator==( const file_iterator &other ) const { return pos == other.pos; }

    char operator*() { if( ( pos < block_begin ) || ( pos >= block_begin + block_size ) ) _read(); return block[ pos - block_begin ]; }
    difference_type get_pos() const { return pos; }
};

typedef file_iterator byte_file_iterator;
