

namespace fonts
{
    // Allocate and Return an instance of the consola.ttf using the chosen allocator
    extern size_t GetConsolaTTF( void* ( *allocator )( size_t ), void*& bytes_out );
}

