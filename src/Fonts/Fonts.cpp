#include "Fonts.h"
#include "consola.ttf.h"
#include <memory>

namespace fonts
{
    static size_t GetFont(
        void* ( *allocator )( size_t ),
        void*& bytes_out,
        const unsigned char* source,
        size_t source_size
    )
    {
        bytes_out = allocator( source_size );
        std::memcpy( bytes_out, source, source_size );
        return source_size;
    }

    size_t GetConsolaTTF( void* ( *allocator )( size_t ), void*& bytes_out )
    {
        return GetFont( allocator, bytes_out, consola_ttf, sizeof( consola_ttf ) );
    }
}
